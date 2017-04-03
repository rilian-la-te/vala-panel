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

#include "vala-panel-platform-standalone-x11.h"
#include "gio/gsettingsbackend.h"
#include "lib/applets-new/applet-api.h"
#include "lib/c-lib/toplevel.h"
#include "lib/definitions.h"

struct _ValaPanelPlatformX11
{
	GObject __parent__;
	char *profile;
};

#define g_key_file_load_from_config(f, p)                                                          \
	g_key_file_load_from_file(f,                                                               \
	                          _user_config_file_name(GETTEXT_PACKAGE, p, NULL),                \
	                          G_KEY_FILE_KEEP_COMMENTS,                                        \
	                          NULL)

#define gravity_from_edge(e)                                                                       \
	e == GTK_POS_TOP ? GDK_GRAVITY_NORTH                                                       \
	                 : (GTK_POS_BOTTOM ? GDK_GRAVITY_SOUTH                                     \
	                                   : (GTK_POS_LEFT ? GDK_GRAVITY_WEST : GDK_GRAVITY_EAST))

static void vala_panel_platform_x11_default_init(ValaPanelPlatformInterface *iface);
G_DEFINE_TYPE_WITH_CODE(ValaPanelPlatformX11, vala_panel_platform_x11, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(vala_panel_platform_get_type(),
                                              vala_panel_platform_x11_default_init))

ValaPanelPlatformX11 *vala_panel_platform_x11_new(const char *profile)
{
	ValaPanelPlatformX11 *pl =
	    VALA_PANEL_PLATFORM_X11(g_object_new(vala_panel_platform_x11_get_type(), NULL));
	pl->profile = g_strdup(profile);
	return pl;
}

static GSettings *vala_panel_platform_x11_get_settings_for_scheme(ValaPanelPlatform *obj,
                                                                  const char *scheme,
                                                                  const char *path)
{
	ValaPanelPlatformX11 *self = VALA_PANEL_PLATFORM_X11(obj);
	g_autoptr(GSettingsBackend) backend =
	    g_keyfile_settings_backend_new(_user_config_file_name(GETTEXT_PACKAGE,
	                                                          self->profile,
	                                                          NULL),
	                                   DEFAULT_PLUGIN_PATH,
	                                   "main-settings");
	return g_settings_new_with_backend_and_path(scheme, backend, path);
}

static void vala_panel_platform_x11_remove_settings_path(ValaPanelPlatform *obj, const char *path,
                                                         const char *child_name)
{
	ValaPanelPlatformX11 *self = VALA_PANEL_PLATFORM_X11(obj);
	g_autoptr(GKeyFile) f      = g_key_file_new();
	g_key_file_load_from_config(f, self->profile);
	if (g_key_file_has_group(f, child_name))
	{
		g_key_file_remove_group(f, child_name, NULL);
		g_key_file_save_to_file(f,
		                        _user_config_file_name(GETTEXT_PACKAGE,
		                                               self->profile,
		                                               NULL),
		                        NULL);
	}
}

static bool vala_panel_platform_x11_start_panels_from_profile(ValaPanelPlatform *obj,
                                                              GtkApplication *app,
                                                              const char *profile)
{
	ValaPanelPlatformX11 *self = VALA_PANEL_PLATFORM_X11(obj);
	g_autoptr(GKeyFile) f      = g_key_file_new();
	g_key_file_load_from_config(f, self->profile);
	g_autoptr(GSettingsBackend) backend =
	    g_keyfile_settings_backend_new(_user_config_file_name(GETTEXT_PACKAGE,
	                                                          self->profile,
	                                                          NULL),
	                                   DEFAULT_PLUGIN_PATH,
	                                   "main-settings");
	g_autoptr(GSettings) s =
	    g_settings_new_with_backend_and_path(VALA_PANEL_APPLICATION_SETTINGS,
	                                         backend,
	                                         DEFAULT_PLUGIN_PATH);
	g_autoptr(GSettings) settings = g_settings_get_child(s, profile);
	g_auto(GStrv) panels = g_settings_get_strv(settings, VALA_PANEL_APPLICATION_PANELS);
	for (int i = 0; panels[i] != NULL; i++)
	{
		//		ValaPanelToplevelUnit *unit =
		// vala_panel_toplevel_unit_new_from_uid(app, panels[i]);
		//		gtk_application_add_window(app, GTK_WINDOW(unit));
	}
	return true;
}

static void vala_panel_platform_x11_move_to_coords(ValaPanelPlatform *f, GtkWindow *top, int x,
                                                   int y)
{
	gtk_window_move(top, x, y);
}

static void vala_panel_platform_x11_move_to_side(ValaPanelPlatform *f, GtkWindow *top,
                                                 GtkPositionType alloc)
{
	gtk_window_set_gravity(top, gravity_from_edge(alloc));
	gtk_window_move(top, 0, 0);
}

// static bool panel_edge_can_strut(ValaPanelPlatform *f, ValaPanelToplevelUnit *top, unsigned long*
// size)
//{
//    ulong s = 0;
//    size = 0;
//    if (!gtk_widget_get_mapped(GTK_WIDGET(top)))
//        return false;
//    bool autohide;
//    GtkOrientation orient;
//    g_object_get(top,"autohide",autohide,"orientation",orient,NULL);
//    if (autohide)
//        s = GAP;
//    else switch (orient)
//    {
//        case GTK_ORIENTATION_VERTICAL:
//            s = a.width;
//            break;
//        case GTK_ORIENTATION_HORIZONTAL:
//            s = a.height;
//            break;
//        default: return false;
//    }
//    if (monitor < 0)
//    {
//        *size = s;
//        return true;
//    }
//    if (monitor >= gdk_screen_get_n_monitors(gtk_window_get_screen(GTK_WINDOW(top))));
//        return false;
//    GdkRectangle rect, rect2;
//    get_screen().get_monitor_geometry(monitor, out rect);
//    switch(edge)
//    {
//        case PositionType.LEFT:
//            rect.width = rect.x;
//            rect.x = 0;
//            s += rect.width;
//            break;
//        case PositionType.RIGHT:
//            rect.x += rect.width;
//            rect.width = get_screen().get_width() - rect.x;
//            s += rect.width;
//            break;
//        case PositionType.TOP:
//            rect.height = rect.y;
//            rect.y = 0;
//            s += rect.height;
//            break;
//        case PositionType.BOTTOM:
//            rect.y += rect.height;
//            rect.height = get_screen().get_height() - rect.y;
//            s += rect.height;
//            break;
//    }
//    if (!(rect.height == 0 || rect.width == 0)) /* on a border of monitor */
//    {
//        var n = get_screen().get_n_monitors();
//        for (var i = 0; i < n; i++)
//        {
//            if (i == monitor)
//                continue;
//            get_screen().get_monitor_geometry(i, out rect2);
//            if (rect.intersect(rect2, null))
//                /* that monitor lies over the edge */
//                return false;
//        }
//    }
//    size = s;
//    return true;
//}
// static void update_strut(ValaPanelPlatform *f, ValaPanelToplevelUnit *top)
//{
//    int index;
//    GdkAtom atom;
//    ulong strut_size = 0;
//    ulong strut_lower = 0;
//    ulong strut_upper = 0;

//    if (!gtk_widget_get_mapped(GTK_WIDGET(top)))
//        return;
//    /* most wm's tend to ignore struts of unmapped windows, and that's how
//     * panel hides itself. so no reason to set it. If it was be, it must be removed */
//    if (autohide && this.strut_size == 0)
//        return;

//    /* Dispatch on edge to set up strut parameters. */
//    switch (edge)
//    {
//        case PositionType.LEFT:
//            index = 0;
//            strut_lower = a.y;
//            strut_upper = a.y + a.height;
//            break;
//        case PositionType.RIGHT:
//            index = 1;
//            strut_lower = a.y;
//            strut_upper = a.y + a.height;
//            break;
//        case PositionType.TOP:
//            index = 2;
//            strut_lower = a.x;
//            strut_upper = a.x + a.width;
//            break;
//        case PositionType.BOTTOM:
//            index = 3;
//            strut_lower = a.x;
//            strut_upper = a.x + a.width;
//            break;
//        default:
//            return;
//    }

//    /* Set up strut value in property format. */
//    ulong desired_strut[12];
//    if (strut &&
//        panel_edge_can_strut(out strut_size))
//    {
//        desired_strut[index] = strut_size;
//        desired_strut[4 + index * 2] = strut_lower;
//        desired_strut[5 + index * 2] = strut_upper-1;
//    }
//    /* If strut value changed, set the property value on the panel window.
//     * This avoids property change traffic when the panel layout is recalculated but strut
//     geometry hasn't changed. */
//    if ((this.strut_size != strut_size) || (this.strut_lower != strut_lower) || (this.strut_upper
//    != strut_upper) || (this.strut_edge != this.edge))
//    {
//        this.strut_size = strut_size;
//        this.strut_lower = strut_lower;
//        this.strut_upper = strut_upper;
//        this.strut_edge = this.edge;
//        /* If window manager supports STRUT_PARTIAL, it will ignore STRUT.
//         * Set STRUT also for window managers that do not support STRUT_PARTIAL. */
//        var xwin = get_window();
//        if (strut_size != 0)
//        {
//            atom = Atom.intern_static_string("_NET_WM_STRUT_PARTIAL");
//            Gdk.property_change(xwin,atom,Atom.intern_static_string("CARDINAL"),32,Gdk.PropMode.REPLACE,(uint8[])desired_strut,12);
//            atom = Atom.intern_static_string("_NET_WM_STRUT");
//            Gdk.property_change(xwin,atom,Atom.intern_static_string("CARDINAL"),32,Gdk.PropMode.REPLACE,(uint8[])desired_strut,4);
//        }
//        else
//        {
//            atom = Atom.intern_static_string("_NET_WM_STRUT_PARTIAL");
//            Gdk.property_delete(xwin,atom);
//            atom = Atom.intern_static_string("_NET_WM_STRUT");
//            Gdk.property_delete(xwin,atom);
//        }
//    }
//}

static void vala_panel_platform_x11_default_init(ValaPanelPlatformInterface *iface)
{
	iface->move_to_coords            = vala_panel_platform_x11_move_to_coords;
	iface->move_to_side              = vala_panel_platform_x11_move_to_side;
	iface->get_settings_for_scheme   = vala_panel_platform_x11_get_settings_for_scheme;
	iface->remove_settings_path      = vala_panel_platform_x11_remove_settings_path;
	iface->start_panels_from_profile = vala_panel_platform_x11_start_panels_from_profile;
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
	G_OBJECT_CLASS(klass)->finalize = vala_panel_platform_x11_finalize;
}
