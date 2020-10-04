/*
 * vala-panel
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
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

#include <gtk-layer-shell/gtk-layer-shell.h>

#include "config.h"

#include "definitions.h"
#include "gio/gsettingsbackend.h"
#include "server.h"
#include "vala-panel-platform-standalone-layer-shell.h"

#define VALA_PANEL_CONFIG_HEADER "global"

struct _ValaPanelPlatformLayer
{
	ValaPanelPlatform __parent__;
	GtkApplication *app;
	char *profile;
};

#define g_key_file_load_from_config(f, p)                                                          \
	g_key_file_load_from_file(f,                                                               \
	                          _user_config_file_name(GETTEXT_PACKAGE, p, NULL),                \
	                          G_KEY_FILE_KEEP_COMMENTS,                                        \
	                          NULL)

G_DEFINE_TYPE(ValaPanelPlatformLayer, vala_panel_platform_layer, vala_panel_platform_get_type())

ValaPanelPlatformLayer *vala_panel_platform_layer_new(GtkApplication *app, const char *profile)
{
	ValaPanelPlatformLayer *pl =
	    VALA_PANEL_PLATFORM_LAYER(g_object_new(vala_panel_platform_layer_get_type(), NULL));
	pl->app                   = app;
	pl->profile               = g_strdup(profile);
	g_autofree char *filename = _user_config_file_name_new(pl->profile);
	g_autoptr(GSettingsBackend) backend =
	    g_keyfile_settings_backend_new(filename,
	                                   VALA_PANEL_OBJECT_PATH,
	                                   VALA_PANEL_CONFIG_HEADER);
	vala_panel_platform_init_settings(VALA_PANEL_PLATFORM(pl), backend);
	return pl;
}

static void predicate_func(const char *key, ValaPanelUnitSettings *value, ValaPanelPlatform *self)
{
	bool is_toplevel                  = vala_panel_unit_settings_is_toplevel(value);
	ValaPanelPlatformLayer *user_data = VALA_PANEL_PLATFORM_LAYER(self);
	if (is_toplevel)
	{
		ValaPanelToplevel *unit = vala_panel_toplevel_new(user_data->app, self, key);
		GtkWindow *win          = GTK_WINDOW(unit);
		vala_panel_platform_register_unit(self, win);
		gtk_layer_init_for_window(win);
		gtk_layer_set_layer(win, GTK_LAYER_SHELL_LAYER_TOP);
		gtk_layer_set_namespace(win, "panel"); // FIXME: may have conflicts with mate-panel
		vala_panel_toplevel_init_ui(unit);
		gtk_application_add_window(user_data->app, win);
	}
}

/*********************************************************************************************
 * Positioning
 *********************************************************************************************/
static void update_toplevel_geometry_for_all(GdkDisplay *scr, void *data)
{
	GtkApplication *app = GTK_APPLICATION(data);
	int mons            = gdk_display_get_n_monitors(scr);
	GList *win          = gtk_application_get_windows(app);
	for (GList *il = win; il != NULL; il = il->next)
	{
		if (VALA_PANEL_IS_TOPLEVEL(il->data))
		{
			ValaPanelToplevel *panel = (ValaPanelToplevel *)il->data;
			vala_panel_update_visibility(panel, mons);
		}
	}
}
static void monitor_notify_cb(GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, gpointer user_data)
{
	GdkMonitor *mon = GDK_MONITOR(gobject);
	update_toplevel_geometry_for_all(gdk_monitor_get_display(mon), user_data);
}

static void monitors_update_init(void *user_data)
{
	GdkDisplay *disp = gdk_display_get_default();
	int mons         = gdk_display_get_n_monitors(disp);
	for (int i = 0; i < mons; i++)
	{
		GdkMonitor *mon = gdk_display_get_monitor(disp, i);
		g_signal_connect(mon, "notify", G_CALLBACK(monitor_notify_cb), user_data);
	}
}
static void monitors_update_finalize(void *user_data)
{
	GdkDisplay *disp = gdk_display_get_default();
	int mons         = gdk_display_get_n_monitors(disp);
	for (int i = 0; i < mons; i++)
	{
		GdkMonitor *mon = gdk_display_get_monitor(disp, i);
		g_signal_handlers_disconnect_by_data(mon, user_data);
	}
}

static void monitor_added_cb(GdkDisplay *scr, GdkMonitor *mon, void *data)
{
	g_signal_connect(mon, "notify", G_CALLBACK(monitor_notify_cb), data);
	update_toplevel_geometry_for_all(scr, data);
}

static void monitor_removed_cb(GdkDisplay *scr, GdkMonitor *mon, void *data)
{
	g_signal_handlers_disconnect_by_data(mon, data);
	update_toplevel_geometry_for_all(scr, data);
}

static const char *vpp_layer_get_name(ValaPanelPlatform *obj)
{
	return "layer-shell";
}

static bool vpp_layer_start_panels_from_profile(ValaPanelPlatform *obj, GtkApplication *app,
                                                G_GNUC_UNUSED const char *profile)
{
	ValaPanelPlatformLayer *self = VALA_PANEL_PLATFORM_LAYER(obj);
	ValaPanelCoreSettings *core  = vala_panel_platform_get_settings(obj);
	g_hash_table_foreach(core->all_units, (GHFunc)predicate_func, self);
	monitors_update_init(app);
	g_signal_connect(gdk_display_get_default(),
	                 "monitor-added",
	                 G_CALLBACK(monitor_added_cb),
	                 self->app);
	g_signal_connect(gdk_display_get_default(),
	                 "monitor-removed",
	                 G_CALLBACK(monitor_removed_cb),
	                 self->app);
	return vala_panel_platform_has_units_loaded(obj);
}

// TODO: Make more readable code without switch
static void vpp_layer_move_to_side(G_GNUC_UNUSED ValaPanelPlatform *f, GtkWindow *top,
                                   PanelGravity gravity, int monitor)
{
	bool anchor[GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER] = { false };
	switch (gravity)
	{
	case NORTH_LEFT:
	case WEST_UP:
		anchor[GTK_LAYER_SHELL_EDGE_LEFT] = true;
		anchor[GTK_LAYER_SHELL_EDGE_TOP]  = true;
		break;
	case NORTH_CENTER:
		anchor[GTK_LAYER_SHELL_EDGE_TOP] = true;
		break;
	case EAST_UP:
	case NORTH_RIGHT:
		anchor[GTK_LAYER_SHELL_EDGE_RIGHT] = true;
		anchor[GTK_LAYER_SHELL_EDGE_TOP]   = true;
		break;
	case WEST_DOWN:
	case SOUTH_LEFT:
		anchor[GTK_LAYER_SHELL_EDGE_LEFT]   = true;
		anchor[GTK_LAYER_SHELL_EDGE_BOTTOM] = true;
		break;
	case SOUTH_CENTER:
		anchor[GTK_LAYER_SHELL_EDGE_BOTTOM] = true;
		break;
	case EAST_DOWN:
	case SOUTH_RIGHT:
		anchor[GTK_LAYER_SHELL_EDGE_RIGHT]  = true;
		anchor[GTK_LAYER_SHELL_EDGE_BOTTOM] = true;
		break;
	case WEST_CENTER:
		anchor[GTK_LAYER_SHELL_EDGE_LEFT] = true;
		break;
	case EAST_CENTER:
		anchor[GTK_LAYER_SHELL_EDGE_RIGHT] = true;
		break;
	}
	for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++)
		gtk_layer_set_anchor(top, i, anchor[i]);
}

static bool vpp_layer_edge_can_strut(G_GNUC_UNUSED ValaPanelPlatform *f, GtkWindow *top)
{
	int strut_set = false;
	g_object_get(top, VP_KEY_STRUT, &strut_set, NULL);
	if (!gtk_widget_get_mapped(GTK_WIDGET(top)))
		return false;
	return strut_set;
}

static void vpp_layer_update_strut(ValaPanelPlatform *f, GtkWindow *top)
{
	if (vala_panel_platform_can_strut(f, top))
		gtk_layer_auto_exclusive_zone_enable(top);
	else
		gtk_layer_set_exclusive_zone(top, 0);
}

static bool vpp_layer_edge_available(ValaPanelPlatform *self, GtkWindow *top, PanelGravity gravity,
                                     int monitor)
{
	ValaPanelPlatformLayer *pl = VALA_PANEL_PLATFORM_LAYER(self);
	int edge                   = vala_panel_edge_from_gravity(gravity);
	bool strut                 = true;
	g_object_get(top, VP_KEY_STRUT, &strut, NULL);
	if (!strut)
	{
		return strut;
	}
	for (g_autoptr(GList) w = gtk_application_get_windows(pl->app); w != NULL; w = w->next)
		if (VALA_PANEL_IS_TOPLEVEL(w))
		{
			ValaPanelToplevel *pl = VALA_PANEL_TOPLEVEL(w->data);
			bool have_toplevel    = VALA_PANEL_IS_TOPLEVEL(top);
			int smonitor          = 0;
			PanelGravity sgravity;
			g_object_get(pl,
			             VP_KEY_MONITOR,
			             &smonitor,
			             VP_KEY_GRAVITY,
			             &sgravity,
			             NULL);
			if ((!have_toplevel || (pl != VALA_PANEL_TOPLEVEL(top))) &&
			    (vala_panel_edge_from_gravity(sgravity) == edge) &&
			    ((monitor == smonitor) || smonitor < 0))
				return false;
		}
	return true;
}
static void vpp_layer_finalize(GObject *obj)
{
	ValaPanelPlatformLayer *self = VALA_PANEL_PLATFORM_LAYER(obj);
	g_signal_handlers_disconnect_by_data(gdk_display_get_default(), self->app);
	monitors_update_finalize(self->app);
	g_clear_pointer(&self->profile, g_free);
	G_OBJECT_CLASS(vala_panel_platform_layer_parent_class)->finalize(obj);
}

static void vala_panel_platform_layer_init(G_GNUC_UNUSED ValaPanelPlatformLayer *self)
{
}

static void vala_panel_platform_layer_class_init(ValaPanelPlatformLayerClass *klass)
{
	VALA_PANEL_PLATFORM_CLASS(klass)->get_name     = vpp_layer_get_name;
	VALA_PANEL_PLATFORM_CLASS(klass)->move_to_side = vpp_layer_move_to_side;
	VALA_PANEL_PLATFORM_CLASS(klass)->update_strut = vpp_layer_update_strut;
	VALA_PANEL_PLATFORM_CLASS(klass)->can_strut    = vpp_layer_edge_can_strut;
	VALA_PANEL_PLATFORM_CLASS(klass)->start_panels_from_profile =
	    vpp_layer_start_panels_from_profile;
	VALA_PANEL_PLATFORM_CLASS(klass)->edge_available = vpp_layer_edge_available;
	G_OBJECT_CLASS(klass)->finalize                  = vpp_layer_finalize;
}
