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

#define gravity_from_edge(e)                                                                       \
	e == GTK_POS_TOP ? GDK_GRAVITY_NORTH                                                       \
	                 : (GTK_POS_BOTTOM ? GDK_GRAVITY_SOUTH                                     \
	                                   : (GTK_POS_LEFT ? GDK_GRAVITY_WEST : GDK_GRAVITY_EAST))

static void vala_panel_platform_x11_default_init(ValaPanelPlatform *iface);
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
	GSettingsBackend *backend   = core->backend;
	g_autoptr(GSettings) s =
	    g_settings_new_with_backend_and_path(core->root_schema, backend, core->root_schema);
	g_auto(GStrv) panels = g_settings_get_strv(s, VALA_PANEL_APPLICATION_PANELS);
	for (int i = 0; panels[i] != NULL; i++)
	{
#ifdef ENABLE_NEW_INTERFACE
		ValaPanelToplevel *unit =
		    vala_panel_toplevel_new_with_platform(self->app, obj, panels[i]);
#endif
		gtk_application_add_window(app, GTK_WINDOW(unit));
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
static ulong vala_panel_platform_x11_edge_can_strut(ValaPanelPlatform *f, GtkWindow *top)
{
	ulong s = 0;
	if (!gtk_widget_get_mapped(GTK_WIDGET(top)))
		return 0;
	bool autohide         = false;
	GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
	GtkAllocation a;
	gtk_widget_get_allocation(GTK_WIDGET(top), &a);
	g_object_get(top,
	             VALA_PANEL_KEY_AUTOHIDE,
	             autohide,
	             VALA_PANEL_KEY_ORIENTATION,
	             orient,
	             NULL);
	if (autohide)
		s = GAP;
	else
		switch (orient)
		{
		case GTK_ORIENTATION_VERTICAL:
			s = (uint)a.width;
			break;
		case GTK_ORIENTATION_HORIZONTAL:
			s = (uint)a.height;
			break;
		default:
			return 0;
		}
	int monitor;
	g_object_get(top, VALA_PANEL_KEY_MONITOR, &monitor, NULL);
	if (monitor < 0)
		return s;
	if (monitor >=
	    gdk_display_get_n_monitors(gdk_screen_get_display(gtk_window_get_screen(top))))
		return false;
	GdkRectangle rect, rect2;
	gdk_monitor_get_geometry(gdk_display_get_monitor(gdk_screen_get_display(
	                                                     gtk_window_get_screen(top)),
	                                                 monitor),
	                         &rect);
	GtkPositionType edge;
	g_object_get(top, VALA_PANEL_KEY_EDGE, &edge, NULL);
	switch (edge)
	{
	case GTK_POS_LEFT:
		rect.width = rect.x;
		rect.x     = 0;
		s += rect.width;
		break;
	case GTK_POS_RIGHT:
		rect.x += rect.width;
		rect.width = gdk_screen_get_width(gtk_widget_get_screen(GTK_WIDGET(top))) - rect.x;
		s += rect.width;
		break;
	case GTK_POS_TOP:
		rect.height = rect.y;
		rect.y      = 0;
		s += rect.height;
		break;
	case GTK_POS_BOTTOM:
		rect.y += rect.height;
		rect.height =
		    gdk_screen_get_height(gtk_widget_get_screen(GTK_WIDGET(top))) - rect.y;
		s += rect.height;
		break;
	}
	if (!(rect.height == 0 || rect.width == 0)) /* on a border of monitor */
	{
		int n =
		    gdk_display_get_n_monitors(gdk_screen_get_display(gtk_window_get_screen(top)));
		for (int i = 0; i < n; i++)
		{
			if (i == monitor)
				continue;
			gdk_monitor_get_geometry(gdk_display_get_monitor(gdk_screen_get_display(
			                                                     gtk_window_get_screen(
			                                                         top)),
			                                                 i),
			                         &rect2);
			if (gdk_rectangle_intersect(&rect, &rect2, NULL))
				/* that monitor lies over the edge */
				return 0;
		}
	}
	return s;
}

static void vala_panel_platform_x11_update_strut(ValaPanelPlatform *f, GtkWindow *top)
{
	int index;
	GdkAtom atom;
	ulong strut_size  = 0;
	ulong strut_lower = 0;
	ulong strut_upper = 0;
	bool autohide;
	GtkPositionType edge;

	g_object_get(top, VALA_PANEL_KEY_AUTOHIDE, &autohide, VALA_PANEL_KEY_EDGE, &edge, NULL);

	if (!gtk_widget_get_mapped(GTK_WIDGET(top)))
		return;
	/* most wm's tend to ignore struts of unmapped windows, and that's how
	 * panel hides itself. so no reason to set it. If it was be, it must be removed */
	/*this.strut_size == 0, we must take it from toplevel */
	if (autohide && !gtk_widget_get_mapped(GTK_WIDGET(top)))
		return;
	GtkAllocation a;
	gtk_widget_get_allocation(GTK_WIDGET(top), &a);
	//    /* Dispatch on edge to set up strut parameters. */
	switch (edge)
	{
	case GTK_POS_LEFT:
		index       = 0;
		strut_lower = a.y;
		strut_upper = a.y + a.height;
		break;
	case GTK_POS_RIGHT:
		index       = 1;
		strut_lower = a.y;
		strut_upper = a.y + a.height;
		break;
	case GTK_POS_TOP:
		index       = 2;
		strut_lower = a.x;
		strut_upper = a.x + a.width;
		break;
	case GTK_POS_BOTTOM:
		index       = 3;
		strut_lower = a.x;
		strut_upper = a.x + a.width;
		break;
	default:
		return;
	}

	//    /* Set up strut value in property format. */
	ulong desired_strut[12];
	strut_size = vala_panel_platform_x11_edge_can_strut(f, top);
	if (strut_size > 0)
	{
		desired_strut[index]         = strut_size;
		desired_strut[4 + index * 2] = strut_lower;
		desired_strut[5 + index * 2] = strut_upper - 1;
	}
	//    /* If strut value changed, set the property value on the panel window.
	//     * This avoids property change traffic when the panel layout is recalculated but strut
	//     geometry hasn't changed. */
	//    if ((this.strut_size != strut_size) || (this.strut_lower != strut_lower) ||
	//    (this.strut_upper
	//    != strut_upper) || (this.strut_edge != this.edge))
	{
		//            this.strut_size = strut_size;
		//            this.strut_lower = strut_lower;
		//            this.strut_upper = strut_upper;
		//            this.strut_edge = this.edge;
		/* If window manager supports STRUT_PARTIAL, it will ignore STRUT.
		 * Set STRUT also for window managers that do not support STRUT_PARTIAL. */
		GdkWindow *xwin = gtk_widget_get_window(GTK_WIDGET(top));
		if (strut_size != 0)
		{
			atom = gdk_atom_intern_static_string("_NET_WM_STRUT_PARTIAL");
			gdk_property_change(xwin,
			                    atom,
			                    gdk_atom_intern_static_string("CARDINAL"),
			                    32,
			                    GDK_PROP_MODE_REPLACE,
			                    (u_int8_t *)desired_strut,
			                    12);
			atom = gdk_atom_intern_static_string("_NET_WM_STRUT");
			gdk_property_change(xwin,
			                    atom,
			                    gdk_atom_intern_static_string("CARDINAL"),
			                    32,
			                    GDK_PROP_MODE_REPLACE,
			                    (u_int8_t *)desired_strut,
			                    4);
		}
		else
		{
			atom = gdk_atom_intern_static_string("_NET_WM_STRUT_PARTIAL");
			gdk_property_delete(xwin, atom);
			atom = gdk_atom_intern_static_string("_NET_WM_STRUT");
			gdk_property_delete(xwin, atom);
		}
	}
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
	G_OBJECT_CLASS(klass)->finalize                  = vala_panel_platform_x11_finalize;
	VALA_PANEL_PLATFORM_CLASS(klass)->move_to_coords = vala_panel_platform_x11_move_to_coords;
	VALA_PANEL_PLATFORM_CLASS(klass)->move_to_side   = vala_panel_platform_x11_move_to_side;
	VALA_PANEL_PLATFORM_CLASS(klass)->update_strut   = vala_panel_platform_x11_update_strut;
	VALA_PANEL_PLATFORM_CLASS(klass)->can_strut      = vala_panel_platform_x11_edge_can_strut;
	VALA_PANEL_PLATFORM_CLASS(klass)->start_panels_from_profile =
	    vala_panel_platform_x11_start_panels_from_profile;
}
