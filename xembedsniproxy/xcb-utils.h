#ifndef XCBUTILS_H
#define XCBUTILS_H

#include <gdk/gdkx.h>
#include <xcb/xcb.h>

/* X11 data types */
extern xcb_atom_t a_UTF8_STRING;
extern xcb_atom_t a_XROOTPMAP_ID;
/* SYSTEM TRAY spec */
extern xcb_atom_t a_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR;
extern xcb_atom_t a_NET_SYSTEM_TRAY_OPCODE;
extern xcb_atom_t a_NET_SYSTEM_TRAY_MESSAGE_DATA;
extern xcb_atom_t a_NET_SYSTEM_TRAY_ORIENTATION;
extern xcb_atom_t a_MANAGER;

/* SYSTEM TRAY Protocol constants. */
#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

#define SYSTEM_TRAY_ORIENTATION_HORZ 0
#define SYSTEM_TRAY_ORIENTATION_VERT 1

void resolve_atoms(xcb_connection_t *con);
xcb_connection_t *gdk_x11_display_get_xcb_connection(GdkX11Display *display);
xcb_screen_t *xcb_screen_of_display(xcb_connection_t *c, int screen);

#endif // XCBUTILS_H
