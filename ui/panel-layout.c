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
#include "settings-manager.h"

ValaPanelCoreSettings *core_settings;
ValaPanelAppletManager *manager;

enum
{
	VALA_PANEL_LAYOUT_DUMMY,
	VALA_PANEL_LAYOUT_TOPLEVEL_ID,
	VALA_PANEL_LAYOUT_LAST
};
static GParamSpec *vala_panel_layout_properties[VALA_PANEL_LAYOUT_LAST];

struct _ValaPanelLayout
{
	GtkBox __parent__;
	const char *toplevel_id;
};

G_DEFINE_TYPE(ValaPanelLayout, vala_panel_layout, GTK_TYPE_BOX)

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
			    g_settings_get_string(pl->default_settings, VALA_PANEL_TOPLEVEL_ID);
			g_autofree char *name =
			    g_settings_get_string(pl->default_settings, VALA_PANEL_KEY_NAME);
			if (!g_strcmp0(id, self->toplevel_id))
				vala_panel_layout_place_applet(
				    self, vala_panel_applet_manager_applet_ref(manager, name), pl);
		}
	}
	vala_panel_layout_update_applet_positions(self);
	return;
}

static void vala_panel_applet_on_destroy(ValaPanelApplet *self, void *data)
{
	ValaPanelLayout *layout = VALA_PANEL_LAYOUT(data);
	const char *uuid        = vala_panel_applet_get_uuid(self);
	vala_panel_layout_applet_destroyed(layout, uuid);
	if (gtk_widget_in_destruction(GTK_WIDGET(layout)))
		vala_panel_core_settings_remove_unit_settings(core_settings, uuid);
}

void vala_panel_layout_place_applet(ValaPanelLayout *self, AppletInfoData *data,
                                    ValaPanelUnitSettings *s)
{
	if (data == NULL)
		return;
	ValaPanelApplet *applet =
	    vala_panel_applet_plugin_get_applet_widget(data->plugin,
	                                               VALA_PANEL_TOPLEVEL(gtk_widget_get_toplevel(
	                                                   GTK_WIDGET(self))),
	                                               s->custom_settings,
	                                               s->uuid);
	int position = g_settings_get_uint(s->default_settings, VALA_PANEL_KEY_POSITION);
	gtk_box_pack_start(self, applet, false, true, 0);
	gtk_box_reorder_child(self, applet, position);
	if (vala_panel_applet_info_is_expandable(data->info))
	{
		g_settings_bind(s->default_settings,
		                VALA_PANEL_KEY_EXPAND,
		                applet,
		                "hexpand",
		                G_SETTINGS_BIND_GET);
		g_object_bind_property(applet, "hexpand", applet, "vexpand", G_BINDING_SYNC_CREATE);
	}
	g_signal_connect(applet, "destroy", G_CALLBACK(vala_panel_applet_on_destroy), self);
}

void vala_panel_layout_applet_destroyed(ValaPanelLayout *self, const char *uuid)
{
	ValaPanelUnitSettings *s = vala_panel_core_settings_get_by_uuid(core_settings, uuid);
	g_autofree char *name    = g_settings_get_string(s->default_settings, VALA_PANEL_KEY_NAME);
	vala_panel_applet_manager_applet_unref(manager, name);
}

void vala_panel_layout_remove_applet(ValaPanelLayout *self, ValaPanelApplet *applet)
{
	const char *uuid         = vala_panel_applet_get_uuid(applet);
	ValaPanelUnitSettings *s = vala_panel_core_settings_get_by_uuid(core_settings, uuid);
	gtk_widget_destroy(GTK_WIDGET(applet));
	vala_panel_core_settings_remove_unit_settings_full(core_settings, uuid, true);
}

const char *vala_panel_layout_get_toplevel_id(ValaPanelLayout *self)
{
	return self->toplevel_id;
}

GList *vala_panel_layout_get_applets_list(ValaPanelLayout *self)
{
	return gtk_container_get_children(GTK_CONTAINER(self));
}

ValaPanelUnitSettings *vala_panel_layout_get_applet_settings(ValaPanelApplet *pl)
{
	const char *uuid = vala_panel_applet_get_uuid(pl);
	return vala_panel_core_settings_get_by_uuid(core_settings, uuid);
}

void vala_panel_layout_update_applet_positions(ValaPanelLayout *self)
{
	GList *children = gtk_container_get_children(GTK_CONTAINER(self));
	for (GList *l = children; l != NULL; l = l->next)
	{
		ValaPanelUnitSettings *settings =
		    vala_panel_layout_get_applet_settings(VALA_PANEL_APPLET(l->data));
		uint idx = g_settings_get_uint(settings->default_settings, VALA_PANEL_KEY_POSITION);
		gtk_box_reorder_child(GTK_BOX(self), GTK_WIDGET(l->data), (int)idx);
	}
	g_list_free(children);
}

uint vala_panel_layout_get_applet_position(ValaPanelLayout *self, ValaPanelApplet *pl)
{
	int res;
	gtk_container_child_get(GTK_CONTAINER(self), GTK_WIDGET(pl), "position", &res, NULL);
	return (uint)res;
}

void vala_panel_layout_set_applet_position(ValaPanelLayout *self, ValaPanelApplet *pl, int pos)
{
	gtk_box_reorder_child(GTK_BOX(self), GTK_WIDGET(pl), pos);
}

ValaPanelAppletManager *vala_panel_layout_get_manager()
{
	return manager;
}

void vala_panel_layout_add_applet(ValaPanelLayout *self, const gchar *type)
{
	ValaPanelUnitSettings *s =
	    vala_panel_core_settings_add_unit_settings(core_settings, type, false);
	g_settings_set_string(s->default_settings, VALA_PANEL_KEY_NAME, type);
	g_settings_set_string(s->default_settings, VALA_PANEL_TOPLEVEL_ID, self->toplevel_id);
	vala_panel_applet_manager_reload_applets(manager);
	vala_panel_layout_place_applet(self,
	                               vala_panel_applet_manager_applet_ref(manager, type),
	                               s);
	vala_panel_layout_update_applet_positions(self);
}

static void vala_panel_layout_set_property(GObject *object, guint property_id, const GValue *value,
                                           GParamSpec *pspec)
{
	ValaPanelLayout *self = VALA_PANEL_LAYOUT(object);
	switch (property_id)
	{
	case VALA_PANEL_LAYOUT_TOPLEVEL_ID:
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
	case VALA_PANEL_LAYOUT_TOPLEVEL_ID:
		g_value_set_string(value, self->toplevel_id);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_layout_init(ValaPanelLayout *self)
{
}

static void vala_panel_layout_class_init(ValaPanelLayoutClass *klass)
{
	manager                             = vala_panel_applet_manager_new();
	core_settings                       = vala_panel_toplevel_get_core_settings();
	G_OBJECT_CLASS(klass)->set_property = vala_panel_layout_set_property;
	G_OBJECT_CLASS(klass)->get_property = vala_panel_layout_get_property;
	g_object_class_install_property(
	    G_OBJECT_CLASS(klass),
	    VALA_PANEL_LAYOUT_TOPLEVEL_ID,
	    vala_panel_layout_properties[VALA_PANEL_LAYOUT_TOPLEVEL_ID] =
	        g_param_spec_string("toplevel-id",
	                            "toplevel-id",
	                            "toplevel-ids",
	                            NULL,
	                            G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
	                                G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE |
	                                G_PARAM_CONSTRUCT_ONLY));
}
