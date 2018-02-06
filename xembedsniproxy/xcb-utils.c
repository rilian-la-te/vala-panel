#include "xcb-utils.h"

#include <X11/Xlib-xcb.h>
#include <string.h>

#include "xcb-utils.h"

/* X11 data types */
xcb_atom_t a_UTF8_STRING;
xcb_atom_t a_XROOTPMAP_ID;
/* SYSTEM TRAY spec */
xcb_atom_t a_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR;
xcb_atom_t a_NET_SYSTEM_TRAY_OPCODE;
xcb_atom_t a_NET_SYSTEM_TRAY_MESSAGE_DATA;
xcb_atom_t a_NET_SYSTEM_TRAY_ORIENTATION;
xcb_atom_t a_MANAGER;

enum
{
	I_UTF8_STRING,
	I_XROOTPMAP_ID,
	I_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR,
	I_NET_SYSTEM_TRAY_OPCODE,
	I_NET_SYSTEM_TRAY_MESSAGE_DATA,
	I_NET_SYSTEM_TRAY_ORIENTATION,
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

xcb_connection_t *gdk_x11_display_get_xcb_connection(GdkX11Display *display)
{
	Display *xd           = GDK_DISPLAY_XDISPLAY(display);
	xcb_connection_t *con = XGetXCBConnection(xd);
	return con;
}
