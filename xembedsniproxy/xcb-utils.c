/*
 * Copyright (C) 2017 <davidedmundson@kde.org> David Edmundson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "xcb-utils.h"

#include <X11/Xlib-xcb.h>
#include <string.h>

#include "xcb-utils.h"

#include <xcb/xcb_atom.h>

/* X11 data types */
xcb_atom_t a_UTF8_STRING;
xcb_atom_t a_XROOTPMAP_ID;
/* SYSTEM TRAY spec */
xcb_atom_t a_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR;
xcb_atom_t a_NET_SYSTEM_TRAY_OPCODE;
xcb_atom_t a_NET_SYSTEM_TRAY_MESSAGE_DATA;
xcb_atom_t a_NET_SYSTEM_TRAY_ORIENTATION;
xcb_atom_t a_NET_SYSTEM_TRAY_VISUAL;
xcb_atom_t a_MANAGER;

enum
{
	I_UTF8_STRING,
	I_XROOTPMAP_ID,
	I_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR,
	I_NET_SYSTEM_TRAY_OPCODE,
	I_NET_SYSTEM_TRAY_MESSAGE_DATA,
	I_NET_SYSTEM_TRAY_ORIENTATION,
	I_NET_SYSTEM_TRAY_VISUAL,
	I_MANAGER,
	N_ATOMS
};

void resolve_atoms(xcb_connection_t *con)
{
	static const char *atom_names[N_ATOMS];

	atom_names[I_UTF8_STRING]                       = "UTF8_STRING";
	atom_names[I_XROOTPMAP_ID]                      = "_XROOTPMAP_ID";
	atom_names[I_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR] = "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR";
	atom_names[I_NET_SYSTEM_TRAY_OPCODE]            = "_NET_SYSTEM_TRAY_OPCODE";
	atom_names[I_NET_SYSTEM_TRAY_MESSAGE_DATA]      = "_NET_SYSTEM_TRAY_MESSAGE_DATA";
	atom_names[I_NET_SYSTEM_TRAY_ORIENTATION]       = "_NET_SYSTEM_TRAY_ORIENTATION";
	atom_names[I_NET_SYSTEM_TRAY_VISUAL]            = "_NET_SYSTEM_TRAY_VISUAL";
	atom_names[I_MANAGER]                           = "MANAGER";
	xcb_atom_t atoms[N_ATOMS];
	for (int i = 0; i < N_ATOMS; i++)
	{
		uint16_t len = (uint16_t)strlen(atom_names[i]);

		xcb_intern_atom_cookie_t cookie = xcb_intern_atom(con, 0, len, atom_names[i]);
		g_autofree xcb_intern_atom_reply_t *reply =
		    xcb_intern_atom_reply(con, cookie, NULL);
		/* ... do other work here if possible ... */
		if (!reply)
		{
			g_warning("Error: unable to return Atoms");
			return;
		}
		else
		{
			atoms[i] = reply->atom;
		}
	}
	a_UTF8_STRING                       = atoms[I_UTF8_STRING];
	a_XROOTPMAP_ID                      = atoms[I_XROOTPMAP_ID];
	a_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR = atoms[I_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR];
	a_NET_SYSTEM_TRAY_OPCODE            = atoms[I_NET_SYSTEM_TRAY_OPCODE];
	a_NET_SYSTEM_TRAY_MESSAGE_DATA      = atoms[I_NET_SYSTEM_TRAY_MESSAGE_DATA];
	a_NET_SYSTEM_TRAY_ORIENTATION       = atoms[I_NET_SYSTEM_TRAY_ORIENTATION];
	a_NET_SYSTEM_TRAY_VISUAL            = atoms[I_NET_SYSTEM_TRAY_VISUAL];
	a_MANAGER                           = atoms[I_MANAGER];
	return;
}

xcb_screen_t *xcb_screen_of_display(xcb_connection_t *c, int screen)
{
	xcb_screen_iterator_t iter;

	iter = xcb_setup_roots_iterator(xcb_get_setup(c));
	for (; iter.rem; --screen, xcb_screen_next(&iter))
		if (screen == 0)
			return iter.data;

	return NULL;
}

void xcb_connection_set_composited_for_xcb_window(xcb_connection_t *c, xcb_window_t win,
                                                  bool composited)
{
	xcb_screen_t *screen      = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
	xcb_visualid_t trayVisual = screen->root_visual;
	if (composited)
	{
		xcb_depth_iterator_t depth_iterator = xcb_screen_allowed_depths_iterator(screen);
		xcb_depth_t *depth                  = NULL;

		while (depth_iterator.rem)
		{
			if (depth_iterator.data->depth == 32)
			{
				depth = depth_iterator.data;
				break;
			}
			xcb_depth_next(&depth_iterator);
		}

		if (depth)
		{
			xcb_visualtype_iterator_t visualtype_iterator =
			    xcb_depth_visuals_iterator(depth);
			while (visualtype_iterator.rem)
			{
				xcb_visualtype_t *visualtype = visualtype_iterator.data;
				if (visualtype->_class == XCB_VISUAL_CLASS_TRUE_COLOR)
				{
					trayVisual = visualtype->visual_id;
					break;
				}
				xcb_visualtype_next(&visualtype_iterator);
			}
		}
	}

	xcb_change_property(c,
	                    XCB_PROP_MODE_REPLACE,
	                    win,
	                    a_NET_SYSTEM_TRAY_VISUAL,
	                    XCB_ATOM_VISUALID,
	                    32,
	                    1,
	                    &trayVisual);
}

xcb_screen_t *xcb_get_screen_for_connection(xcb_connection_t *connection, int screen_num)
{
	xcb_screen_t *screen       = NULL;
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(connection));
	for (; iter.rem; --screen_num, xcb_screen_next(&iter))
		if (screen_num == 0)
		{
			screen = iter.data;
			break;
		}
	return screen;
}

xcb_atom_t xcb_atom_get_for_connection(xcb_connection_t *connection, const char *atom_name)
{
	xcb_intern_atom_cookie_t atom_q;
	xcb_intern_atom_reply_t *atom_r;

	g_autofree char *true_atom_name = xcb_atom_name_by_screen(atom_name, 0);
	if (!true_atom_name)
		g_warning("error getting %s atom name", atom_name);

	atom_q =
	    xcb_intern_atom_unchecked(connection, false, strlen(true_atom_name), true_atom_name);

	atom_r = xcb_intern_atom_reply(connection, atom_q, NULL);
	if (!atom_r)
		g_warning("error getting %s atom", true_atom_name);

	return atom_r->atom;
}
