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

#include "config.h"

#include "definitions.h"
#include "gio/gsettingsbackend.h"
#include "server.h"
#include "vala-panel-platform-standalone-x11.h"

#define VALA_PANEL_CONFIG_HEADER "global"

struct _ValaPanelPlatformX11
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

G_DEFINE_TYPE(ValaPanelPlatformX11, vala_panel_platform_x11, vala_panel_platform_get_type())

ValaPanelPlatformX11 *vala_panel_platform_x11_new(GtkApplication *app, const char *profile)
{
	ValaPanelPlatformX11 *pl =
	    VALA_PANEL_PLATFORM_X11(g_object_new(vala_panel_platform_x11_get_type(), NULL));
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
	bool is_toplevel                = vala_panel_unit_settings_is_toplevel(value);
	ValaPanelPlatformX11 *user_data = VALA_PANEL_PLATFORM_X11(self);
	if (is_toplevel)
	{
		ValaPanelToplevel *unit = vala_panel_toplevel_new(user_data->app, self, key);
		vala_panel_platform_register_unit(self, GTK_WINDOW(unit));
		vala_panel_toplevel_init_ui(unit);
		gtk_application_add_window(user_data->app, GTK_WINDOW(unit));
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
			vala_panel_toplevel_update_visibility(panel, mons);
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

static const char *vpp_x11_get_name(ValaPanelPlatform *obj)
{
	return "x11";
}

static bool vpp_x11_start_panels_from_profile(ValaPanelPlatform *obj, GtkApplication *app,
                                              G_GNUC_UNUSED const char *profile)
{
	ValaPanelPlatformX11 *self  = VALA_PANEL_PLATFORM_X11(obj);
	ValaPanelCoreSettings *core = vala_panel_platform_get_settings(obj);
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
static void vpp_x11_move_to_side(G_GNUC_UNUSED ValaPanelPlatform *f, GtkWindow *top,
                                 ValaPanelGravity gravity, int monitor)
{
	GtkOrientation orient = vala_panel_orient_from_gravity(gravity);
	GdkMonitor *mon       = vala_panel_platform_get_suitable_monitor(GTK_WIDGET(top), monitor);
	GdkRectangle marea;
	int x = 0, y = 0;
	gdk_monitor_get_geometry(mon, &marea);
	GtkRequisition size, min;
	gtk_widget_get_preferred_size(GTK_WIDGET(top), &min, &size);
	int height = orient == GTK_ORIENTATION_HORIZONTAL ? size.height : size.width;
	int width  = orient == GTK_ORIENTATION_HORIZONTAL ? size.width : size.height;
	switch (gravity)
	{
	case VALA_PANEL_GRAVITY_NORTH_LEFT:
	case VALA_PANEL_GRAVITY_WEST_UP:
		x = marea.x;
		y = marea.y;
		break;
	case VALA_PANEL_GRAVITY_NORTH_CENTER:
		x = marea.x + (marea.width - width) / 2;
		y = marea.y;
		break;
	case VALA_PANEL_GRAVITY_NORTH_RIGHT:
		x = marea.x + marea.width - width;
		y = marea.y;
		break;
	case VALA_PANEL_GRAVITY_SOUTH_LEFT:
		x = marea.x;
		y = marea.y + marea.height - height;
		break;
	case VALA_PANEL_GRAVITY_SOUTH_CENTER:
		x = marea.x + (marea.width - width) / 2;
		y = marea.y + marea.height - height;
		break;
	case VALA_PANEL_GRAVITY_SOUTH_RIGHT:
		x = marea.x + marea.width - width;
		y = marea.y + marea.height - height;
		break;
	case VALA_PANEL_GRAVITY_WEST_CENTER:
		x = marea.x;
		y = marea.y + (marea.height - width) / 2;
		break;
	case VALA_PANEL_GRAVITY_WEST_DOWN:
		x = marea.x;
		y = marea.y + (marea.height - width);
		break;
	case VALA_PANEL_GRAVITY_EAST_UP:
		x = marea.x + marea.width - height;
		y = marea.y;
		break;
	case VALA_PANEL_GRAVITY_EAST_CENTER:
		x = marea.x + marea.width - height;
		y = marea.y + (marea.height - width) / 2;
		break;
	case VALA_PANEL_GRAVITY_EAST_DOWN:
		x = marea.x + marea.width - height;
		y = marea.y + (marea.height - width);
		break;
	}
	gtk_window_move(top, x, y);
}

static bool vpp_x11_edge_can_strut(G_GNUC_UNUSED ValaPanelPlatform *f, GtkWindow *top)
{
	int strut_set = false;
	g_object_get(top, VALA_PANEL_KEY_STRUT, &strut_set, NULL);
	if (!gtk_widget_get_mapped(GTK_WIDGET(top)))
		return false;
	return strut_set;
}

static void vpp_x11_update_strut(ValaPanelPlatform *f, GtkWindow *top)
{
	bool autohide;
	ValaPanelGravity gravity;
	int monitor;
	int size, len;
	g_object_get(top,
	             VALA_PANEL_KEY_AUTOHIDE,
	             &autohide,
	             VALA_PANEL_KEY_GRAVITY,
	             &gravity,
	             VALA_PANEL_KEY_MONITOR,
	             &monitor,
	             VALA_PANEL_KEY_HEIGHT,
	             &size,
	             VALA_PANEL_KEY_WIDTH,
	             &len,
	             NULL);
	GdkRectangle primary_monitor_rect;
	GtkPositionType edge = vala_panel_edge_from_gravity(gravity);
	long struts[12]      = { 0 };
	GdkMonitor *mon      = vala_panel_platform_get_suitable_monitor(GTK_WIDGET(top), monitor);
	int scale_factor     = gdk_monitor_get_scale_factor(mon);
	gdk_monitor_get_geometry(mon, &primary_monitor_rect);
	/*
	strut-left strut-right strut-top strut-bottom
	strut-left-start-y   strut-left-end-y
	strut-right-start-y  strut-right-end-y
	strut-top-start-x    strut-top-end-x
	strut-bottom-start-x strut-bottom-end-x
	*/

	if (!gtk_widget_get_realized(GTK_WIDGET(top)))
		return;
	int panel_size = autohide ? VALA_PANEL_AUTOHIDE_GAP : size;
	// Struts dependent on position
	switch (edge)
	{
	case GTK_POS_TOP:
		struts[2] = (primary_monitor_rect.y + panel_size) * scale_factor;
		struts[8] = (primary_monitor_rect.x) * scale_factor;
		struts[9] = (primary_monitor_rect.x + primary_monitor_rect.width / 100 * len) *
		            scale_factor;
		break;
	case GTK_POS_LEFT:
		struts[0] = panel_size * scale_factor;
		struts[4] = primary_monitor_rect.y * scale_factor;
		struts[5] = (primary_monitor_rect.y + primary_monitor_rect.height / 100 * len) *
		            scale_factor;
		break;
	case GTK_POS_RIGHT:
		struts[1] = panel_size * scale_factor;
		struts[6] = primary_monitor_rect.y * scale_factor;
		struts[7] = (primary_monitor_rect.y + primary_monitor_rect.height / 100 * len) *
		            scale_factor;
		break;
	case GTK_POS_BOTTOM:
		struts[3]  = (primary_monitor_rect.y - panel_size) * scale_factor;
		struts[10] = primary_monitor_rect.x * scale_factor;
		struts[11] = (primary_monitor_rect.x + primary_monitor_rect.width / 100 * len) *
		             scale_factor;
		break;
	}
	GdkAtom atom     = gdk_atom_intern_static_string("_NET_WM_STRUT_PARTIAL");
	GdkAtom old_atom = gdk_atom_intern_static_string("_NET_WM_STRUT");
	GdkWindow *xwin  = gtk_widget_get_window(GTK_WIDGET(top));
	if (vala_panel_platform_can_strut(f, top))
	{
		gdk_property_change(xwin,
		                    atom,
		                    gdk_atom_intern_static_string("CARDINAL"),
		                    32,
		                    GDK_PROP_MODE_REPLACE,
		                    (unsigned char *)struts,
		                    12);
		gdk_property_change(xwin,
		                    old_atom,
		                    gdk_atom_intern_static_string("CARDINAL"),
		                    32,
		                    GDK_PROP_MODE_REPLACE,
		                    (unsigned char *)struts,
		                    4);
	}
	else
	{
		gdk_property_delete(xwin, atom);
		gdk_property_delete(xwin, old_atom);
	}
}

static bool vpp_x11_edge_available(ValaPanelPlatform *self, GtkWindow *top, ValaPanelGravity gravity,
                                   int monitor)
{
	ValaPanelPlatformX11 *pl = VALA_PANEL_PLATFORM_X11(self);
	int edge                 = vala_panel_edge_from_gravity(gravity);
	bool strut               = true;
	g_object_get(top, VALA_PANEL_KEY_STRUT, &strut, NULL);
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
			ValaPanelGravity sgravity;
			g_object_get(pl,
			             VALA_PANEL_KEY_MONITOR,
			             &smonitor,
			             VALA_PANEL_KEY_GRAVITY,
			             &sgravity,
			             NULL);
			if ((!have_toplevel || (pl != VALA_PANEL_TOPLEVEL(top))) &&
			    (vala_panel_edge_from_gravity(sgravity) == edge) &&
			    ((monitor == smonitor) || smonitor < 0))
				return false;
		}
	return true;
}
static void vpp_x11_finalize(GObject *obj)
{
	ValaPanelPlatformX11 *self = VALA_PANEL_PLATFORM_X11(obj);
	g_signal_handlers_disconnect_by_data(gdk_display_get_default(), self->app);
	monitors_update_finalize(self->app);
	g_clear_pointer(&self->profile, g_free);
	G_OBJECT_CLASS(vala_panel_platform_x11_parent_class)->finalize(obj);
}

static void vala_panel_platform_x11_init(G_GNUC_UNUSED ValaPanelPlatformX11 *self)
{
}

static void vala_panel_platform_x11_class_init(ValaPanelPlatformX11Class *klass)
{
	VALA_PANEL_PLATFORM_CLASS(klass)->get_name     = vpp_x11_get_name;
	VALA_PANEL_PLATFORM_CLASS(klass)->move_to_side = vpp_x11_move_to_side;
	VALA_PANEL_PLATFORM_CLASS(klass)->update_strut = vpp_x11_update_strut;
	VALA_PANEL_PLATFORM_CLASS(klass)->can_strut    = vpp_x11_edge_can_strut;
	VALA_PANEL_PLATFORM_CLASS(klass)->start_panels_from_profile =
	    vpp_x11_start_panels_from_profile;
	VALA_PANEL_PLATFORM_CLASS(klass)->edge_available = vpp_x11_edge_available;
	G_OBJECT_CLASS(klass)->finalize                  = vpp_x11_finalize;
}
