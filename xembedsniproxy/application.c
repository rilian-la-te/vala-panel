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

#include "application.h"
#include "config.h"
#include "xcb-utils.h"

#include <glib/gi18n-lib.h>
#include <gtk/gtkx.h>
#include <inttypes.h>
#include <locale.h>
#include <stdbool.h>
#include <xcb/damage.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>

struct _XEmbedSNIApplication
{
	GtkApplication parent;
	uint8_t damageEventBase;
	xcb_window_t selection;
	GHashTable *damageWatches;
	GHashTable *proxies;
};

G_DEFINE_TYPE(XEmbedSNIApplication, xembed_sni_application, GTK_TYPE_APPLICATION)

static const GOptionEntry entries[] =
    { { "version", 'v', 0, G_OPTION_ARG_NONE, NULL, N_("Print version and exit"), NULL },
      { NULL } };

enum
{
	XEMBED_SNI_APP_DUMMY_PROPERTY,
	XEMBED_SNI_APP_ALL
};

XEmbedSNIApplication *xembed_sni_application_new()
{
	return XEMBED_SNI_APPLICATION(g_object_new(xembed_sni_application_get_type(),
	                                           "application-id",
	                                           "org.valapanel.xembedsni",
	                                           "flags",
	                                           G_APPLICATION_HANDLES_COMMAND_LINE,
	                                           "resource-base-path",
	                                           "/org/vala-panel/xembedsni",
	                                           NULL));
}

static void xembed_sni_application_init(XEmbedSNIApplication *self)
{
	self->damageWatches = g_hash_table_new(g_direct_hash, g_direct_equal);
	self->proxies = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_object_unref);
	self->selection = XCB_WINDOW_NONE;
	g_application_add_main_option_entries(G_APPLICATION(self), entries);
}

/* GDK event filter. */
static GdkFilterReturn xembed_sni_event_filter(XEvent *xev, GdkEvent *event,
                                               XEmbedSNIApplication *self)
{
	// Try to convert XEvent to xcb_event_t
	xcb_generic_event_t *ev    = (xcb_generic_event_t *)xev;
	const uint8_t responseType = ev->response_type;
	if (responseType == XCB_CLIENT_MESSAGE)
	{
		const xcb_client_message_event_t *ce = (xcb_client_message_event_t *)ev;
		if (ce->type == a_NET_SYSTEM_TRAY_OPCODE)
		{
			switch (ce->data.data32[1])
			{
			case SYSTEM_TRAY_REQUEST_DOCK:
				//                    dock(ce->data.data32[2]);
				return GDK_FILTER_REMOVE;
			}
		}
	}
	else if (responseType == XCB_UNMAP_NOTIFY)
	{
		const xcb_window_t unmappedWId = ((xcb_unmap_notify_event_t *)ev)->window;
		if (g_hash_table_contains(self->proxies, GDK_XID_TO_POINTER(unmappedWId)))
		{
			//            undock(unmappedWId);
		}
	}
	else if (responseType == XCB_DESTROY_NOTIFY)
	{
		const xcb_window_t destroyedWId = ((xcb_destroy_notify_event_t *)ev)->window;
		if (g_hash_table_contains(self->proxies, GDK_XID_TO_POINTER(destroyedWId)))
		{
			//            undock(destroyedWId);
		}
	}
	else if (responseType == self->damageEventBase + XCB_DAMAGE_NOTIFY)
	{
		const xcb_window_t damagedWId = ((xcb_damage_notify_event_t *)ev)->drawable;
		GObject *sniProxy =
		    G_OBJECT(g_hash_table_lookup(self->proxies, GDK_XID_TO_POINTER(damagedWId)));
		if (sniProxy)
		{
			//            sniProx->update();
			//            xcb_damage_subtract(QX11Info::connection(),
			//            m_damageWatches[damagedWId], XCB_NONE, XCB_NONE);
		}
	}

	//    if (xev->type == DestroyNotify)
	//    {
	//        /* Look for DestroyNotify events on tray icon windows and update state.
	//         * We do it this way rather than with a "plug_removed" event because delivery
	//         * of plug_removed events is observed to be unreliable if the client
	//         * disconnects within less than 10 ms. */
	//        XDestroyWindowEvent *xev_destroy = (XDestroyWindowEvent *)xev;
	//        TrayClient *tc                   = client_lookup(tr, xev_destroy->window);
	//        if (tc != NULL)
	//            client_delete(tr, tc, TRUE, TRUE);
	//    }

	//    else if (xev->type == ClientMessage)
	//    {
	//        if (xev->xclient.message_type == a_NET_SYSTEM_TRAY_OPCODE)
	//        {
	//            /* Client message of type _NET_SYSTEM_TRAY_OPCODE.
	//             * Dispatch on the request. */
	//            switch (xev->xclient.data.l[1])
	//            {
	//            case SYSTEM_TRAY_REQUEST_DOCK:
	//                /* If a Request Dock event on the invisible window, which is holding
	//                 * the manager selection, execute it. */
	//                if (xev->xclient.window == tr->invisible_window)
	//                {
	//                    trayclient_request_dock(tr, (XClientMessageEvent *)xev);
	//                    return GDK_FILTER_REMOVE;
	//                }
	//                break;

	//            case SYSTEM_TRAY_BEGIN_MESSAGE:
	//                /* If a Begin Message event. look up the tray icon and execute it.
	//                 */
	//                balloon_message_begin_event(tr, (XClientMessageEvent *)xev);
	//                return GDK_FILTER_REMOVE;

	//            case SYSTEM_TRAY_CANCEL_MESSAGE:
	//                /* If a Cancel Message event. look up the tray icon and execute it.
	//                 */
	//                balloon_message_cancel_event(tr, (XClientMessageEvent *)xev);
	//                return GDK_FILTER_REMOVE;
	//            }
	//        }

	//        else if (xev->xclient.message_type == a_NET_SYSTEM_TRAY_MESSAGE_DATA)
	//        {
	//            /* Client message of type _NET_SYSTEM_TRAY_MESSAGE_DATA.
	//             * Look up the tray icon and execute it. */
	//            balloon_message_data_event(tr, (XClientMessageEvent *)xev);
	//            return GDK_FILTER_REMOVE;
	//        }
	//    }

	//    else if ((xev->type == SelectionClear) && (xev->xclient.window ==
	//    tr->invisible_window))
	//    {
	//        /* Look for SelectionClear events on the invisible window, which is holding the
	//         * manager selection.
	//         * This should not happen. */
	//        tray_unmanage_selection(tr);
	//    }

	return GDK_FILTER_CONTINUE;
}

static void claim_systray_selection(XEmbedSNIApplication *self)
{
	GdkDisplay *display   = gdk_display_get_default();
	xcb_connection_t *con = gdk_x11_display_get_xcb_connection(display);
	g_autofree char *selection_atom_name =
	    g_strdup_printf("_NET_SYSTEM_TRAY_S%d", gdk_x11_get_default_screen());
	xcb_atom_t selection_atom =
	    gdk_x11_get_xatom_by_name_for_display(display, selection_atom_name);
	g_autofree xcb_generic_error_t *err    = NULL;
	xcb_get_selection_owner_cookie_t sel_c = xcb_get_selection_owner(con, selection_atom);
	g_autofree xcb_get_selection_owner_reply_t *sel_r =
	    xcb_get_selection_owner_reply(con, sel_c, &err);
	if (err)
	{
		g_printerr("tray: selection claiming error");
		g_application_quit(G_APPLICATION(self));
	}
	/* If the selection is already owned, there is another tray running. */
	else if (sel_r->owner != XCB_WINDOW_NONE)
	{
		g_printerr("tray: another systray already running");
		g_application_quit(G_APPLICATION(self));
	}
	/* Create an invisible window to hold the selection. */
	uint32_t values[] = { true,
		              XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY };
	self->selection   = xcb_generate_id(con);
	xcb_window_t root = (xcb_window_t)gdk_x11_get_default_root_xwindow();
	xcb_create_window(con,
	                  XCB_COPY_FROM_PARENT,
	                  self->selection,
	                  root,
	                  0,
	                  0,
	                  1,
	                  1,
	                  0,
	                  XCB_WINDOW_CLASS_INPUT_ONLY,
	                  XCB_COPY_FROM_PARENT,
	                  XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK,
	                  values);
	xcb_icccm_set_wm_name(con,
	                      self->selection,
	                      XCB_ATOM_STRING,
	                      8,
	                      sizeof(GETTEXT_PACKAGE) - 1,
	                      GETTEXT_PACKAGE);
	xcb_icccm_set_wm_class(con, self->selection, sizeof(GETTEXT_PACKAGE), GETTEXT_PACKAGE);
	xcb_client_message_event_t ev = { 0 };
	/* Fill event */
	ev.response_type  = XCB_CLIENT_MESSAGE;
	ev.window         = root;
	ev.format         = 32;
	ev.type           = a_MANAGER;
	ev.data.data32[0] = XCB_CURRENT_TIME;
	ev.data.data32[1] = selection_atom;
	ev.data.data32[2] = self->selection;
	ev.data.data32[3] = ev.data.data32[4] = 0 /* manager specific data */;

	xcb_void_cookie_t ret =
	    xcb_set_selection_owner_checked(con, self->selection, selection_atom, XCB_CURRENT_TIME);
	g_autofree xcb_generic_error_t *oerr = xcb_request_check(con, ret);
	if (!oerr)
	{
		/* Send MANAGER client event (ICCCM). XCB_CURRENT_TIME is bad, but I do not know how
		 * to avoid it*/
		xcb_send_event(con, false, root, 0xFFFFFF, (char *)&ev);
		/* Set the orientation property.
		 * We always set "horizontal" since even vertical panels are designed to use a lot
		 * of width. */
		uint32_t data = SYSTEM_TRAY_ORIENTATION_HORZ;
		xcb_change_property(con,
		                    XCB_PROP_MODE_REPLACE,
		                    self->selection,
		                    a_NET_SYSTEM_TRAY_ORIENTATION,
		                    XCB_ATOM_CARDINAL,
		                    32,
		                    1,
		                    &data);
	}
	else
	{
		xcb_unmap_window(con, self->selection);
		g_printerr("tray: System tray didn't get the system tray manager selection\n");
		g_application_quit(G_APPLICATION(self));
	}
}

static void xembed_sni_application_startup(GApplication *base)
{
	XEmbedSNIApplication *self = XEMBED_SNI_APPLICATION(base);
	G_APPLICATION_CLASS(xembed_sni_application_parent_class)
	    ->startup((GApplication *)G_TYPE_CHECK_INSTANCE_CAST(self,
	                                                         gtk_application_get_type(),
	                                                         GtkApplication));
	g_application_mark_busy((GApplication *)self);
	setlocale(LC_CTYPE, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
	// We only support X11
	GdkDisplay *display = gdk_display_get_default();
	if (!GDK_IS_X11_DISPLAY(display))
		g_application_quit(base);
	g_application_hold(base);
	xcb_connection_t *con = gdk_x11_display_get_xcb_connection(display);
	xcb_prefetch_extension_data(con, &xcb_damage_id);
	const xcb_query_extension_reply_t *reply = xcb_get_extension_data(con, &xcb_damage_id);
	if (reply->present)
	{
		self->damageEventBase = reply->first_event;
		xcb_damage_query_version_unchecked(con,
		                                   XCB_DAMAGE_MAJOR_VERSION,
		                                   XCB_DAMAGE_MINOR_VERSION);
	}
	else
	{
		// no XDamage means
		g_critical("could not load damage extension. Quitting");
		g_application_quit(base);
	}
	resolve_atoms(con);
	claim_systray_selection(self);
	/* Add GDK event filter. */
	gdk_window_add_filter(NULL, (GdkFilterFunc)xembed_sni_event_filter, self);
}

static void xembed_sni_application_shutdown(GApplication *base)
{
	G_APPLICATION_CLASS(xembed_sni_application_parent_class)
	    ->shutdown((GApplication *)G_TYPE_CHECK_INSTANCE_CAST(base,
	                                                          gtk_application_get_type(),
	                                                          GtkApplication));
}

static gint xembed_sni_app_handle_local_options(GApplication *application, GVariantDict *options)
{
	if (g_variant_dict_contains(options, "version"))
	{
		g_print(_("%s - Version %s\n"), g_get_application_name(), VERSION);
		return 0;
	}
	return -1;
}

static int xembed_sni_app_command_line(GApplication *application,
                                       GApplicationCommandLine *commandline)
{
	g_autofree gchar *profile_name = NULL;
	g_autofree gchar *ccommand     = NULL;
	GVariantDict *options          = g_application_command_line_get_options_dict(commandline);
	return 0;
}

void xembed_sni_application_activate(GApplication *app)
{
	XEmbedSNIApplication *self = XEMBED_SNI_APPLICATION(app);
}

static void xembed_sni_app_finalize(GObject *object)
{
	XEmbedSNIApplication *app = XEMBED_SNI_APPLICATION(object);
	g_clear_pointer(&app->damageWatches, g_hash_table_destroy);
	g_clear_pointer(&app->proxies, g_hash_table_destroy);
	(*G_OBJECT_CLASS(xembed_sni_application_parent_class)->finalize)(object);
}

static void xembed_sni_app_set_property(GObject *object, guint prop_id, const GValue *value,
                                        GParamSpec *pspec)
{
	XEmbedSNIApplication *app;
	g_return_if_fail(XEMBED_SNI_IS_APPLICATION(object));

	app = XEMBED_SNI_APPLICATION(object);

	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void xembed_sni_app_get_property(GObject *object, guint prop_id, GValue *value,
                                        GParamSpec *pspec)
{
	XEmbedSNIApplication *app;
	g_return_if_fail(XEMBED_SNI_IS_APPLICATION(object));

	app = XEMBED_SNI_APPLICATION(object);

	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void xembed_sni_application_class_init(XEmbedSNIApplicationClass *klass)
{
	xembed_sni_application_parent_class                = g_type_class_peek_parent(klass);
	((GApplicationClass *)klass)->startup              = xembed_sni_application_startup;
	((GApplicationClass *)klass)->shutdown             = xembed_sni_application_shutdown;
	((GApplicationClass *)klass)->activate             = xembed_sni_application_activate;
	((GApplicationClass *)klass)->handle_local_options = xembed_sni_app_handle_local_options;
	((GApplicationClass *)klass)->command_line         = xembed_sni_app_command_line;
	G_OBJECT_CLASS(klass)->get_property                = xembed_sni_app_get_property;
	G_OBJECT_CLASS(klass)->set_property                = xembed_sni_app_set_property;
	G_OBJECT_CLASS(klass)->finalize                    = xembed_sni_app_finalize;
}

int main(int argc, char *argv[])
{
	XEmbedSNIApplication *app = xembed_sni_application_new();
	return g_application_run(G_APPLICATION(app), argc, argv);
}
