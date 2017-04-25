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

#include "gio/gsettingsbackend.h"
#include "lib/applets-new/applet-api.h"
#include "lib/c-lib/toplevel.h"
#include "lib/definitions.h"
#include "lib/settings-manager.h"
#include "lib/vala-panel-compat.h"
#include "vala-panel-platform-standalone-x11.h"

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
	pl->app     = app;
	pl->profile = g_strdup(profile);
	GSettingsBackend *backend =
	    g_keyfile_settings_backend_new(_user_config_file_name_new(pl->profile),
	                                   VALA_PANEL_OBJECT_PATH,
	                                   VALA_PANEL_CONFIG_HEADER);
	vala_panel_platform_init_settings(VALA_PANEL_PLATFORM(pl), backend);
	return pl;
}

static bool vala_panel_platform_x11_start_panels_from_profile(ValaPanelPlatform *obj,
                                                              GtkApplication *app,
                                                              const char *profile)
{
	ValaPanelCoreSettings *core = vala_panel_platform_get_settings(obj);
	ValaPanelPlatformX11 *self  = VALA_PANEL_PLATFORM_X11(obj);
	g_autoptr(GSettings) s =
	    g_settings_new_with_backend_and_path(core->root_schema, core->backend, core->root_path);
	g_auto(GStrv) panels = g_settings_get_strv(s, VALA_PANEL_APPLICATION_PANELS);
	int count;
	for (count = 0; count < g_strv_length(panels); count++)
	{
		g_autofree char *toplevel_uuid = g_strdup(panels[count]);
		vala_panel_core_settings_add_unit_settings_full(core,
		                                                VALA_PANEL_TOPLEVEL_SCHEMA_ELEM,
		                                                toplevel_uuid,
		                                                true);
		ValaPanelToplevel *unit = vala_panel_toplevel_new(self->app, obj, toplevel_uuid);
		gtk_application_add_window(app, GTK_WINDOW(unit));
	}
	return count > 0 ? true : false;
}

static void vala_panel_platform_x11_move_to_coords(ValaPanelPlatform *f, GtkWindow *top, int x,
                                                   int y)
{
	gtk_window_move(top, x, y);
}

static void vala_panel_platform_x11_move_to_side(ValaPanelPlatform *f, GtkWindow *top,
                                                 GtkPositionType alloc, int monitor)
{
	GtkOrientation orient = vala_panel_orient_from_edge(alloc);
	GdkDisplay *d         = gtk_widget_get_display(GTK_WIDGET(top));
	GdkMonitor *mon       = gdk_display_get_monitor(d, monitor);
	GdkRectangle geom;
	gdk_monitor_get_workarea(mon, &geom);
	gtk_window_move(top, 0, 0);
}

/*
 *             ulong s = 0;
            size = 0;
            if (!get_mapped())
                return false;
            if (autohide)
                s = GAP;
            else switch (orientation)
            {
                case Gtk.Orientation.VERTICAL:
                    s = a.width;
                    break;
                case Gtk.Orientation.HORIZONTAL:
                    s = a.height;
                    break;
                default: return false;
            }
            if (monitor < 0)
            {
                size = s;
                return true;
            }
            if (monitor >= get_screen().get_n_monitors())
                return false;
            Gdk.Rectangle rect, rect2;
            get_screen().get_monitor_geometry(monitor, out rect);
            switch(edge)
            {
                case PositionType.LEFT:
                    rect.width = rect.x;
                    rect.x = 0;
                    s += rect.width;
                    break;
                case PositionType.RIGHT:
                    rect.x += rect.width;
                    rect.width = get_screen().get_width() - rect.x;
                    s += rect.width;
                    break;
                case PositionType.TOP:
                    rect.height = rect.y;
                    rect.y = 0;
                    s += rect.height;
                    break;
                case PositionType.BOTTOM:
                    rect.y += rect.height;
                    rect.height = get_screen().get_height() - rect.y;
                    s += rect.height;
                    break;
            }
            if (!(rect.height == 0 || rect.width == 0)) // on a border of monitor
            {
                var n = get_screen().get_n_monitors();
                for (var i = 0; i < n; i++)
                {
                    if (i == monitor)
                        continue;
                    get_screen().get_monitor_geometry(i, out rect2);
                    if (rect.intersect(rect2, null))
                        // that monitor lies over the edge
                        return false;
                }
            }
            size = s;
            return true;*/
static bool vala_panel_platform_x11_edge_can_strut(ValaPanelPlatform *f, GtkWindow *top)
{
	bool strut;
	g_object_get(f, VALA_PANEL_KEY_STRUT, &strut, NULL);
	if (!gtk_widget_get_mapped(GTK_WIDGET(top)) || !strut)
		return false;
	return true;
}

static void vala_panel_platform_x11_update_strut(ValaPanelPlatform *f, GtkWindow *top)
{
	bool autohide;
	GtkPositionType edge;
	int monitor;
	int size;
	g_object_get(top,
	             VALA_PANEL_KEY_AUTOHIDE,
	             &autohide,
	             VALA_PANEL_KEY_EDGE,
	             &edge,
	             VALA_PANEL_KEY_MONITOR,
	             &monitor,
	             VALA_PANEL_KEY_HEIGHT,
	             &size,
	             NULL);
	GdkRectangle primary_monitor_rect;
	long struts[12]    = { 0 };
	GdkDisplay *screen = gtk_widget_get_display(GTK_WIDGET(top));
	GdkMonitor *mon    = monitor < 0 ? gdk_display_get_primary_monitor(screen)
	                              : gdk_display_get_monitor(screen, monitor);
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
	int panel_size = autohide ? GAP : size;
	// Struts dependent on position
	switch (edge)
	{
	case GTK_POS_TOP:
		struts[2] = primary_monitor_rect.y + panel_size;
		struts[8] = primary_monitor_rect.x;
		struts[9] = (primary_monitor_rect.x + primary_monitor_rect.width);
		break;
	case GTK_POS_LEFT:
		struts[0] = panel_size;
		struts[4] = primary_monitor_rect.y;
		struts[5] = primary_monitor_rect.y + primary_monitor_rect.height;
		break;
	case GTK_POS_RIGHT:
		struts[1] = panel_size;
		struts[6] = primary_monitor_rect.y;
		struts[7] = primary_monitor_rect.y + primary_monitor_rect.height;
		break;
	case GTK_POS_BOTTOM:
		struts[3]  = (primary_monitor_rect.height + primary_monitor_rect.y) - panel_size;
		struts[10] = primary_monitor_rect.x;
		struts[11] = (primary_monitor_rect.x + primary_monitor_rect.width);
		break;
	}
	GdkAtom atom = gdk_atom_intern_static_string("_NET_WM_STRUT_PARTIAL");
	if (vala_panel_platform_x11_edge_can_strut(f, top))
		// all relevant WMs support this, Mutter included
		gdk_property_change(gtk_widget_get_window(GTK_WIDGET(top)),
		                    atom,
		                    gdk_atom_intern_static_string("CARDINAL"),
		                    32,
		                    GDK_PROP_MODE_REPLACE,
		                    (unsigned char *)struts,
		                    12);
	else
		gdk_property_delete(gtk_widget_get_window(GTK_WIDGET(top)), atom);
}

static void vala_panel_platform_x11_finalize(GObject *obj)
{
	ValaPanelPlatformX11 *self = VALA_PANEL_PLATFORM_X11(obj);
	g_free(self->profile);
	(*G_OBJECT_CLASS(vala_panel_platform_x11_parent_class)->finalize)(obj);
}

static void vala_panel_platform_x11_init(ValaPanelPlatformX11 *self)
{
}

static void vala_panel_platform_x11_class_init(ValaPanelPlatformX11Class *klass)
{
	VALA_PANEL_PLATFORM_CLASS(klass)->move_to_coords = vala_panel_platform_x11_move_to_coords;
	VALA_PANEL_PLATFORM_CLASS(klass)->move_to_side   = vala_panel_platform_x11_move_to_side;
	VALA_PANEL_PLATFORM_CLASS(klass)->update_strut   = vala_panel_platform_x11_update_strut;
	VALA_PANEL_PLATFORM_CLASS(klass)->can_strut      = vala_panel_platform_x11_edge_can_strut;
	VALA_PANEL_PLATFORM_CLASS(klass)->start_panels_from_profile =
	    vala_panel_platform_x11_start_panels_from_profile;
	G_OBJECT_CLASS(klass)->finalize = vala_panel_platform_x11_finalize;
}
