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

#include "application.h"
#include "config.h"
#include "gwater-xcb.h"
#include "xcb-utils.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <inttypes.h>
#include <locale.h>
#include <stdbool.h>
#include <xcb/damage.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>

struct _XEmbedSNIApplication
{
	GApplication parent;
	uint8_t damageEventBase;
	GWaterXcbSource *src;
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

bool xembed_sni_application_add_damage_watch(XEmbedSNIApplication *self, xcb_window_t client)
{
	g_debug("adding damage watch for  %u\n", client);

	xcb_connection_t *c = g_water_xcb_source_get_connection(self->src);
	const xcb_get_window_attributes_cookie_t attribsCookie =
	    xcb_get_window_attributes_unchecked(c, client);

	const xcb_window_t damageId = xcb_generate_id(c);
	g_hash_table_insert(self->damageWatches,
	                    GUINT_TO_POINTER(client),
	                    GUINT_TO_POINTER(damageId));
	xcb_damage_create(c, damageId, client, XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);

	g_autofree xcb_generic_error_t *error = NULL;
	g_autofree xcb_get_window_attributes_reply_t *attr =
	    xcb_get_window_attributes_reply(c, attribsCookie, &error);
	uint32_t events = XCB_EVENT_MASK_STRUCTURE_NOTIFY;
	if (attr)
	{
		events = events | attr->your_event_mask;
	}
	// if window is already gone, there is no need to handle it.
	if (error && error->error_code == XCB_WINDOW)
	{
		g_free(error);
		return false;
	}
	// the event mask will not be removed again. We cannot track whether another component also
	// needs STRUCTURE_NOTIFY (e.g. KWindowSystem). if we would remove the event mask again,
	// other areas will break.
	const xcb_void_cookie_t changeAttrCookie =
	    xcb_change_window_attributes_checked(c, client, XCB_CW_EVENT_MASK, &events);
	g_autofree xcb_generic_error_t *changeAttrError = xcb_request_check(c, changeAttrCookie);
	// if window is gone by this point, it will be catched by eventFilter, so no need to check
	// later errors.
	if (changeAttrError && changeAttrError->error_code == XCB_WINDOW)
	{
		return false;
	}
	return true;
}

static void xembed_sni_application_dock(XEmbedSNIApplication *self, xcb_window_t winId)
{
	g_debug("trying to dock window %u\n", winId);

	if (g_hash_table_contains(self->proxies, GUINT_TO_POINTER(winId)))
	{
		return;
	}

	if (xembed_sni_application_add_damage_watch(self, winId))
	{
		//        g_hash_table_insert(self->proxies,GUINT_TO_POINTER(winID),);
		//        m_proxies[winId] = new SNIProxy(winId, this);
	}
}

static void xembed_sni_application_undock(XEmbedSNIApplication *self, xcb_window_t winId)
{
	g_debug("trying to undock window %u\n", winId);

	if (g_hash_table_contains(self->proxies, GUINT_TO_POINTER(winId)))
	{
		return;
	}
	g_hash_table_remove(self->proxies, GUINT_TO_POINTER(winId));
}

/*XCB event filter*/

static bool xcb_event_filter(xcb_generic_event_t *event, XEmbedSNIApplication *self)
{
}

/* GDK event filter. */
// static GdkFilterReturn xembed_sni_event_filter(XEvent *xev, GdkEvent *event,
//                                               XEmbedSNIApplication *self)
//{
//	// Try to convert XEvent to xcb_event_t
//	xcb_generic_event_t *ev = (xcb_generic_event_t *)xev;
//	//    xcb_connection_t *con      = gdk_x11_get_default_xcb_connection();
//	const uint8_t responseType = ev->response_type;
//	if (responseType == XCB_CLIENT_MESSAGE)
//	{
//		const xcb_client_message_event_t *ce = (xcb_client_message_event_t *)ev;
//		if (ce->type == a_NET_SYSTEM_TRAY_OPCODE)
//		{
//			switch (ce->data.data32[1])
//			{
//			case SYSTEM_TRAY_REQUEST_DOCK:
//				xembed_sni_application_dock(self, ce->data.data32[2]);
//				return GDK_FILTER_REMOVE;
//			}
//		}
//	}
//	else if (responseType == XCB_UNMAP_NOTIFY)
//	{
//		const xcb_window_t unmappedWId = ((xcb_unmap_notify_event_t *)ev)->window;
//		if (g_hash_table_contains(self->proxies, GUINT_TO_POINTER(unmappedWId)))
//		{
//			xembed_sni_application_undock(self, unmappedWId);
//		}
//	}
//	else if (responseType == XCB_DESTROY_NOTIFY)
//	{
//		const xcb_window_t destroyedWId = ((xcb_destroy_notify_event_t *)ev)->window;
//		if (g_hash_table_contains(self->proxies, GUINT_TO_POINTER(destroyedWId)))
//		{
//			xembed_sni_application_undock(self, destroyedWId);
//		}
//	}
//	else if (responseType == self->damageEventBase + XCB_DAMAGE_NOTIFY)
//	{
//		const xcb_window_t damagedWId = ((xcb_damage_notify_event_t *)ev)->drawable;
//		GObject *sniProxy =
//		    G_OBJECT(g_hash_table_lookup(self->proxies, GUINT_TO_POINTER(damagedWId)));
//		if (sniProxy)
//		{
//			//            sniProx->update();
//			xcb_damage_subtract(gdk_x11_get_default_xcb_connection(),
//			                    GDK_POINTER_TO_XID(
//			                        g_hash_table_lookup(self->damageWatches,
//			                                            GUINT_TO_POINTER(
//			                                                damagedWId))),
//			                    XCB_NONE,
//			                    XCB_NONE);
//		}
//	}

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

//	return GDK_FILTER_CONTINUE;
//}

static void claim_systray_selection(XEmbedSNIApplication *self)
{
	xcb_connection_t *con = g_water_xcb_source_get_connection(self->src);
	g_autofree char *selection_atom_name =
	    g_strdup_printf("_NET_SYSTEM_TRAY_S%d", gdk_x11_get_default_screen());
	xcb_atom_t selection_atom =
	    (xcb_atom_t)gdk_x11_get_xatom_by_name_for_display(display, selection_atom_name);
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

static void release_systray_selection(XEmbedSNIApplication *self)
{
	if (self->selection != XCB_WINDOW_NONE)
	{
		GdkDisplay *display   = gdk_display_get_default();
		xcb_connection_t *con = gdk_x11_display_get_xcb_connection(display);
		g_autofree char *selection_atom_name =
		    g_strdup_printf("_NET_SYSTEM_TRAY_S%d", gdk_x11_get_default_screen());
		xcb_atom_t selection_atom =
		    (xcb_atom_t)gdk_x11_get_xatom_by_name_for_display(display, selection_atom_name);
		g_autofree xcb_generic_error_t *err = NULL;
		xcb_get_selection_owner_cookie_t sel_c =
		    xcb_get_selection_owner(con, selection_atom);
		g_autofree xcb_get_selection_owner_reply_t *sel_r =
		    xcb_get_selection_owner_reply(con, sel_c, &err);
		if (err)
		{
			g_printerr("tray: selection releasing error");
			g_application_quit(G_APPLICATION(self));
		}
		if (sel_r->owner == self->selection)
		{
			xcb_void_cookie_t ret                = xcb_set_selection_owner_checked(con,
                                                                                self->selection,
                                                                                selection_atom,
                                                                                XCB_CURRENT_TIME);
			g_autofree xcb_generic_error_t *oerr = xcb_request_check(con, ret);
		}
		self->selection = XCB_WINDOW_NONE;
	}
}

static void xembed_sni_application_startup(GApplication *base)
{
	XEmbedSNIApplication *self = XEMBED_SNI_APPLICATION(base);
	G_APPLICATION_CLASS(xembed_sni_application_parent_class)
	    ->startup((GApplication *)G_TYPE_CHECK_INSTANCE_CAST(self,
	                                                         g_application_get_type(),
	                                                         GApplication));
	g_application_mark_busy((GApplication *)self);
	setlocale(LC_CTYPE, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
	// We only support X11
	self->src = g_water_xcb_source_new(g_main_context_default(),
	                                   "0",
	                                   0,
	                                   (GWaterXcbEventCallback)xcb_event_filter,
	                                   self,
	                                   NULL);
	g_application_hold(base);
	xcb_connection_t *con = g_water_xcb_source_get_connection(self->src);
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
	//	gdk_window_add_filter(NULL, (GdkFilterFunc)xembed_sni_event_filter, self);
}

static void xembed_sni_application_shutdown(GApplication *base)
{
	XEmbedSNIApplication *self = XEMBED_SNI_APPLICATION(base);
	//	gdk_window_remove_filter(NULL, (GdkFilterFunc)xembed_sni_event_filter, self);
	release_systray_selection(self);
	G_APPLICATION_CLASS(xembed_sni_application_parent_class)
	    ->shutdown((GApplication *)G_TYPE_CHECK_INSTANCE_CAST(base,
	                                                          g_application_get_type(),
	                                                          GApplication));
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
