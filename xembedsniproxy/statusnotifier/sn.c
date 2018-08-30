#include "sn.h"
#include "../xcb-utils.h"
#include "../xtestsender.h"
#include "closures.h"
#include "interfaces.h"
#include "sni-enums.h"
#include <cairo/cairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <unistd.h>

#define _UNUSED_ __attribute__((unused))

enum
{
	PROP_0,

	PROP_ID,
	PROP_TITLE,
	PROP_CATEGORY,
	PROP_STATUS,
	PROP_MAIN_ICON_NAME,
	PROP_MAIN_ICON_PIXBUF,
	PROP_OVERLAY_ICON_NAME,
	PROP_OVERLAY_ICON_PIXBUF,
	PROP_ATTENTION_ICON_NAME,
	PROP_ATTENTION_ICON_PIXBUF,
	PROP_ATTENTION_MOVIE_NAME,
	PROP_TOOLTIP_ICON_NAME,
	PROP_TOOLTIP_ICON_PIXBUF,
	PROP_TOOLTIP_TITLE,
	PROP_TOOLTIP_BODY,
	PROP_ITEM_IS_MENU,
	PROP_MENU,
	PROP_WINDOW_ID,

	PROP_STATE,

	NB_PROPS
};

static uint prop_name_from_icon[SN_ICONS_NUM] = { PROP_MAIN_ICON_NAME,
	                                          PROP_ATTENTION_ICON_NAME,
	                                          PROP_OVERLAY_ICON_NAME,
	                                          PROP_TOOLTIP_ICON_NAME };
static uint prop_pixbuf_from_icon[SN_ICONS_NUM] = { PROP_MAIN_ICON_PIXBUF,
	                                            PROP_ATTENTION_ICON_PIXBUF,
	                                            PROP_OVERLAY_ICON_PIXBUF,
	                                            PROP_TOOLTIP_ICON_PIXBUF };

enum
{
	SIGNAL_REGISTRATION_FAILED,
	NB_SIGNALS
};

enum InjectMode
{
	INJECT_DIRECT,
	INJECT_XTEST
};

typedef struct
{
	char *id;
	StatusNotifierCategory category;
	char *title;
	StatusNotifierStatus status;
	GdkPixbuf *icon[SN_ICONS_NUM];
	char *tooltip_title;
	char *tooltip_body;
	enum InjectMode mode;
	xcb_window_t window_id;
	xcb_window_t container_id;
	xcb_connection_t *conn;
	bool item_is_menu;

	uint tooltip_freeze;

	StatusNotifierState state;
	uint dbus_watch_id;
	gulong dbus_sid;
	uint dbus_owner_id;
	uint dbus_reg_id;
	GDBusProxy *dbus_proxy;
	GDBusConnection *dbus_conn;
	GError *dbus_err;
} StatusNotifierItemPrivate;

static uint uniq_id = 0;

static GParamSpec *status_notifier_item_props[NB_PROPS] = {
	NULL,
};
static uint status_notifier_item_signals[NB_SIGNALS] = {
	0,
};

#define notify(sn, prop) g_object_notify_by_pspec((GObject *)sn, status_notifier_item_props[prop])

static void status_notifier_item_set_property(GObject *object, uint prop_id, const GValue *value,
                                              GParamSpec *pspec);
static void status_notifier_item_get_property(GObject *object, uint prop_id, GValue *value,
                                              GParamSpec *pspec);
static void status_notifier_item_finalize(GObject *object);

GdkPixbuf *status_notifier_item_get_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon);
void status_notifier_item_set_from_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon,
                                          GdkPixbuf *pixbuf);

void status_notifier_item_set_tooltip_title(StatusNotifierItem *sn, const char *title);

G_DEFINE_TYPE_WITH_PRIVATE(StatusNotifierItem, status_notifier_item, G_TYPE_OBJECT)

static void status_notifier_item_class_init(StatusNotifierItemClass *klass)
{
	GObjectClass *o_class;

	o_class               = G_OBJECT_CLASS(klass);
	o_class->set_property = status_notifier_item_set_property;
	o_class->get_property = status_notifier_item_get_property;
	o_class->finalize     = status_notifier_item_finalize;
	status_notifier_item_props[PROP_ID] =
	    g_param_spec_string("id",
	                        "id",
	                        "Unique application identifier",
	                        NULL,
	                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	status_notifier_item_props[PROP_TITLE] =
	    g_param_spec_string("title",
	                        "title",
	                        "Descriptive name for the item",
	                        NULL,
	                        G_PARAM_READWRITE);
	status_notifier_item_props[PROP_CATEGORY] =
	    g_param_spec_enum("category",
	                      "category",
	                      "Category of the item",
	                      status_notifier_category_get_type(),
	                      SN_CATEGORY_APPLICATION_STATUS,
	                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	status_notifier_item_props[PROP_STATUS] =
	    g_param_spec_enum("status",
	                      "status",
	                      "Status of the item",
	                      status_notifier_status_get_type(),
	                      SN_STATUS_PASSIVE,
	                      G_PARAM_READWRITE);
	status_notifier_item_props[PROP_MAIN_ICON_NAME] =
	    g_param_spec_string("main-icon-name",
	                        "main-icon-name",
	                        "Icon name for the main icon",
	                        NULL,
	                        G_PARAM_READWRITE);
	status_notifier_item_props[PROP_MAIN_ICON_PIXBUF] =
	    g_param_spec_object("main-icon-pixbuf",
	                        "main-icon-pixbuf",
	                        "Pixbuf for the main icon",
	                        GDK_TYPE_PIXBUF,
	                        G_PARAM_READWRITE);
	status_notifier_item_props[PROP_OVERLAY_ICON_NAME] =
	    g_param_spec_string("overlay-icon-name",
	                        "overlay-icon-name",
	                        "Icon name for the overlay icon",
	                        NULL,
	                        G_PARAM_READWRITE);
	status_notifier_item_props[PROP_OVERLAY_ICON_PIXBUF] =
	    g_param_spec_object("overlay-icon-pixbuf",
	                        "overlay-icon-pixbuf",
	                        "Pixbuf for the overlay icon",
	                        GDK_TYPE_PIXBUF,
	                        G_PARAM_READWRITE);
	status_notifier_item_props[PROP_ATTENTION_ICON_NAME] =
	    g_param_spec_string("attention-icon-name",
	                        "attention-icon-name",
	                        "Icon name for the attention icon",
	                        NULL,
	                        G_PARAM_READWRITE);
	status_notifier_item_props[PROP_ATTENTION_ICON_PIXBUF] =
	    g_param_spec_object("attention-icon-pixbuf",
	                        "attention-icon-pixbuf",
	                        "Pixbuf for the attention icon",
	                        GDK_TYPE_PIXBUF,
	                        G_PARAM_READWRITE);
	status_notifier_item_props[PROP_ATTENTION_MOVIE_NAME] = g_param_spec_string(
	    "attention-movie-name",
	    "attention-movie-name",
	    "Animation name/full path when the item is in needs-attention status",
	    NULL,
	    G_PARAM_READWRITE);
	status_notifier_item_props[PROP_TOOLTIP_ICON_NAME] =
	    g_param_spec_string("tooltip-icon-name",
	                        "tooltip-icon-name",
	                        "Icon name for the tooltip icon",
	                        NULL,
	                        G_PARAM_READWRITE);
	status_notifier_item_props[PROP_TOOLTIP_ICON_PIXBUF] =
	    g_param_spec_object("tooltip-icon-pixbuf",
	                        "tooltip-icon-pixbuf",
	                        "Pixbuf for the tooltip icon",
	                        GDK_TYPE_PIXBUF,
	                        G_PARAM_READWRITE);
	status_notifier_item_props[PROP_TOOLTIP_TITLE] = g_param_spec_string("tooltip-title",
	                                                                     "tooltip-title",
	                                                                     "Title of the tooltip",
	                                                                     NULL,
	                                                                     G_PARAM_READWRITE);
	status_notifier_item_props[PROP_TOOLTIP_BODY] = g_param_spec_string("tooltip-body",
	                                                                    "tooltip-body",
	                                                                    "Body of the tooltip",
	                                                                    NULL,
	                                                                    G_PARAM_READWRITE);
	status_notifier_item_props[PROP_ITEM_IS_MENU] =
	    g_param_spec_boolean("item-is-menu",
	                         "item-is-menu",
	                         "Whether or not the item only supports context menu",
	                         FALSE,
	                         G_PARAM_READWRITE);
	status_notifier_item_props[PROP_MENU] =
	    g_param_spec_object("menu",
	                        "menu",
	                        "Context menu to be exposed via dbus",
	                        G_TYPE_OBJECT,
	                        G_PARAM_READABLE);
	status_notifier_item_props[PROP_WINDOW_ID] = g_param_spec_uint("window-id",
	                                                               "window-id",
	                                                               "Window ID",
	                                                               0,
	                                                               G_MAXUINT32,
	                                                               0,
	                                                               G_PARAM_READWRITE);
	status_notifier_item_props[PROP_STATE] =
	    g_param_spec_enum("state",
	                      "state",
	                      "DBus registration state of the item",
	                      status_notifier_state_get_type(),
	                      SN_STATE_NOT_REGISTERED,
	                      G_PARAM_READABLE);

	g_object_class_install_properties(o_class, NB_PROPS, status_notifier_item_props);

	status_notifier_item_signals[SIGNAL_REGISTRATION_FAILED] =
	    g_signal_new("registration-failed",
	                 status_notifier_item_get_type(),
	                 G_SIGNAL_RUN_LAST,
	                 G_STRUCT_OFFSET(StatusNotifierItemClass, registration_failed),
	                 NULL,
	                 NULL,
	                 g_cclosure_marshal_VOID__BOXED,
	                 G_TYPE_NONE,
	                 1,
	                 G_TYPE_ERROR);
	g_type_class_add_private(klass, sizeof(StatusNotifierItemPrivate));
}

static void status_notifier_item_init(StatusNotifierItem *sn)
{
}

static void free_icon(StatusNotifierItem *sn, StatusNotifierIcon icon)
{
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	g_object_unref(priv->icon[icon]);
}

static void dbus_free(StatusNotifierItem *sn)
{
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	if (priv->dbus_watch_id > 0)
	{
		g_bus_unwatch_name(priv->dbus_watch_id);
		priv->dbus_watch_id = 0;
	}
	if (priv->dbus_sid > 0)
	{
		g_signal_handler_disconnect(priv->dbus_proxy, priv->dbus_sid);
		priv->dbus_sid = 0;
	}
	if (G_LIKELY(priv->dbus_owner_id > 0))
	{
		g_bus_unown_name(priv->dbus_owner_id);
		priv->dbus_owner_id = 0;
	}
	if (priv->dbus_proxy)
	{
		g_object_unref(priv->dbus_proxy);
		priv->dbus_proxy = NULL;
	}
	if (priv->dbus_reg_id > 0)
	{
		g_dbus_connection_unregister_object(priv->dbus_conn, priv->dbus_reg_id);
		priv->dbus_reg_id = 0;
	}
	if (priv->dbus_conn)
	{
		g_object_unref(priv->dbus_conn);
		priv->dbus_conn = NULL;
	}
}

static void status_notifier_item_finalize(GObject *object)
{
	StatusNotifierItem *sn = (StatusNotifierItem *)object;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	uint i;

	g_free(priv->id);
	g_free(priv->title);
	for (i = 0; i < SN_ICONS_NUM; ++i)
		free_icon(sn, i);
	g_free(priv->tooltip_title);
	g_free(priv->tooltip_body);

	dbus_free(sn);

	G_OBJECT_CLASS(status_notifier_item_parent_class)->finalize(object);
}

static void dbus_notify(StatusNotifierItem *sn, uint prop)
{
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	const char *signal;

	if (priv->state != SN_STATE_REGISTERED)
		return;

	switch (prop)
	{
	case PROP_STATUS:
	{
		const char *const *s_status[] = { "Passive", "Active", "NeedsAttention" };
		signal                        = "NewStatus";
		g_dbus_connection_emit_signal(priv->dbus_conn,
		                              NULL,
		                              ITEM_OBJECT,
		                              ITEM_INTERFACE,
		                              signal,
		                              g_variant_new("(s)", s_status[priv->status]),
		                              NULL);
		return;
	}
	case PROP_TITLE:
		signal = "NewTitle";
		break;
	case PROP_MAIN_ICON_NAME:
	case PROP_MAIN_ICON_PIXBUF:
		signal = "NewIcon";
		break;
	case PROP_ATTENTION_ICON_NAME:
	case PROP_ATTENTION_ICON_PIXBUF:
		signal = "NewAttentionIcon";
		break;
	case PROP_OVERLAY_ICON_NAME:
	case PROP_OVERLAY_ICON_PIXBUF:
		signal = "NewOverlayIcon";
		break;
	case PROP_TOOLTIP_TITLE:
	case PROP_TOOLTIP_BODY:
	case PROP_TOOLTIP_ICON_NAME:
	case PROP_TOOLTIP_ICON_PIXBUF:
		signal = "NewToolTip";
		break;
	default:
		g_return_if_reached();
	}

	g_dbus_connection_emit_signal(priv->dbus_conn,
	                              NULL,
	                              ITEM_OBJECT,
	                              ITEM_INTERFACE,
	                              signal,
	                              NULL,
	                              NULL);
}

StatusNotifierItem *status_notifier_item_new_from_pixbuf(const char *id,
                                                         StatusNotifierCategory category,
                                                         GdkPixbuf *pixbuf)
{
	return (StatusNotifierItem *)g_object_new(status_notifier_item_get_type(),
	                                          "id",
	                                          id,
	                                          "category",
	                                          category,
	                                          "main-icon-pixbuf",
	                                          pixbuf,
	                                          NULL);
}

StatusNotifierItem *status_notifier_item_new_from_icon_name(const char *id,
                                                            StatusNotifierCategory category,
                                                            const char *icon_name)
{
	return (StatusNotifierItem *)g_object_new(status_notifier_item_get_type(),
	                                          "id",
	                                          id,
	                                          "category",
	                                          category,
	                                          "main-icon-name",
	                                          icon_name,
	                                          NULL);
}

void status_notifier_item_set_from_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon,
                                          GdkPixbuf *pixbuf)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	free_icon(sn, icon);
	priv->icon[icon] = g_object_ref(pixbuf);

	notify(sn, prop_name_from_icon[icon]);
	if (icon != SN_TOOLTIP_ICON || priv->tooltip_freeze == 0)
		dbus_notify(sn, prop_name_from_icon[icon]);
}

GdkPixbuf *status_notifier_item_get_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon)
{
	StatusNotifierItemPrivate *priv;

	g_return_val_if_fail(SN_IS_ITEM(sn), NULL);
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	return GDK_PIXBUF(g_object_ref(priv->icon[icon]));
}

void status_notifier_item_set_title(StatusNotifierItem *sn, const char *title)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	g_free(priv->title);
	priv->title = g_strdup(title);

	notify(sn, PROP_TITLE);
	dbus_notify(sn, PROP_TITLE);
}

void status_notifier_item_set_status(StatusNotifierItem *sn, StatusNotifierStatus status)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	priv->status = status;

	notify(sn, PROP_STATUS);
	dbus_notify(sn, PROP_STATUS);
}

StatusNotifierStatus status_notifier_item_get_status(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), -1);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return priv->status;
}

void status_notifier_item_set_window_id(StatusNotifierItem *sn, u_int32_t window_id)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	priv->window_id = window_id;

	notify(sn, PROP_WINDOW_ID);
}

void status_notifier_item_set_tooltip_title(StatusNotifierItem *sn, const char *title)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	g_free(priv->tooltip_title);
	priv->tooltip_title = g_strdup(title);

	notify(sn, PROP_TOOLTIP_TITLE);
	if (priv->tooltip_freeze == 0)
		dbus_notify(sn, PROP_TOOLTIP_TITLE);
}

char *status_notifier_item_get_tooltip_title(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), NULL);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return g_strdup(priv->tooltip_title);
}

void status_notifier_item_set_tooltip_body(StatusNotifierItem *sn, const char *body)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	g_free(priv->tooltip_body);
	priv->tooltip_body = g_strdup(body);

	notify(sn, PROP_TOOLTIP_BODY);
	if (priv->tooltip_freeze == 0)
		dbus_notify(sn, PROP_TOOLTIP_BODY);
}

void status_notifier_item_set_tooltip(StatusNotifierItem *sn, GdkPixbuf *icon, const char *title,
                                      const char *body)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	status_notifier_item_set_from_pixbuf(sn, SN_TOOLTIP_ICON, icon);
	status_notifier_item_set_tooltip_title(sn, title);
	status_notifier_item_set_tooltip_body(sn, body);
}

char *status_notifier_item_get_tooltip_body(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), NULL);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return g_strdup(priv->tooltip_body);
}

void status_notifier_item_send_click(StatusNotifierItem *sn, uint8_t mouseButton, int x, int y)
{
	// it's best not to look at this code
	// GTK doesn't like send_events and double checks the mouse position matches where the
	// window is and is top level
	// in order to solve this we move the embed container over to where the mouse is then replay
	// the event using send_event
	// if patching, test with xchat + xchat context menus

	// note x,y are not actually where the mouse is, but the plasmoid
	// ideally we should make this match the plasmoid hit area

	g_debug("sn: received click %d with x=%d,y=%d\n", mouseButton, x, y);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	xcb_get_geometry_cookie_t cookieSize = xcb_get_geometry(priv->conn, priv->window_id);
	g_autofree xcb_get_geometry_reply_t *clientGeom =
	    xcb_get_geometry_reply(priv->conn, cookieSize, NULL);
	if (!clientGeom)
	{
		return;
	}

	xcb_query_pointer_cookie_t cookie = xcb_query_pointer(priv->conn, priv->window_id);
	g_autofree xcb_query_pointer_reply_t *pointer =
	    xcb_query_pointer_reply(priv->conn, cookie, NULL);

	/*qCDebug(SNIPROXY) << "samescreen" << pointer->same_screen << endl
	<< "root x*y" << pointer->root_x << pointer->root_y << endl
	<< "win x*y" << pointer->win_x << pointer->win_y;*/

	// move our window so the mouse is within its geometry
	uint32_t configVals[2] = { 0, 0 };
	if (mouseButton >= XCB_BUTTON_INDEX_4)
	{
		// scroll event, take pointer position
		configVals[0] = pointer->root_x;
		configVals[1] = pointer->root_y;
	}
	else
	{
		if (pointer->root_x > x + clientGeom->width)
			configVals[0] = pointer->root_x - clientGeom->width + 1;
		else
			configVals[0] = (uint32_t)(x);
		if (pointer->root_y > y + clientGeom->height)
			configVals[1] = pointer->root_y - clientGeom->height + 1;
		else
			configVals[1] = (uint32_t)(y);
	}
	xcb_configure_window(priv->conn,
	                     priv->container_id,
	                     XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
	                     configVals);

	// pull window up
	const uint32_t stackAboveData[] = { XCB_STACK_MODE_ABOVE };
	xcb_configure_window(priv->conn,
	                     priv->container_id,
	                     XCB_CONFIG_WINDOW_STACK_MODE,
	                     stackAboveData);

	// mouse down
	if (priv->mode == INJECT_DIRECT)
	{
		xcb_button_press_event_t *event = g_malloc0(sizeof(xcb_button_press_event_t));
		memset(event, 0x00, sizeof(xcb_button_press_event_t));
		event->response_type = XCB_BUTTON_PRESS;
		event->event         = priv->window_id;
		event->time          = xcb_get_timestamp_for_connection(priv->conn);
		event->same_screen   = 1;
		event->root          = xcb_get_screen_for_connection(priv->conn, 0)->root;
		event->root_x        = x;
		event->root_y        = y;
		event->event_x       = 0;
		event->event_y       = 0;
		event->child         = 0;
		event->state         = 0;
		event->detail        = mouseButton;

		xcb_send_event(priv->conn,
		               false,
		               priv->window_id,
		               XCB_EVENT_MASK_BUTTON_PRESS,
		               (char *)event);
		g_free(event);
	}
	else
	{
		sendXTestPressed(priv->conn, mouseButton);
	}

	// mouse up
	if (priv->mode == INJECT_DIRECT)
	{
		xcb_button_release_event_t *event = g_malloc0(sizeof(xcb_button_release_event_t));
		memset(event, 0x00, sizeof(xcb_button_release_event_t));
		event->response_type = XCB_BUTTON_RELEASE;
		event->event         = priv->window_id;
		event->time          = xcb_get_timestamp_for_connection(priv->conn);
		event->same_screen   = 1;
		event->root          = xcb_get_screen_for_connection(priv->conn, 0)->root;
		event->root_x        = x;
		event->root_y        = y;
		event->event_x       = 0;
		event->event_y       = 0;
		event->child         = 0;
		event->state         = 0;
		event->detail        = mouseButton;

		xcb_send_event(priv->conn,
		               false,
		               priv->window_id,
		               XCB_EVENT_MASK_BUTTON_RELEASE,
		               (char *)event);
		g_free(event);
	}
	else
	{
		sendXTestReleased(priv->conn, mouseButton);
	}

#ifndef VISUAL_DEBUG
	const uint32_t stackBelowData[] = { XCB_STACK_MODE_BELOW };
	xcb_configure_window(priv->conn,
	                     priv->container_id,
	                     XCB_CONFIG_WINDOW_STACK_MODE,
	                     stackBelowData);
#endif
}

static void method_call(GDBusConnection *conn _UNUSED_, const char *sender _UNUSED_,
                        const char *object _UNUSED_, const char *interface _UNUSED_,
                        const char *method, GVariant *params, GDBusMethodInvocation *invocation,
                        gpointer data)
{
	StatusNotifierItem *sn = (StatusNotifierItem *)data;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	uint signal;
	gint x, y;
	bool ret;

	if (!g_strcmp0(method, "Scroll"))
	{
		gint delta, orientation;
		char *s_orientation;

		g_variant_get(params, "(is)", &delta, &s_orientation);
		if (!g_ascii_strcasecmp(s_orientation, "vertical"))
		{
			status_notifier_item_send_click(priv->conn,
			                                delta > 0 ? XCB_BUTTON_INDEX_4
			                                          : XCB_BUTTON_INDEX_5,
			                                0,
			                                0);
		}
		else
		{
			status_notifier_item_send_click(priv->conn, delta > 0 ? 6 : 7, 0, 0);
		}
		g_free(s_orientation);
		return;
	}
	else if (!g_strcmp0(method, "ContextMenu"))
	{
		g_variant_get(params, "(ii)", &x, &y);
		status_notifier_item_send_click(priv->conn, XCB_BUTTON_INDEX_3, x, y);
	}
	else if (!g_strcmp0(method, "Activate"))
	{
		g_variant_get(params, "(ii)", &x, &y);
		status_notifier_item_send_click(priv->conn, XCB_BUTTON_INDEX_1, x, y);
	}
	else if (!g_strcmp0(method, "SecondaryActivate"))
	{
		g_variant_get(params, "(ii)", &x, &y);
		status_notifier_item_send_click(priv->conn, XCB_BUTTON_INDEX_2, x, y);
	}
	else
		/* should never happen */
		g_return_if_reached();
	g_dbus_method_invocation_return_value(invocation, NULL);
}

static GVariantBuilder *get_builder_for_icon_pixmap(StatusNotifierItem *sn, StatusNotifierIcon icon)
{
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	GVariantBuilder *builder;
	cairo_surface_t *surface;
	cairo_t *cr;
	gint width, height, stride;
	uint *data;

	width  = gdk_pixbuf_get_width(priv->icon[icon]);
	height = gdk_pixbuf_get_height(priv->icon[icon]);

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cr      = cairo_create(surface);
	gdk_cairo_set_source_pixbuf(cr, priv->icon[icon], 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);

	stride = cairo_image_surface_get_stride(surface);
	cairo_surface_flush(surface);
	data = (uint *)cairo_image_surface_get_data(surface);
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	uint i, max;

	max = (uint)(stride * height) / sizeof(uint);
	for (i          = 0; i < max; ++i)
		data[i] = GUINT_TO_BE(data[i]);
#endif

	builder = g_variant_builder_new(G_VARIANT_TYPE("a(iiay)"));
	g_variant_builder_open(builder, G_VARIANT_TYPE("(iiay)"));
	g_variant_builder_add(builder, "i", width);
	g_variant_builder_add(builder, "i", height);
	g_variant_builder_add_value(builder,
	                            g_variant_new_from_data(G_VARIANT_TYPE("ay"),
	                                                    data,
	                                                    (gsize)(stride * height),
	                                                    TRUE,
	                                                    (GDestroyNotify)cairo_surface_destroy,
	                                                    surface));
	g_variant_builder_close(builder);
	return builder;
}

static void status_notifier_item_set_property(GObject *object, uint prop_id, const GValue *value,
                                              GParamSpec *pspec)
{
	StatusNotifierItem *sn = (StatusNotifierItem *)object;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	switch (prop_id)
	{
	case PROP_ID: /* G_PARAM_CONSTRUCT_ONLY */
		priv->id = g_value_dup_string(value);
		break;
	case PROP_TITLE:
		status_notifier_item_set_title(sn, g_value_get_string(value));
		break;
	case PROP_CATEGORY: /* G_PARAM_CONSTRUCT_ONLY */
		priv->category = g_value_get_enum(value);
		break;
	case PROP_STATUS:
		status_notifier_item_set_status(sn, g_value_get_enum(value));
		break;
	case PROP_MAIN_ICON_PIXBUF:
		status_notifier_item_set_from_pixbuf(sn, SN_ICON, g_value_get_object(value));
		break;
	case PROP_OVERLAY_ICON_PIXBUF:
		status_notifier_item_set_from_pixbuf(sn,
		                                     SN_OVERLAY_ICON,
		                                     g_value_get_object(value));
		break;
	case PROP_ATTENTION_ICON_PIXBUF:
		status_notifier_item_set_from_pixbuf(sn,
		                                     SN_ATTENTION_ICON,
		                                     g_value_get_object(value));
		break;
	case PROP_TOOLTIP_ICON_PIXBUF:
		status_notifier_item_set_from_pixbuf(sn,
		                                     SN_TOOLTIP_ICON,
		                                     g_value_get_object(value));
		break;
	case PROP_TOOLTIP_TITLE:
		status_notifier_item_set_tooltip_title(sn, g_value_get_string(value));
		break;
	case PROP_TOOLTIP_BODY:
		status_notifier_item_set_tooltip_body(sn, g_value_get_string(value));
		break;
	case PROP_WINDOW_ID:
		status_notifier_item_set_window_id(sn, g_value_get_uint(value));
		break;
	case PROP_MAIN_ICON_NAME:
	case PROP_OVERLAY_ICON_NAME:
	case PROP_ATTENTION_ICON_NAME:
	case PROP_ATTENTION_MOVIE_NAME:
	case PROP_TOOLTIP_ICON_NAME:
	case PROP_ITEM_IS_MENU:

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void status_notifier_item_get_property(GObject *object, uint prop_id, GValue *value,
                                              GParamSpec *pspec)
{
	StatusNotifierItem *sn = (StatusNotifierItem *)object;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	switch (prop_id)
	{
	case PROP_ID:
		g_value_set_string(value, priv->id);
		break;
	case PROP_TITLE:
		g_value_set_string(value, priv->title);
		break;
	case PROP_CATEGORY:
		g_value_set_enum(value, priv->category);
		break;
	case PROP_STATUS:
		g_value_set_enum(value, priv->status);
		break;
	case PROP_MAIN_ICON_NAME:
		g_value_set_static_string(value, "");
		break;
	case PROP_MAIN_ICON_PIXBUF:
		g_value_take_object(value, status_notifier_item_get_pixbuf(sn, SN_ICON));
		break;
	case PROP_OVERLAY_ICON_NAME:
		g_value_set_static_string(value, "");
		break;
	case PROP_OVERLAY_ICON_PIXBUF:
		g_value_take_object(value, status_notifier_item_get_pixbuf(sn, SN_OVERLAY_ICON));
		break;
	case PROP_ATTENTION_ICON_NAME:
		g_value_set_static_string(value, "");
		break;
	case PROP_ATTENTION_ICON_PIXBUF:
		g_value_take_object(value, status_notifier_item_get_pixbuf(sn, SN_ATTENTION_ICON));
		break;
	case PROP_ATTENTION_MOVIE_NAME:
		g_value_set_static_string(value, "");
		break;
	case PROP_TOOLTIP_ICON_NAME:
		g_value_set_static_string(value, "");
		break;
	case PROP_TOOLTIP_ICON_PIXBUF:
		g_value_take_object(value, status_notifier_item_get_pixbuf(sn, SN_TOOLTIP_ICON));
		break;
	case PROP_TOOLTIP_TITLE:
		g_value_set_string(value, priv->tooltip_title);
		break;
	case PROP_TOOLTIP_BODY:
		g_value_set_string(value, priv->tooltip_body);
		break;
	case PROP_ITEM_IS_MENU:
		g_value_set_boolean(value, priv->item_is_menu);
		break;
	case PROP_MENU:
		g_value_set_object(value, NULL);
		break;
	case PROP_WINDOW_ID:
		g_value_set_uint(value, priv->window_id);
		break;
	case PROP_STATE:
		g_value_set_enum(value, priv->state);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static GVariant *get_prop(GDBusConnection *conn _UNUSED_, const char *sender _UNUSED_,
                          const char *object _UNUSED_, const char *interface _UNUSED_,
                          const char *property, GError **error _UNUSED_, gpointer data)
{
	StatusNotifierItem *sn = (StatusNotifierItem *)data;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	if (!g_strcmp0(property, "Id"))
		return g_variant_new("s", priv->id);
	else if (!g_strcmp0(property, "Category"))
	{
		return g_variant_new("s", "ApplicationStatus");
	}
	else if (!g_strcmp0(property, "Title"))
		return g_variant_new("s", (priv->title) ? priv->title : "");
	else if (!g_strcmp0(property, "Status"))
	{
		const char *const *s_status[] = { "Passive", "Active", "NeedsAttention" };
		return g_variant_new("s", s_status[priv->status]);
	}
	else if (!g_strcmp0(property, "WindowId"))
		return g_variant_new("i", priv->window_id);
	else if (!g_strcmp0(property, "IconName"))
		return g_variant_new("s", "");
	else if (!g_strcmp0(property, "IconPixmap"))
		return g_variant_new("a(iiay)", get_builder_for_icon_pixmap(sn, SN_ICON));
	else if (!g_strcmp0(property, "OverlayIconName"))
		return g_variant_new("s", "");
	else if (!g_strcmp0(property, "OverlayIconPixmap"))
		return g_variant_new("a(iiay)", get_builder_for_icon_pixmap(sn, SN_OVERLAY_ICON));
	else if (!g_strcmp0(property, "AttentionIconName"))
		return g_variant_new("s", "");
	else if (!g_strcmp0(property, "AttentionIconPixmap"))
		return g_variant_new("a(iiay)", get_builder_for_icon_pixmap(sn, SN_ATTENTION_ICON));
	else if (!g_strcmp0(property, "AttentionMovieName"))
		return g_variant_new("s", "");
	else if (!g_strcmp0(property, "ToolTip"))
	{
		GVariant *variant;
		GVariantBuilder *builder;

		builder = get_builder_for_icon_pixmap(sn, SN_TOOLTIP_ICON);
		variant = g_variant_new("(sa(iiay)ss)",
		                        "",
		                        builder,
		                        (priv->tooltip_title) ? priv->tooltip_title : "",
		                        (priv->tooltip_body) ? priv->tooltip_body : "");
		g_variant_builder_unref(builder);

		return variant;
	}
	else if (!g_strcmp0(property, "ItemIsMenu"))
		return g_variant_new("b", false);
	else if (!g_strcmp0(property, "Menu"))
	{
		return g_variant_new("o", "/NO_DBUSMENU");
	}

	g_return_val_if_reached(NULL);
}

static void dbus_failed(StatusNotifierItem *sn, GError *error, bool fatal)
{
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	dbus_free(sn);
	if (fatal)
	{
		priv->state = SN_STATE_FAILED;
		notify(sn, PROP_STATE);
	}
	g_signal_emit(sn, status_notifier_item_signals[SIGNAL_REGISTRATION_FAILED], 0, error);
	g_error_free(error);
}

static void bus_acquired(GDBusConnection *conn, const char *name _UNUSED_, gpointer data)
{
	GError *err            = NULL;
	StatusNotifierItem *sn = (StatusNotifierItem *)data;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	GDBusInterfaceVTable interface_vtable = {.method_call  = method_call,
		                                 .get_property = get_prop,
		                                 .set_property = NULL };
	GDBusNodeInfo *info;

	info              = g_dbus_node_info_new_for_xml(item_xml, NULL);
	priv->dbus_reg_id = g_dbus_connection_register_object(conn,
	                                                      ITEM_OBJECT,
	                                                      info->interfaces[0],
	                                                      &interface_vtable,
	                                                      sn,
	                                                      NULL,
	                                                      &err);
	g_dbus_node_info_unref(info);
	if (priv->dbus_reg_id == 0)
	{
		dbus_failed(sn, err, TRUE);
		return;
	}

	priv->dbus_conn = g_object_ref(conn);
}

static void register_item_cb(GObject *sce, GAsyncResult *result, gpointer data)
{
	GError *err            = NULL;
	StatusNotifierItem *sn = (StatusNotifierItem *)data;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	GVariant *variant;

	variant = g_dbus_proxy_call_finish((GDBusProxy *)sce, result, &err);
	if (!variant)
	{
		dbus_failed(sn, err, TRUE);
		return;
	}
	g_variant_unref(variant);

	priv->state = SN_STATE_REGISTERED;
	notify(sn, PROP_STATE);
}

static void name_acquired(GDBusConnection *conn _UNUSED_, const char *name, gpointer data)
{
	StatusNotifierItem *sn = (StatusNotifierItem *)data;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	g_dbus_proxy_call(priv->dbus_proxy,
	                  "RegisterStatusNotifierItem",
	                  g_variant_new("(s)", name),
	                  G_DBUS_CALL_FLAGS_NONE,
	                  -1,
	                  NULL,
	                  register_item_cb,
	                  sn);
	g_object_unref(priv->dbus_proxy);
	priv->dbus_proxy = NULL;
}

static void name_lost(GDBusConnection *conn, const char *name _UNUSED_, gpointer data)
{
	GError *err            = NULL;
	StatusNotifierItem *sn = (StatusNotifierItem *)data;

	if (!conn)
		g_set_error(&err,
		            SN_ERROR,
		            SN_ERROR_NO_CONNECTION,
		            "Failed to establish DBus connection");
	else
		g_set_error(&err, SN_ERROR, SN_ERROR_NO_NAME, "Failed to acquire name for item");
	dbus_failed(sn, err, TRUE);
}

static void dbus_reg_item(StatusNotifierItem *sn)
{
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	char buf[64], *b = buf;

	if (G_UNLIKELY(
	        g_snprintf(buf, 64, "org.kde.StatusNotifierItem-%u-%u", getpid(), ++uniq_id) >= 64))
		b = g_strdup_printf("org.kde.StatusNotifierItem-%u-%u", getpid(), uniq_id);
	priv->dbus_owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
	                                     b,
	                                     G_BUS_NAME_OWNER_FLAGS_NONE,
	                                     bus_acquired,
	                                     name_acquired,
	                                     name_lost,
	                                     sn,
	                                     NULL);
	if (G_UNLIKELY(b != buf))
		g_free(b);
}

static void watcher_signal(GDBusProxy *proxy _UNUSED_, const char *sender _UNUSED_,
                           const char *signal, GVariant *params _UNUSED_, StatusNotifierItem *sn)
{
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	if (!g_strcmp0(signal, "StatusNotifierHostRegistered"))
	{
		g_signal_handler_disconnect(priv->dbus_proxy, priv->dbus_sid);
		priv->dbus_sid = 0;

		dbus_reg_item(sn);
	}
}

static void proxy_cb(GObject *sce _UNUSED_, GAsyncResult *result, gpointer data)
{
	GError *err            = NULL;
	StatusNotifierItem *sn = (StatusNotifierItem *)data;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	GVariant *variant;

	priv->dbus_proxy = g_dbus_proxy_new_for_bus_finish(result, &err);
	if (!priv->dbus_proxy)
	{
		dbus_failed(sn, err, TRUE);
		return;
	}

	variant =
	    g_dbus_proxy_get_cached_property(priv->dbus_proxy, "IsStatusNotifierHostRegistered");
	if (!variant || !g_variant_get_boolean(variant))
	{
		GDBusProxy *proxy;

		g_set_error(&err, SN_ERROR, SN_ERROR_NO_HOST, "No Host registered on the Watcher");
		if (variant)
			g_variant_unref(variant);

		/* keep the proxy, we'll wait for the signal when a host registers */
		proxy = priv->dbus_proxy;
		/* (so dbus_free() from dbus_failed() doesn't unref) */
		priv->dbus_proxy = NULL;
		dbus_failed(sn, err, FALSE);
		priv->dbus_proxy = proxy;

		priv->dbus_sid =
		    g_signal_connect(priv->dbus_proxy, "g-signal", (GCallback)watcher_signal, sn);
		return;
	}
	g_variant_unref(variant);

	dbus_reg_item(sn);
}

static void watcher_appeared(GDBusConnection *conn _UNUSED_, const char *name _UNUSED_,
                             const char *owner _UNUSED_, gpointer data)
{
	StatusNotifierItem *sn = data;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	GDBusNodeInfo *info;

	g_bus_unwatch_name(priv->dbus_watch_id);
	priv->dbus_watch_id = 0;

	info = g_dbus_node_info_new_for_xml(watcher_xml, NULL);
	g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
	                         G_DBUS_PROXY_FLAGS_NONE,
	                         info->interfaces[0],
	                         WATCHER_NAME,
	                         WATCHER_OBJECT,
	                         WATCHER_INTERFACE,
	                         NULL,
	                         proxy_cb,
	                         sn);
	g_dbus_node_info_unref(info);
}

static void watcher_vanished(GDBusConnection *conn _UNUSED_, const char *name _UNUSED_,
                             gpointer data)
{
	GError *err            = NULL;
	StatusNotifierItem *sn = data;
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	uint id;

	/* keep the watch active, so if a watcher shows up we'll resume the
	 * registering automatically */
	id = priv->dbus_watch_id;
	/* (so dbus_free() from dbus_failed() doesn't unwatch) */
	priv->dbus_watch_id = 0;

	g_set_error(&err, SN_ERROR, SN_ERROR_NO_WATCHER, "No Watcher found");
	dbus_failed(sn, err, FALSE);

	priv->dbus_watch_id = id;
}

void status_notifier_item_register(StatusNotifierItem *sn)

{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	if (priv->state == SN_STATE_REGISTERING || priv->state == SN_STATE_REGISTERED)
		return;
	priv->state = SN_STATE_REGISTERING;

	priv->dbus_watch_id = g_bus_watch_name(G_BUS_TYPE_SESSION,
	                                       WATCHER_NAME,
	                                       G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
	                                       watcher_appeared,
	                                       watcher_vanished,
	                                       sn,
	                                       NULL);
}
