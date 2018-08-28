#include "sn.h"
#include "closures.h"
#include "interfaces.h"
#include "sni-enums.h"
#include <gdk/gdk.h>
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

static uint prop_name_from_icon[SN_ICONS_NUM]   = { PROP_MAIN_ICON_NAME,
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
	SIGNAL_CONTEXT_MENU,
	SIGNAL_ACTIVATE,
	SIGNAL_SECONDARY_ACTIVATE,
	SIGNAL_SCROLL,
	NB_SIGNALS
};

typedef struct
{
    char *id;
	StatusNotifierCategory category;
    char *title;
	StatusNotifierStatus status;
	struct
	{
		bool has_pixbuf;
		union {
            char *icon_name;
			GdkPixbuf *pixbuf;
		};
	} icon[SN_ICONS_NUM];
    char *attention_movie_name;
    char *tooltip_title;
    char *tooltip_body;
	u_int32_t window_id;
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
	status_notifier_item_props[PROP_TOOLTIP_BODY]  = g_param_spec_string("tooltip-body",
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
	status_notifier_item_signals[SIGNAL_CONTEXT_MENU] =
	    g_signal_new("context-menu",
	                 status_notifier_item_get_type(),
	                 G_SIGNAL_RUN_LAST,
	                 G_STRUCT_OFFSET(StatusNotifierItemClass, context_menu),
	                 g_signal_accumulator_true_handled,
	                 NULL,
	                 g_cclosure_user_marshal_BOOLEAN__INT_INT,
	                 G_TYPE_BOOLEAN,
	                 2,
	                 G_TYPE_INT,
	                 G_TYPE_INT);

	status_notifier_item_signals[SIGNAL_ACTIVATE] =
	    g_signal_new("activate",
	                 status_notifier_item_get_type(),
	                 G_SIGNAL_RUN_LAST,
	                 G_STRUCT_OFFSET(StatusNotifierItemClass, activate),
	                 g_signal_accumulator_true_handled,
	                 NULL,
	                 g_cclosure_user_marshal_BOOLEAN__INT_INT,
	                 G_TYPE_BOOLEAN,
	                 2,
	                 G_TYPE_INT,
	                 G_TYPE_INT);

	status_notifier_item_signals[SIGNAL_SECONDARY_ACTIVATE] =
	    g_signal_new("secondary-activate",
	                 status_notifier_item_get_type(),
	                 G_SIGNAL_RUN_LAST,
	                 G_STRUCT_OFFSET(StatusNotifierItemClass, secondary_activate),
	                 g_signal_accumulator_true_handled,
	                 NULL,
	                 g_cclosure_user_marshal_BOOLEAN__INT_INT,
	                 G_TYPE_BOOLEAN,
	                 2,
	                 G_TYPE_INT,
	                 G_TYPE_INT);
	status_notifier_item_signals[SIGNAL_SCROLL] =
	    g_signal_new("scroll",
	                 status_notifier_item_get_type(),
	                 G_SIGNAL_RUN_LAST,
	                 G_STRUCT_OFFSET(StatusNotifierItemClass, scroll),
	                 g_signal_accumulator_true_handled,
	                 NULL,
	                 g_cclosure_user_marshal_BOOLEAN__INT_INT,
	                 G_TYPE_BOOLEAN,
	                 2,
	                 G_TYPE_INT,
	                 status_notifier_scroll_orientation_get_type());

	g_type_class_add_private(klass, sizeof(StatusNotifierItemPrivate));
}

static void status_notifier_item_init(StatusNotifierItem *sn)
{
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
	case PROP_MAIN_ICON_NAME:
		status_notifier_item_set_from_icon_name(sn, SN_ICON, g_value_get_string(value));
		break;
	case PROP_MAIN_ICON_PIXBUF:
		status_notifier_item_set_from_pixbuf(sn, SN_ICON, g_value_get_object(value));
		break;
	case PROP_OVERLAY_ICON_NAME:
		status_notifier_item_set_from_icon_name(sn,
		                                        SN_OVERLAY_ICON,
		                                        g_value_get_string(value));
		break;
	case PROP_OVERLAY_ICON_PIXBUF:
		status_notifier_item_set_from_pixbuf(sn,
		                                     SN_OVERLAY_ICON,
		                                     g_value_get_object(value));
		break;
	case PROP_ATTENTION_ICON_NAME:
		status_notifier_item_set_from_icon_name(sn,
		                                        SN_ATTENTION_ICON,
		                                        g_value_get_string(value));
		break;
	case PROP_ATTENTION_ICON_PIXBUF:
		status_notifier_item_set_from_pixbuf(sn,
		                                     SN_ATTENTION_ICON,
		                                     g_value_get_object(value));
		break;
	case PROP_ATTENTION_MOVIE_NAME:
		status_notifier_item_set_attention_movie_name(sn, g_value_get_string(value));
		break;
	case PROP_TOOLTIP_ICON_NAME:
		status_notifier_item_set_from_icon_name(sn,
		                                        SN_TOOLTIP_ICON,
		                                        g_value_get_string(value));
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
	case PROP_ITEM_IS_MENU:
		status_notifier_item_set_item_is_menu(sn, g_value_get_boolean(value));
		break;
	case PROP_WINDOW_ID:
		status_notifier_item_set_window_id(sn, g_value_get_uint(value));
		break;
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
		g_value_take_string(value, status_notifier_item_get_icon_name(sn, SN_ICON));
		break;
	case PROP_MAIN_ICON_PIXBUF:
		g_value_take_object(value, status_notifier_item_get_pixbuf(sn, SN_ICON));
		break;
	case PROP_OVERLAY_ICON_NAME:
		g_value_take_string(value, status_notifier_item_get_icon_name(sn, SN_OVERLAY_ICON));
		break;
	case PROP_OVERLAY_ICON_PIXBUF:
		g_value_take_object(value, status_notifier_item_get_pixbuf(sn, SN_OVERLAY_ICON));
		break;
	case PROP_ATTENTION_ICON_NAME:
		g_value_take_string(value,
		                    status_notifier_item_get_icon_name(sn, SN_ATTENTION_ICON));
		break;
	case PROP_ATTENTION_ICON_PIXBUF:
		g_value_take_object(value, status_notifier_item_get_pixbuf(sn, SN_ATTENTION_ICON));
		break;
	case PROP_ATTENTION_MOVIE_NAME:
		g_value_set_string(value, priv->attention_movie_name);
		break;
	case PROP_TOOLTIP_ICON_NAME:
		g_value_take_string(value, status_notifier_item_get_icon_name(sn, SN_TOOLTIP_ICON));
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
		g_value_set_object(value, status_notifier_item_get_context_menu(sn));
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

static void free_icon(StatusNotifierItem *sn, StatusNotifierIcon icon)
{
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	if (priv->icon[icon].has_pixbuf)
		g_object_unref(priv->icon[icon].pixbuf);
	else
		g_free(priv->icon[icon].icon_name);
	priv->icon[icon].has_pixbuf = FALSE;
	priv->icon[icon].icon_name  = NULL;
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
	g_free(priv->attention_movie_name);
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

const char *status_notifier_item_get_id(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), NULL);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return priv->id;
}

StatusNotifierCategory status_notifier_item_get_category(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), -1);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return priv->category;
}

void status_notifier_item_set_from_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon,
                                          GdkPixbuf *pixbuf)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	free_icon(sn, icon);
	priv->icon[icon].has_pixbuf = TRUE;
	priv->icon[icon].pixbuf     = g_object_ref(pixbuf);

	notify(sn, prop_name_from_icon[icon]);
	if (icon != SN_TOOLTIP_ICON || priv->tooltip_freeze == 0)
		dbus_notify(sn, prop_name_from_icon[icon]);
}

void status_notifier_item_set_from_icon_name(StatusNotifierItem *sn, StatusNotifierIcon icon,
                                             const char *icon_name)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	free_icon(sn, icon);
	priv->icon[icon].icon_name = g_strdup(icon_name);

	notify(sn, prop_pixbuf_from_icon[icon]);
	if (icon != SN_TOOLTIP_ICON || priv->tooltip_freeze == 0)
		dbus_notify(sn, prop_name_from_icon[icon]);
}

bool status_notifier_item_has_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), FALSE);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	return priv->icon[icon].has_pixbuf;
}

GdkPixbuf *status_notifier_item_get_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon)
{
	StatusNotifierItemPrivate *priv;

	g_return_val_if_fail(SN_IS_ITEM(sn), NULL);
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	if (!priv->icon[icon].has_pixbuf)
		return NULL;

	return g_object_ref(priv->icon[icon].pixbuf);
}

char *status_notifier_item_get_icon_name(StatusNotifierItem *sn, StatusNotifierIcon icon)
{
	StatusNotifierItemPrivate *priv;

	g_return_val_if_fail(SN_IS_ITEM(sn), NULL);
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	if (priv->icon[icon].has_pixbuf)
		return NULL;

	return g_strdup(priv->icon[icon].icon_name);
}

void status_notifier_item_set_attention_movie_name(StatusNotifierItem *sn, const char *movie_name)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	g_free(priv->attention_movie_name);
	priv->attention_movie_name = g_strdup(movie_name);

	notify(sn, PROP_ATTENTION_MOVIE_NAME);
}

char *status_notifier_item_get_attention_movie_name(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), NULL);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return g_strdup(priv->attention_movie_name);
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

char *status_notifier_item_get_title(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), NULL);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return g_strdup(priv->title);
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

u_int32_t status_notifier_item_get_window_id(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), 0);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return priv->window_id;
}

void status_notifier_item_freeze_tooltip(StatusNotifierItem *sn)
{
	g_return_if_fail(SN_IS_ITEM(sn));
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	++priv->tooltip_freeze;
}

void status_notifier_item_thaw_tooltip(StatusNotifierItem *sn)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	g_return_if_fail(priv->tooltip_freeze > 0);

	if (--priv->tooltip_freeze == 0)
		dbus_notify(sn, PROP_TOOLTIP_TITLE);
}

void status_notifier_item_set_tooltip(StatusNotifierItem *sn, const char *icon_name,
                                      const char *title, const char *body)
{
	StatusNotifierItemPrivate *priv;

	g_return_if_fail(SN_IS_ITEM(sn));
	priv = (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);

	++priv->tooltip_freeze;
	status_notifier_item_set_from_icon_name(sn, SN_TOOLTIP_ICON, icon_name);
	status_notifier_item_set_tooltip_title(sn, title);
	status_notifier_item_set_tooltip_body(sn, body);
	status_notifier_item_thaw_tooltip(sn);
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

char *status_notifier_item_get_tooltip_body(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), NULL);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return g_strdup(priv->tooltip_body);
}

static void method_call(GDBusConnection *conn _UNUSED_, const char *sender _UNUSED_,
                        const char *object _UNUSED_, const char *interface _UNUSED_,
                        const char *method, GVariant *params, GDBusMethodInvocation *invocation,
                        gpointer data)
{
	StatusNotifierItem *sn = (StatusNotifierItem *)data;
	uint signal;
	gint x, y;
	bool ret;

	if (!g_strcmp0(method, "ContextMenu"))
		signal = SIGNAL_CONTEXT_MENU;
	else if (!g_strcmp0(method, "Activate"))
		signal = SIGNAL_ACTIVATE;
	else if (!g_strcmp0(method, "SecondaryActivate"))
		signal = SIGNAL_SECONDARY_ACTIVATE;
	else if (!g_strcmp0(method, "Scroll"))
	{
		gint delta, orientation;
        char *s_orientation;

		g_variant_get(params, "(is)", &delta, &s_orientation);
		if (!g_ascii_strcasecmp(s_orientation, "vertical"))
			orientation = SN_SCROLL_ORIENTATION_VERTICAL;
		else
			orientation = SN_SCROLL_ORIENTATION_HORIZONTAL;
		g_free(s_orientation);

		g_signal_emit(sn,
		              status_notifier_item_signals[SIGNAL_SCROLL],
		              0,
		              delta,
		              orientation,
		              &ret);
		g_dbus_method_invocation_return_value(invocation, NULL);
		return;
	}
	else
		/* should never happen */
		g_return_if_reached();

	g_variant_get(params, "(ii)", &x, &y);
	g_signal_emit(sn, status_notifier_item_signals[signal], 0, x, y, &ret);
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

	if (G_UNLIKELY(!priv->icon[icon].has_pixbuf))
		return NULL;

	width  = gdk_pixbuf_get_width(priv->icon[icon].pixbuf);
	height = gdk_pixbuf_get_height(priv->icon[icon].pixbuf);

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cr      = cairo_create(surface);
	gdk_cairo_set_source_pixbuf(cr, priv->icon[icon].pixbuf, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);

	stride = cairo_image_surface_get_stride(surface);
	cairo_surface_flush(surface);
	data = (uint *)cairo_image_surface_get_data(surface);
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	uint i, max;

	max = (uint)(stride * height) / sizeof(uint);
	for (i = 0; i < max; ++i)
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
		const char *const *s_category[] = { "ApplicationStatus",
			                            "Communications",
			                            "SystemServices",
			                            "Hardware" };
		return g_variant_new("s", s_category[priv->category]);
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
		return g_variant_new("s",
		                     (!priv->icon[SN_ICON].has_pixbuf)
		                         ? ((priv->icon[SN_ICON].icon_name)
		                                ? priv->icon[SN_ICON].icon_name
		                                : "")
		                         : "");
	else if (!g_strcmp0(property, "IconPixmap"))
		return g_variant_new("a(iiay)", get_builder_for_icon_pixmap(sn, SN_ICON));
	else if (!g_strcmp0(property, "OverlayIconName"))
		return g_variant_new("s",
		                     (!priv->icon[SN_OVERLAY_ICON].has_pixbuf)
		                         ? ((priv->icon[SN_OVERLAY_ICON].icon_name)
		                                ? priv->icon[SN_OVERLAY_ICON].icon_name
		                                : "")
		                         : "");
	else if (!g_strcmp0(property, "OverlayIconPixmap"))
		return g_variant_new("a(iiay)", get_builder_for_icon_pixmap(sn, SN_OVERLAY_ICON));
	else if (!g_strcmp0(property, "AttentionIconName"))
		return g_variant_new("s",
		                     (!priv->icon[SN_ATTENTION_ICON].has_pixbuf)
		                         ? ((priv->icon[SN_ATTENTION_ICON].icon_name)
		                                ? priv->icon[SN_ATTENTION_ICON].icon_name
		                                : "")
		                         : "");
	else if (!g_strcmp0(property, "AttentionIconPixmap"))
		return g_variant_new("a(iiay)", get_builder_for_icon_pixmap(sn, SN_ATTENTION_ICON));
	else if (!g_strcmp0(property, "AttentionMovieName"))
		return g_variant_new("s",
		                     (priv->attention_movie_name) ? priv->attention_movie_name
		                                                  : "");
	else if (!g_strcmp0(property, "ToolTip"))
	{
		GVariant *variant;
		GVariantBuilder *builder;

		if (!priv->icon[SN_TOOLTIP_ICON].has_pixbuf)
		{
			variant = g_variant_new("(sa(iiay)ss)",
			                        (priv->icon[SN_TOOLTIP_ICON].icon_name)
			                            ? priv->icon[SN_TOOLTIP_ICON].icon_name
			                            : "",
			                        NULL,
			                        (priv->tooltip_title) ? priv->tooltip_title : "",
			                        (priv->tooltip_body) ? priv->tooltip_body : "");
			return variant;
		}

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
		return g_variant_new("b", priv->item_is_menu);
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
	GDBusInterfaceVTable interface_vtable = { .method_call  = method_call,
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

StatusNotifierState status_notifier_item_get_state(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), FALSE);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return priv->state;
}

void status_notifier_item_set_item_is_menu(StatusNotifierItem *sn, bool is_menu)
{
	g_return_if_fail(SN_IS_ITEM(sn));
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	priv->item_is_menu = is_menu;
}

bool status_notifier_item_get_item_is_menu(StatusNotifierItem *sn)
{
	g_return_val_if_fail(SN_IS_ITEM(sn), FALSE);
	StatusNotifierItemPrivate *priv =
	    (StatusNotifierItemPrivate *)status_notifier_item_get_instance_private(sn);
	return priv->item_is_menu;
}

bool status_notifier_item_set_context_menu(StatusNotifierItem *sn, GObject *menu)
{
	return FALSE;
}

GObject *status_notifier_item_get_context_menu(StatusNotifierItem *sn)
{
	return NULL;
}
