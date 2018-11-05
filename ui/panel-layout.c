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

ValaPanelCoreSettings *core_settings;
ValaPanelAppletManager *manager;

enum
{
	PROP_DUMMY,
	PROP_TOPLEVEL_ID,
	PROP_LAST
};
static GParamSpec *vp_layout_properties[PROP_LAST];

struct _ValaPanelLayout
{
	GtkBox __parent__;
	const char *toplevel_id;
	uint center_applets;
	GHashTable *applets;
	GtkWidget *start_box;
	GtkWidget *end_box;
	GtkWidget *center_box;
	bool suppress_sorting;
};

G_DEFINE_TYPE(ValaPanelLayout, vala_panel_layout, GTK_TYPE_BOX)

void vala_panel_layout_update_applet_positions(ValaPanelLayout *self);
static void vala_panel_layout_applets_reposition_after(ValaPanelLayout *self,
                                                       ValaPanelAppletPackType alignment,
                                                       uint after, GtkPackType direction);

static inline ValaPanelToplevel *vala_panel_layout_get_toplevel(ValaPanelLayout *self)
{
	return VALA_PANEL_TOPLEVEL(
	    gtk_widget_get_ancestor(GTK_WIDGET(self), vala_panel_toplevel_get_type()));
}

static inline ValaPanelLayout *vala_panel_applet_get_layout(ValaPanelApplet *self)
{
	return VALA_PANEL_LAYOUT(
	    gtk_widget_get_ancestor(GTK_WIDGET(self), vala_panel_layout_get_type()));
}

ValaPanelLayout *vala_panel_layout_new(ValaPanelToplevel *top, GtkOrientation orient, int spacing)
{
	return VALA_PANEL_LAYOUT(g_object_new(vala_panel_layout_get_type(),
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
	                                      vala_panel_toplevel_get_uuid(top),
	                                      NULL));
}

void vala_panel_layout_init_applets(ValaPanelLayout *self)
{
	g_auto(GStrv) core_units =
	    g_settings_get_strv(core_settings->core_settings, VALA_PANEL_CORE_UNITS);
	for (int i = 0; core_units[i] != NULL; i++)
	{
		const char *unit = core_units[i];
		ValaPanelUnitSettings *pl =
		    vala_panel_core_settings_get_by_uuid(core_settings, unit);
		if (!vala_panel_unit_settings_is_toplevel(pl))
		{
			g_autofree char *id =
			    g_settings_get_string(pl->common, VALA_PANEL_TOPLEVEL_ID);
			g_autofree char *name = g_settings_get_string(pl->common, VP_KEY_NAME);
			if (!g_strcmp0(id, self->toplevel_id))
				vala_panel_layout_place_applet(self,
				                               vp_applet_manager_applet_ref(manager,
				                                                            name),
				                               pl);
		}
	}
	vala_panel_layout_update_applet_positions(self);
	return;
}

static void vala_panel_applet_on_destroy(ValaPanelApplet *self, void *data)
{
	ValaPanelLayout *layout  = VALA_PANEL_LAYOUT(data);
	const char *uuid         = vala_panel_applet_get_uuid(self);
	ValaPanelUnitSettings *s = vala_panel_core_settings_get_by_uuid(core_settings, uuid);
	g_autofree char *name    = g_settings_get_string(s->common, VP_KEY_NAME);
	vp_applet_manager_applet_unref(manager, name);
	if (gtk_widget_in_destruction(GTK_WIDGET(layout)))
		vala_panel_core_settings_remove_unit_settings(core_settings, uuid);
	else
		g_hash_table_remove(layout->applets, uuid);
}

static void vala_panel_layout_check_center_visible(ValaPanelLayout *self)
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

static void vala_panel_layout_applet_repack(G_GNUC_UNUSED GHashTable *unused, ValaPanelApplet *info,
                                            ValaPanelLayout *self)
{
	/* Handle being reparented. */
	ValaPanelUnitSettings *settings = vala_panel_layout_get_applet_settings(info);
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
	vala_panel_layout_check_center_visible(self);
}

static void vala_panel_layout_applet_reposition(G_GNUC_UNUSED GHashTable *unused,
                                                ValaPanelApplet *pl,
                                                G_GNUC_UNUSED ValaPanelLayout *self)
{
	ValaPanelUnitSettings *settings = vala_panel_layout_get_applet_settings(pl);
	int pos                         = g_settings_get_uint(settings->common, VP_KEY_POSITION);
	gtk_box_reorder_child(GTK_BOX(gtk_widget_get_parent(GTK_WIDGET(pl))), GTK_WIDGET(pl), pos);
}

void vala_panel_layout_applet_packing_updated(GSettings *settings, const char *key, void *user_data)
{
	ValaPanelApplet *pl   = VALA_PANEL_APPLET(user_data);
	ValaPanelLayout *self = vala_panel_applet_get_layout(pl);

	/* Prevent a massive amount of resorting */
	if (!vala_panel_toplevel_is_initialized(vala_panel_applet_get_toplevel(pl)))
		return;

	if (!g_strcmp0(key, VP_KEY_PACK))
		vala_panel_layout_applet_repack(NULL, pl, self);
}

void vala_panel_layout_applet_position_updated(GSettings *settings, const char *key,
                                               void *user_data)
{
	ValaPanelApplet *pl = VALA_PANEL_APPLET(user_data);
	ValaPanelLayout *layout =
	    vala_panel_toplevel_get_layout(vala_panel_applet_get_toplevel(pl));

	/* Prevent a massive amount of resorting */
	if (!vala_panel_toplevel_is_initialized(vala_panel_applet_get_toplevel(pl)))
		return;

	/* Prevent a massive amount of resorting when applets is moved*/
	if (layout->suppress_sorting)
		return;

	if (!g_strcmp0(key, VP_KEY_POSITION))
		vala_panel_layout_applet_reposition(NULL, pl, NULL);
}

ValaPanelApplet *vala_panel_layout_place_applet(ValaPanelLayout *self, AppletInfoData *data,
                                                ValaPanelUnitSettings *s)
{
	if (data == NULL)
		return NULL;
	ValaPanelToplevel *top = vala_panel_layout_get_toplevel(self);
	ValaPanelApplet *applet =
	    vala_panel_applet_plugin_get_applet_widget(data->plugin, top, s->custom, s->uuid);
	g_hash_table_insert(self->applets, g_strdup(vala_panel_applet_get_uuid(applet)), applet);
	vala_panel_layout_applet_repack(NULL, applet, self);
	vala_panel_layout_applet_reposition(NULL, applet, NULL);
	g_signal_connect(s->common,
	                 "changed::" VP_KEY_PACK,
	                 vala_panel_layout_applet_packing_updated,
	                 applet);
	g_signal_connect(s->common,
	                 "changed::" VP_KEY_POSITION,
	                 vala_panel_layout_applet_position_updated,
	                 applet);
	g_signal_connect(applet, "destroy", G_CALLBACK(vala_panel_applet_on_destroy), self);
	return applet;
}

void vala_panel_layout_remove_applet(ValaPanelLayout *self, ValaPanelApplet *applet)
{
	g_autofree char *uuid        = g_strdup(vala_panel_applet_get_uuid(applet));
	ValaPanelUnitSettings *s     = vala_panel_core_settings_get_by_uuid(core_settings, uuid);
	uint pos                     = vala_panel_layout_get_applet_position(self, applet);
	ValaPanelAppletPackType type = vala_panel_layout_get_applet_pack_type(applet);
	g_clear_pointer(&applet, gtk_widget_destroy);
	vala_panel_layout_applets_reposition_after(self, type, pos, GTK_PACK_START);
	vala_panel_core_settings_remove_unit_settings_full(core_settings, uuid, true);
	vala_panel_layout_update_applet_positions(self);
}

const char *vala_panel_layout_get_toplevel_id(ValaPanelLayout *self)
{
	return self->toplevel_id;
}

GList *vala_panel_layout_get_applets_list(ValaPanelLayout *self)
{
	return g_hash_table_get_values(self->applets);
}

ValaPanelUnitSettings *vala_panel_layout_get_applet_settings(ValaPanelApplet *pl)
{
	const char *uuid = vala_panel_applet_get_uuid(pl);
	return vala_panel_core_settings_get_by_uuid(core_settings, uuid);
}

void vala_panel_layout_applets_repack(ValaPanelLayout *self)
{
	g_hash_table_foreach(self->applets, (GHFunc)vala_panel_layout_applet_repack, self);
}

void vala_panel_layout_update_applet_positions(ValaPanelLayout *self)
{
	g_hash_table_foreach(self->applets, (GHFunc)vala_panel_layout_applet_reposition, self);
}

ValaPanelAppletPackType vala_panel_layout_get_applet_pack_type(ValaPanelApplet *pl)
{
	ValaPanelUnitSettings *settings = vala_panel_layout_get_applet_settings(pl);
	ValaPanelAppletPackType type =
	    (ValaPanelAppletPackType)g_settings_get_enum(settings->common, VP_KEY_PACK);
	return type;
}

uint vala_panel_layout_get_applet_position(ValaPanelLayout *self, ValaPanelApplet *pl)
{
	int res;
	gtk_container_child_get(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(pl))),
	                        GTK_WIDGET(pl),
	                        VP_KEY_POSITION,
	                        &res,
	                        NULL);
	return (uint)res;
}

ValaPanelAppletManager *vala_panel_layout_get_manager()
{
	return manager;
}

ValaPanelApplet *vala_panel_layout_insert_applet(ValaPanelLayout *self, const char *type,
                                                 ValaPanelAppletPackType pack, uint pos)
{
	ValaPanelUnitSettings *s =
	    vala_panel_core_settings_add_unit_settings(core_settings, type, false);
	g_settings_set_string(s->common, VP_KEY_NAME, type);
	g_settings_set_string(s->common, VALA_PANEL_TOPLEVEL_ID, self->toplevel_id);
	vala_panel_layout_applets_reposition_after(self, pack, pos, GTK_PACK_END);
	g_settings_set_enum(s->common, VP_KEY_PACK, pack);
	g_settings_set_uint(s->common, VP_KEY_POSITION, pos);
	vp_applet_manager_reload_applets(manager);
	ValaPanelApplet *applet =
	    vala_panel_layout_place_applet(self, vp_applet_manager_applet_ref(manager, type), s);
	vala_panel_layout_update_applet_positions(self);
	return applet;
}

static uint count_applets_in_pack(ValaPanelLayout *self, ValaPanelAppletPackType pack)
{
	uint ret = 0;
	GList *applets;
	switch (pack)
	{
	case PACK_START:
		applets = gtk_container_get_children(self->start_box);
		break;
	case PACK_CENTER:
		applets = gtk_container_get_children(self->center_box);
		break;
	case PACK_END:
		applets = gtk_container_get_children(self->end_box);
		break;
	}
	ret = g_list_length(applets);
	g_list_free(applets);
	return ret;
}

bool vala_panel_layout_can_move_to_direction(ValaPanelLayout *self, ValaPanelApplet *prev,
                                             ValaPanelApplet *next, GtkPackType direction)
{
	ValaPanelUnitSettings *prev_settings = vala_panel_layout_get_applet_settings(prev);
	ValaPanelUnitSettings *next_settings =
	    next ? vala_panel_layout_get_applet_settings(next) : NULL;
	ValaPanelAppletPackType prev_pack = vala_panel_layout_get_applet_pack_type(prev);
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

void vala_panel_layout_move_applet_one_step(ValaPanelLayout *self, ValaPanelApplet *prev,
                                            ValaPanelApplet *next, GtkPackType direction)
{
	ValaPanelUnitSettings *prev_settings = vala_panel_layout_get_applet_settings(prev);
	ValaPanelUnitSettings *next_settings =
	    next ? vala_panel_layout_get_applet_settings(next) : NULL;
	ValaPanelAppletPackType prev_pack = vala_panel_layout_get_applet_pack_type(prev);
	ValaPanelAppletPackType next_pack;
	if (next)
		next_pack = vala_panel_layout_get_applet_pack_type(next);
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
			vala_panel_layout_applets_reposition_after(self, prev_pack, 0, direction);
		}
		else
		{
			vala_panel_layout_applets_reposition_after(self, next_pack, 0, direction);
			g_settings_set_uint(prev_settings->common, VP_KEY_POSITION, 0);
		}
		g_settings_set_enum(prev_settings->common, VP_KEY_PACK, next_pack);
	}
	else
	{
		uint prev_pos = vala_panel_layout_get_applet_position(self, prev);
		uint next_pos = vala_panel_layout_get_applet_position(self, next);
		g_settings_set_uint(prev_settings->common, VP_KEY_POSITION, next_pos);
		g_settings_set_uint(next_settings->common, VP_KEY_POSITION, prev_pos);
	}
	self->suppress_sorting = false;
	vala_panel_layout_update_applet_positions(self);
}

static void vala_panel_layout_applets_reposition_after(ValaPanelLayout *self,
                                                       ValaPanelAppletPackType alignment,
                                                       uint after, GtkPackType direction)
{
	GHashTableIter iter;
	char *key;
	ValaPanelApplet *val;
	g_hash_table_iter_init(&iter, self->applets);
	self->suppress_sorting = true;
	while (g_hash_table_iter_next(&iter, (void **)&key, (void **)&val))
	{
		ValaPanelUnitSettings *settings = vala_panel_layout_get_applet_settings(val);
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

static void vala_panel_layout_set_property(GObject *object, guint property_id, const GValue *value,
                                           GParamSpec *pspec)
{
	ValaPanelLayout *self = VALA_PANEL_LAYOUT(object);
	switch (property_id)
	{
	case PROP_TOPLEVEL_ID:
		g_free0(self->toplevel_id);
		self->toplevel_id = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
static void vala_panel_layout_get_property(GObject *object, guint property_id, GValue *value,
                                           GParamSpec *pspec)
{
	ValaPanelLayout *self = VALA_PANEL_LAYOUT(object);
	switch (property_id)
	{
	case PROP_TOPLEVEL_ID:
		g_value_set_string(value, self->toplevel_id);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_layout_destroy(GtkWidget *obj)
{
	ValaPanelLayout *self = VALA_PANEL_LAYOUT(obj);
	gtk_widget_destroy0(self->start_box);
	gtk_widget_destroy0(self->end_box);
	gtk_widget_destroy0(self->center_box);
	g_clear_pointer(&self->applets, g_hash_table_unref);
	GTK_WIDGET_CLASS(vala_panel_layout_parent_class)->destroy(GTK_WIDGET(self));
}

static GObject *vala_panel_layout_constructor(GType type, guint n_construct_properties,
                                              GObjectConstructParam *construct_properties)
{
	GObjectClass *parent_class = G_OBJECT_CLASS(vala_panel_layout_parent_class);
	GObject *obj =
	    parent_class->constructor(type, n_construct_properties, construct_properties);
	ValaPanelLayout *self = VALA_PANEL_LAYOUT(obj);
	gtk_box_pack_start(self, self->start_box, false, true, 0);
	gtk_widget_show(self->start_box);
	gtk_box_pack_end(self, self->end_box, false, true, 0);
	gtk_widget_show(self->end_box);
	gtk_box_set_center_widget(GTK_BOX(self), self->center_box);
	return G_OBJECT(self);
}

static void vala_panel_layout_init(ValaPanelLayout *self)
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
	self->applets = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

static void vala_panel_layout_class_init(ValaPanelLayoutClass *klass)
{
	manager                             = vp_applet_manager_new();
	core_settings                       = vala_panel_toplevel_get_core_settings();
	G_OBJECT_CLASS(klass)->constructor  = vala_panel_layout_constructor;
	G_OBJECT_CLASS(klass)->set_property = vala_panel_layout_set_property;
	G_OBJECT_CLASS(klass)->get_property = vala_panel_layout_get_property;
	GTK_WIDGET_CLASS(klass)->destroy    = vala_panel_layout_destroy;
	vp_layout_properties[PROP_TOPLEVEL_ID] =
	    g_param_spec_string(VALA_PANEL_TOPLEVEL_ID,
	                        VALA_PANEL_TOPLEVEL_ID,
	                        VALA_PANEL_TOPLEVEL_ID,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE |
	                                      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                PROP_TOPLEVEL_ID,
	                                vp_layout_properties[PROP_TOPLEVEL_ID]);
}
