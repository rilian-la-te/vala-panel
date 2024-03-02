/*
 * vala-panel
 * Copyright (C) 2015-2018 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "panel-layout.h"
#include "definitions.h"
#include "private.h"
#include "settings-manager.h"

static ValaPanelCoreSettings *core_settings;
static ValaPanelAppletManager *manager;

enum
{
	LAYOUT_DUMMY,
	LAYOUT_TOPLEVEL_ID,
	LAYOUT_LAST
};
static GParamSpec *vp_layout_properties[LAYOUT_LAST];

struct _ValaPanelLayout
{
	GtkBox __parent__;
	char *toplevel_id;
	uint center_applets;
	GHashTable *applets;
	GtkWidget *start_box;
	GtkWidget *end_box;
	GtkWidget *center_box;
	bool suppress_sorting;
};

G_DEFINE_TYPE(ValaPanelLayout, vp_layout, GTK_TYPE_BOX)

static ValaPanelApplet *vp_layout_place_applet(ValaPanelLayout *self, const char *name,
                                               ValaPanelUnitSettings *s);
void vp_layout_update_applet_positions(ValaPanelLayout *self);
static void vp_layout_applets_reposition_after(ValaPanelLayout *self,
                                               ValaPanelAppletPackType alignment, uint after,
                                               GtkPackType direction);

static inline ValaPanelToplevel *vp_layout_get_toplevel(ValaPanelLayout *self)
{
	return VALA_PANEL_TOPLEVEL(
	    gtk_widget_get_ancestor(GTK_WIDGET(self), vp_toplevel_get_type()));
}

static inline ValaPanelLayout *vp_applet_get_layout(ValaPanelApplet *self)
{
	return VALA_PANEL_LAYOUT(gtk_widget_get_ancestor(GTK_WIDGET(self), vp_layout_get_type()));
}

G_GNUC_INTERNAL ValaPanelLayout *vp_layout_new(ValaPanelToplevel *top, GtkOrientation orient,
                                               int spacing)
{
	return VALA_PANEL_LAYOUT(g_object_new(vp_layout_get_type(),
	                                      "orientation",
	                                      orient,
	                                      "spacing",
	                                      spacing,
	                                      "baseline-position",
	                                      GTK_BASELINE_POSITION_CENTER,
	                                      "border-width",
	                                      0,
	                                      "hexpand",
	                                      true,
	                                      "vexpand",
	                                      true,
	                                      "toplevel-id",
	                                      vp_toplevel_get_uuid(top),
	                                      NULL));
}

static void update_widget_position_keys(GtkContainer *parent)
{
	g_autoptr(GList) applets_list = gtk_container_get_children(parent);
	for (GList *l = applets_list; l != NULL; l = g_list_next(l))
	{
		ValaPanelApplet *applet = VALA_PANEL_APPLET(l->data);
		uint idx;
		gtk_container_child_get(GTK_CONTAINER(parent),
		                        GTK_WIDGET(applet),
		                        VP_KEY_POSITION,
		                        &idx,
		                        NULL);
		ValaPanelUnitSettings *s = vp_layout_get_applet_settings(applet);
		g_settings_set_uint(s->common, VP_KEY_POSITION, idx);
	}
}

static void restore_positions_by_pack(ValaPanelLayout *layout, ValaPanelAppletPackType pack)
{
	switch (pack)
	{
	case PACK_START:
		update_widget_position_keys(GTK_CONTAINER(layout->start_box));
		break;
	case PACK_CENTER:
		update_widget_position_keys(GTK_CONTAINER(layout->center_box));
		break;
	case PACK_END:
		update_widget_position_keys(GTK_CONTAINER(layout->end_box));
		break;
	}
}

G_GNUC_INTERNAL void vp_layout_init_applets(ValaPanelLayout *self)
{
	g_auto(GStrv) core_units =
	    g_settings_get_strv(core_settings->core_settings, VALA_PANEL_CORE_UNITS);
	for (int i = 0; core_units[i] != NULL; i++)
	{
		const char *unit          = core_units[i];
		ValaPanelUnitSettings *pl = vp_core_settings_get_by_uuid(core_settings, unit);
		if (!vp_unit_settings_is_toplevel(pl))
		{
			g_autofree char *id =
			    g_settings_get_string(pl->common, VALA_PANEL_TOPLEVEL_ID);
			g_autofree char *name = g_settings_get_string(pl->common, VP_KEY_NAME);
			if (!g_strcmp0(id, self->toplevel_id))
				vp_layout_place_applet(self, name, pl);
		}
	}
	vp_layout_update_applet_positions(self);
	return;
}

static void vp_applet_on_destroy(ValaPanelApplet *self, void *data)
{
	ValaPanelLayout *layout  = VALA_PANEL_LAYOUT(data);
	const char *uuid         = vp_applet_get_uuid(self);
	ValaPanelUnitSettings *s = vp_core_settings_get_by_uuid(core_settings, uuid);
	g_autofree char *name    = g_settings_get_string(s->common, VP_KEY_NAME);
	vp_applet_manager_applet_unref(manager, name);
	if (gtk_widget_in_destruction(GTK_WIDGET(layout)))
		vp_core_settings_remove_unit_settings(core_settings, uuid);
	else
		g_hash_table_remove(layout->applets, uuid);
}

static void vp_layout_check_center_visible(ValaPanelLayout *self)
{
	if (self->center_applets)
		gtk_widget_show(self->center_box);
	else
		gtk_widget_hide(self->center_box);
}

static void applet_reparent(GtkWidget *widget, GtkContainer *newp)
{
	g_object_ref(widget);
	if (GTK_IS_WIDGET(gtk_widget_get_parent(widget)))
		gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(widget)), widget);
	gtk_container_add(newp, widget);
	g_object_unref(widget);
}

static void vp_layout_applet_repack(G_GNUC_UNUSED GHashTable *unused, ValaPanelApplet *info,
                                    ValaPanelLayout *self)
{
	/* Handle being reparented. */
	ValaPanelUnitSettings *settings = vp_layout_get_applet_settings(info);
	ValaPanelAppletPackType type =
	    (ValaPanelAppletPackType)g_settings_get_enum(settings->common, VP_KEY_PACK);
	GtkContainer *new_parent = NULL;
	GtkContainer *old_parent = GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(info)));
	switch (type)
	{
	case PACK_START:
		new_parent = GTK_CONTAINER(self->start_box);
		break;
	case PACK_END:
		new_parent = GTK_CONTAINER(self->end_box);
		break;
	default:
		new_parent = GTK_CONTAINER(self->center_box);
		break;
	}
	/* Don't needlessly reparent */
	if (new_parent == old_parent)
		return;

	if (new_parent == GTK_CONTAINER(self->center_box))
		self->center_applets++;
	if (old_parent == GTK_CONTAINER(self->center_box))
		self->center_applets--;

	applet_reparent(GTK_WIDGET(info), GTK_CONTAINER(new_parent));
	vp_layout_check_center_visible(self);
}

static void vp_layout_applet_reposition(G_GNUC_UNUSED GHashTable *unused, ValaPanelApplet *pl,
                                        G_GNUC_UNUSED ValaPanelLayout *self)
{
	ValaPanelUnitSettings *settings = vp_layout_get_applet_settings(pl);
	uint pos                        = g_settings_get_uint(settings->common, VP_KEY_POSITION);
	gtk_box_reorder_child(GTK_BOX(gtk_widget_get_parent(GTK_WIDGET(pl))),
	                      GTK_WIDGET(pl),
	                      (int)pos);
}

G_GNUC_INTERNAL void vp_layout_applet_packing_updated(G_GNUC_UNUSED GSettings *settings,
                                                      const char *key, void *user_data)
{
	ValaPanelApplet *pl   = VALA_PANEL_APPLET(user_data);
	ValaPanelLayout *self = vp_applet_get_layout(pl);

	/* Prevent a massive amount of resorting */
	if (!vp_toplevel_is_initialized(vp_applet_get_toplevel(pl)))
		return;

	if (!g_strcmp0(key, VP_KEY_PACK))
		vp_layout_applet_repack(NULL, pl, self);
}

G_GNUC_INTERNAL void vp_layout_applet_position_updated(G_GNUC_UNUSED GSettings *settings,
                                                       const char *key, void *user_data)
{
	ValaPanelApplet *pl = VALA_PANEL_APPLET(user_data);
	ValaPanelLayout *layout =
	    vp_toplevel_get_layout(vp_applet_get_toplevel(pl));

	/* Prevent a massive amount of resorting */
	if (!vp_toplevel_is_initialized(vp_applet_get_toplevel(pl)))
		return;

	/* Prevent a massive amount of resorting when applets is moved*/
	if (layout->suppress_sorting)
		return;

	if (!g_strcmp0(key, VP_KEY_POSITION))
		vp_layout_applet_reposition(NULL, pl, NULL);
}

static ValaPanelApplet *vp_layout_place_applet(ValaPanelLayout *self, const char *name,
                                               ValaPanelUnitSettings *s)
{
	if (name == NULL)
		return NULL;
	ValaPanelToplevel *top  = vp_layout_get_toplevel(self);
	ValaPanelApplet *applet = vp_applet_manager_get_applet_widget(manager, name, top, s);
	if (!applet)
		return NULL;
	bool nonfloat = g_object_is_floating(applet) ? false : true;
	g_hash_table_insert(self->applets, g_strdup(vp_applet_get_uuid(applet)), applet);
	vp_layout_applet_repack(NULL, applet, self);
	vp_layout_applet_reposition(NULL, applet, NULL);
	g_signal_connect(s->common,
	                 "changed::" VP_KEY_PACK,
	                 G_CALLBACK(vp_layout_applet_packing_updated),
	                 applet);
	g_signal_connect(s->common,
	                 "changed::" VP_KEY_POSITION,
	                 G_CALLBACK(vp_layout_applet_position_updated),
	                 applet);
	g_signal_connect(applet, "destroy", G_CALLBACK(vp_applet_on_destroy), self);
	if (nonfloat)
		g_object_unref(applet);
	return applet;
}

G_GNUC_INTERNAL void vp_layout_remove_applet(ValaPanelLayout *self, ValaPanelApplet *applet)
{
	g_autofree char *uuid        = g_strdup(vp_applet_get_uuid(applet));
	uint pos                     = vp_layout_get_applet_position(self, applet);
	ValaPanelAppletPackType type = vp_layout_get_applet_pack_type(applet);
	ValaPanelUnitSettings *unit  = vp_core_settings_get_by_uuid(core_settings, uuid);
	g_signal_handlers_disconnect_by_data(unit->common, applet);
	gtk_widget_destroy(GTK_WIDGET(applet));
	vp_layout_applets_reposition_after(self, type, pos, GTK_PACK_START);
	vp_core_settings_remove_unit_settings_full(core_settings, uuid, true);
	vp_layout_update_applet_positions(self);
}

GList *vp_layout_get_applets_list(ValaPanelLayout *self)
{
	return g_hash_table_get_values(self->applets);
}

G_GNUC_INTERNAL ValaPanelUnitSettings *vp_layout_get_applet_settings(ValaPanelApplet *pl)
{
	const char *uuid = vp_applet_get_uuid(pl);
	return vp_core_settings_get_by_uuid(core_settings, uuid);
}

G_GNUC_INTERNAL void vp_layout_update_applet_positions(ValaPanelLayout *self)
{
	g_hash_table_foreach(self->applets, (GHFunc)vp_layout_applet_reposition, self);
}

G_GNUC_INTERNAL ValaPanelAppletPackType vp_layout_get_applet_pack_type(ValaPanelApplet *pl)
{
	ValaPanelUnitSettings *settings = vp_layout_get_applet_settings(pl);
	ValaPanelAppletPackType type =
	    (ValaPanelAppletPackType)g_settings_get_enum(settings->common, VP_KEY_PACK);
	return type;
}

G_GNUC_INTERNAL uint vp_layout_get_applet_position(G_GNUC_UNUSED ValaPanelLayout *self,
                                                   ValaPanelApplet *pl)
{
	int res;
	gtk_container_child_get(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(pl))),
	                        GTK_WIDGET(pl),
	                        VP_KEY_POSITION,
	                        &res,
	                        NULL);
	return (uint)res;
}

G_GNUC_INTERNAL ValaPanelAppletManager *vp_layout_get_manager()
{
	return manager;
}

G_GNUC_INTERNAL ValaPanelApplet *vp_layout_insert_applet(ValaPanelLayout *self, const char *type,
                                                         ValaPanelAppletPackType pack, uint pos)
{
	ValaPanelUnitSettings *s = vp_core_settings_add_unit_settings(core_settings, type, false);
	g_settings_set_string(s->common, VP_KEY_NAME, type);
	g_settings_set_string(s->common, VALA_PANEL_TOPLEVEL_ID, self->toplevel_id);
	vp_layout_applets_reposition_after(self, pack, pos, GTK_PACK_END);
	g_settings_set_enum(s->common, VP_KEY_PACK, (int)pack);
	g_settings_set_uint(s->common, VP_KEY_POSITION, pos);
	vp_applet_manager_reload_applets(manager);
	ValaPanelApplet *applet = vp_layout_place_applet(self, type, s);
	vp_layout_update_applet_positions(self);
	return applet;
}

static uint count_applets_in_pack(ValaPanelLayout *self, ValaPanelAppletPackType pack)
{
	uint ret       = 0;
	GList *applets = NULL;
	switch (pack)
	{
	case PACK_START:
		applets = gtk_container_get_children(GTK_CONTAINER(self->start_box));
		break;
	case PACK_CENTER:
		applets = gtk_container_get_children(GTK_CONTAINER(self->center_box));
		break;
	case PACK_END:
		applets = gtk_container_get_children(GTK_CONTAINER(self->end_box));
		break;
	}
	ret = g_list_length(applets);
	g_clear_pointer(&applets, g_list_free);
	return ret;
}

G_GNUC_INTERNAL bool vp_layout_can_move_to_direction(G_GNUC_UNUSED ValaPanelLayout *self,
                                                     ValaPanelApplet *prev, ValaPanelApplet *next,
                                                     GtkPackType direction)
{
	ValaPanelAppletPackType prev_pack = vp_layout_get_applet_pack_type(prev);
	if (!next)
	{
		if ((direction == GTK_PACK_START && prev_pack != PACK_START) ||
		    (direction == GTK_PACK_END && prev_pack != PACK_END))
			return true;
		else
			return false;
	}
	return true;
}

G_GNUC_INTERNAL void vp_layout_move_applet_one_step(ValaPanelLayout *self, ValaPanelApplet *prev,
                                                    ValaPanelApplet *next, GtkPackType direction)
{
	ValaPanelUnitSettings *prev_settings = vp_layout_get_applet_settings(prev);
	ValaPanelUnitSettings *next_settings = next ? vp_layout_get_applet_settings(next) : NULL;
	ValaPanelAppletPackType prev_pack    = vp_layout_get_applet_pack_type(prev);
	ValaPanelAppletPackType next_pack;
	if (next)
		next_pack = vp_layout_get_applet_pack_type(next);
	else
	{
		if (direction == GTK_PACK_START && prev_pack != PACK_START)
			next_pack = prev_pack == PACK_END ? PACK_CENTER : PACK_START;
		else if (direction == GTK_PACK_END && prev_pack != PACK_END)
			next_pack = prev_pack == PACK_START ? PACK_CENTER : PACK_END;
		else
			return;
	}
	self->suppress_sorting = true;
	if (prev_pack != next_pack)
	{
		/* Do not allow to jump from end to start directly*/
		if ((prev_pack == PACK_START && next_pack == PACK_END) ||
		    (prev_pack == PACK_END && next_pack == PACK_START))
			next_pack = PACK_CENTER;
		/* End pack has inverted position, so, when we move to end, we should move like
		 * backward */
		if (direction == GTK_PACK_START)
		{
			g_settings_set_uint(prev_settings->common,
			                    VP_KEY_POSITION,
			                    count_applets_in_pack(self, next_pack));
			g_settings_set_enum(prev_settings->common, VP_KEY_PACK, (int)next_pack);
			vp_layout_applets_reposition_after(self, prev_pack, 0, direction);
		}
		else
		{
			vp_layout_applets_reposition_after(self, next_pack, 0, direction);
			g_settings_set_enum(prev_settings->common, VP_KEY_PACK, (int)next_pack);
			g_settings_set_uint(prev_settings->common, VP_KEY_POSITION, 0);
		}
	}
	else
	{
		uint prev_pos = vp_layout_get_applet_position(self, prev);
		uint next_pos = vp_layout_get_applet_position(self, next);
		g_settings_set_uint(prev_settings->common, VP_KEY_POSITION, next_pos);
		g_settings_set_uint(next_settings->common, VP_KEY_POSITION, prev_pos);
	}
	self->suppress_sorting = false;
	vp_layout_update_applet_positions(self);
	self->suppress_sorting = true;
	restore_positions_by_pack(self, prev_pack);
	restore_positions_by_pack(self, next_pack);
	self->suppress_sorting = false;
}

static void vp_layout_applets_reposition_after(ValaPanelLayout *self,
                                               ValaPanelAppletPackType alignment, uint after,
                                               GtkPackType direction)
{
	GHashTableIter iter;
	char *key;
	ValaPanelApplet *val;
	g_hash_table_iter_init(&iter, self->applets);
	self->suppress_sorting = true;
	while (g_hash_table_iter_next(&iter, (void **)&key, (void **)&val))
	{
		ValaPanelUnitSettings *settings = vp_core_settings_get_by_uuid(core_settings, key);
		ValaPanelAppletPackType type =
		    (ValaPanelAppletPackType)g_settings_get_enum(settings->common, VP_KEY_PACK);
		if (type == alignment)
		{
			uint pos = g_settings_get_uint(settings->common, VP_KEY_POSITION);
			if (pos >= after)
			{
				pos = (direction == GTK_PACK_START) ? pos - 1 : pos + 1;
				g_settings_set_uint(settings->common, VP_KEY_POSITION, pos);
			}
		}
	}
	self->suppress_sorting = false;
}

static void vp_layout_set_property(GObject *object, guint property_id, const GValue *value,
                                   GParamSpec *pspec)
{
	ValaPanelLayout *self = VALA_PANEL_LAYOUT(object);
	switch (property_id)
	{
	case LAYOUT_TOPLEVEL_ID:
		g_free0(self->toplevel_id);
		self->toplevel_id = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
static void vp_layout_get_property(GObject *object, guint property_id, GValue *value,
                                   GParamSpec *pspec)
{
	ValaPanelLayout *self = VALA_PANEL_LAYOUT(object);
	switch (property_id)
	{
	case LAYOUT_TOPLEVEL_ID:
		g_value_set_string(value, self->toplevel_id);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vp_layout_destroy(GObject *obj)
{
	ValaPanelLayout *self = VALA_PANEL_LAYOUT(obj);
	gtk_widget_destroy0(self->start_box);
	gtk_widget_destroy0(self->end_box);
	gtk_widget_destroy0(self->center_box);
	g_clear_pointer(&self->applets, g_hash_table_unref);
	g_clear_pointer(&self->toplevel_id, g_free);
	G_OBJECT_CLASS(vp_layout_parent_class)->dispose(obj);
}

static void vp_layout_constructed(GObject *obj)
{
	ValaPanelLayout *self = VALA_PANEL_LAYOUT(obj);
	gtk_box_pack_start(GTK_BOX(self), self->start_box, false, true, 0);
	gtk_widget_show(self->start_box);
	gtk_box_pack_end(GTK_BOX(self), self->end_box, false, true, 0);
	gtk_widget_show(self->end_box);
	gtk_box_set_center_widget(GTK_BOX(self), self->center_box);
}

static void vp_layout_init(ValaPanelLayout *self)
{
	self->center_box     = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	self->start_box      = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	self->end_box        = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	self->center_applets = 0;
	g_object_bind_property(self,
	                       "orientation",
	                       self->center_box,
	                       "orientation",
	                       G_BINDING_SYNC_CREATE);
	g_object_bind_property(self,
	                       "orientation",
	                       self->start_box,
	                       "orientation",
	                       G_BINDING_SYNC_CREATE);
	g_object_bind_property(self,
	                       "orientation",
	                       self->end_box,
	                       "orientation",
	                       G_BINDING_SYNC_CREATE);
	self->applets = g_hash_table_new_full(g_str_hash,
	                                      g_str_equal,
	                                      g_free,
	                                      (GDestroyNotify)gtk_widget_destroy);
}

static void vp_layout_class_init(ValaPanelLayoutClass *klass)
{
	manager                             = vp_toplevel_get_manager();
	core_settings                       = vp_toplevel_get_core_settings();
	G_OBJECT_CLASS(klass)->constructed  = vp_layout_constructed;
	G_OBJECT_CLASS(klass)->set_property = vp_layout_set_property;
	G_OBJECT_CLASS(klass)->get_property = vp_layout_get_property;
	G_OBJECT_CLASS(klass)->dispose      = vp_layout_destroy;
	vp_layout_properties[LAYOUT_TOPLEVEL_ID] =
	    g_param_spec_string(VALA_PANEL_TOPLEVEL_ID,
	                        VALA_PANEL_TOPLEVEL_ID,
	                        VALA_PANEL_TOPLEVEL_ID,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                                      G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_properties(G_OBJECT_CLASS(klass), LAYOUT_LAST, vp_layout_properties);
}
