
#include "snproxy.h"
#include <gtk/gtk.h>

struct _SnProxy
{
	GObject __parent__;

	bool started;
	bool initialized;

	GCancellable *cancellable;
	GDBusProxy *self_proxy;
	GDBusProxy *properties_proxy;
	uint properties_timeout;

	/* Exposed as properties */
	char *bus_name;
	char *object_path;
	char *id;
	SnStatus status;
	char *x_ayatana_label;
	char *x_ayatana_label_guide;
	GIcon *icon;
	char *icon_theme_path;
	ToolTip *tooltip;
	char *menu_object_path;
	uint x_ayatana_ordering_index;

	/* Internal now */
	char *title;
	char *icon_desc;
	char *attention_desc;

	char *icon_name;
	char *attention_icon_name;
	char *overlay_icon_name;
	GdkPixbuf *icon_pixbuf;
	GdkPixbuf *attention_icon_pixbuf;
	GdkPixbuf *overlay_icon_pixbuf;

	bool items_in_menu;
};

G_DEFINE_TYPE(SnProxy, sn_proxy, G_TYPE_OBJECT)

enum
{
	PROP_0,
	PROP_BUS_NAME,
	PROP_OBJECT_PATH,
	PROP_ID,
	PROP_STATUS,
	PROP_LABEL,
	PROP_LABEL_GUIDE,
	PROP_DESC,
	PROP_ICON,
	PROP_ICON_THEME_PATH,
	PROP_TOOLTIP,
	PROP_MENU_OBJECT_PATH,
	PROP_ORDERING_INDEX,
	PROP_LAST
};

enum
{
	FINISH,
	LAST_SIGNAL
};

static GParamSpec *pspecs[PROP_LAST];
static uint signals[LAST_SIGNAL] = { 0 };

static void sn_proxy_finalize(GObject *object);
static void sn_proxy_get_property(GObject *object, uint prop_id, GValue *value, GParamSpec *pspec);
static void sn_proxy_set_property(GObject *object, uint prop_id, const GValue *value,
                                  GParamSpec *pspec);

typedef struct
{
	GDBusConnection *connection;
	uint handler;
} SubscriptionContext;

static void sn_proxy_class_init(SnProxyClass *klass)
{
	GObjectClass *oclass = G_OBJECT_CLASS(klass);
	pspecs[PROP_BUS_NAME] =
	    g_param_spec_string(PROXY_PROP_BUS_NAME,
	                        PROXY_PROP_BUS_NAME,
	                        PROXY_PROP_BUS_NAME,
	                        NULL,
	                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	pspecs[PROP_OBJECT_PATH] =
	    g_param_spec_string(PROXY_PROP_OBJ_PATH,
	                        PROXY_PROP_OBJ_PATH,
	                        PROXY_PROP_OBJ_PATH,
	                        NULL,
	                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_STATUS] = g_param_spec_enum(PROXY_PROP_STATUS,
	                                        PROXY_PROP_STATUS,
	                                        PROXY_PROP_STATUS,
	                                        sn_status_get_type(),
	                                        SN_STATUS_PASSIVE,
	                                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_ID]     = g_param_spec_string(PROXY_PROP_ID,
                                              PROXY_PROP_LABEL,
                                              PROXY_PROP_LABEL,
                                              NULL,
                                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_LABEL]  = g_param_spec_string(PROXY_PROP_LABEL,
                                                 PROXY_PROP_LABEL,
                                                 PROXY_PROP_LABEL,
                                                 NULL,
                                                 G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	pspecs[PROP_LABEL_GUIDE] = g_param_spec_string(PROXY_PROP_LABEL_GUIDE,
	                                               PROXY_PROP_LABEL_GUIDE,
	                                               PROXY_PROP_LABEL_GUIDE,
	                                               NULL,
	                                               G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	pspecs[PROP_DESC]    = g_param_spec_string(PROXY_PROP_DESC,
                                                PROXY_PROP_DESC,
                                                PROXY_PROP_DESC,
                                                NULL,
                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_ICON]    = g_param_spec_object(PROXY_PROP_ICON,
                                                PROXY_PROP_ICON,
                                                PROXY_PROP_ICON,
                                                G_TYPE_ICON,
                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_TOOLTIP] = g_param_spec_boxed(PROXY_PROP_TOOLTIP,
	                                          PROXY_PROP_TOOLTIP,
	                                          PROXY_PROP_TOOLTIP,
	                                          tooltip_get_type(),
	                                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_MENU_OBJECT_PATH] =
	    g_param_spec_string(PROXY_PROP_MENU_OBJECT_PATH,
	                        PROXY_PROP_MENU_OBJECT_PATH,
	                        PROXY_PROP_MENU_OBJECT_PATH,
	                        NULL,
	                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_ORDERING_INDEX] = g_param_spec_uint(PROXY_PROP_ORDERING_INDEX,
	                                                PROXY_PROP_ORDERING_INDEX,
	                                                PROXY_PROP_ORDERING_INDEX,
	                                                0,
	                                                G_MAXUINT,
	                                                0,
	                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(oclass, PROP_LAST, pspecs);

	signals[FINISH] = g_signal_new(g_intern_static_string(PROXY_SIGNAL_FINISH),
	                               G_TYPE_FROM_CLASS(oclass),
	                               G_SIGNAL_RUN_LAST,
	                               0,
	                               NULL,
	                               NULL,
	                               g_cclosure_marshal_VOID__VOID,
	                               G_TYPE_NONE,
	                               0);

	oclass->finalize     = sn_proxy_finalize;
	oclass->get_property = sn_proxy_get_property;
	oclass->set_property = sn_proxy_set_property;
}

static void sn_proxy_init(SnProxy *self)
{
	self->started     = false;
	self->initialized = false;

	self->cancellable        = g_cancellable_new();
	self->self_proxy         = NULL;
	self->properties_proxy   = NULL;
	self->properties_timeout = 0;

	self->bus_name                 = NULL;
	self->object_path              = NULL;
	self->id                       = NULL;
	self->status                   = SN_STATUS_PASSIVE;
	self->x_ayatana_label          = NULL;
	self->x_ayatana_label_guide    = NULL;
	self->icon                     = NULL;
	self->icon_theme_path          = NULL;
	self->tooltip                  = NULL;
	self->menu_object_path         = NULL;
	self->x_ayatana_ordering_index = 0;

	self->title          = NULL;
	self->icon_desc      = NULL;
	self->attention_desc = NULL;

	self->icon_name             = NULL;
	self->attention_icon_name   = NULL;
	self->overlay_icon_name     = NULL;
	self->icon_pixbuf           = NULL;
	self->attention_icon_pixbuf = NULL;
	self->overlay_icon_pixbuf   = NULL;

	/*DBusMenu ones is much more likely than Activate ones */
	self->items_in_menu = true;
}

static void sn_proxy_finalize(GObject *object)
{
	SnProxy *self = SN_PROXY(object);

	g_clear_object(&self->cancellable);

	if (self->properties_timeout != 0)
		g_source_remove(self->properties_timeout);

	g_clear_object(&self->properties_proxy);
	g_clear_object(&self->self_proxy);

	g_clear_pointer(&self->bus_name, g_free);
	g_clear_pointer(&self->object_path, g_free);
	g_clear_pointer(&self->id, g_free);
	g_clear_pointer(&self->x_ayatana_label, g_free);
	g_clear_pointer(&self->x_ayatana_label_guide, g_free);
	g_clear_object(&self->icon);
	g_clear_pointer(&self->icon_theme_path, g_free);
	g_clear_pointer(&self->tooltip, tooltip_free);
	g_clear_pointer(&self->menu_object_path, g_free);

	g_clear_pointer(&self->title, g_free);
	g_clear_pointer(&self->icon_desc, g_free);
	g_clear_pointer(&self->attention_desc, g_free);

	g_clear_pointer(&self->icon_name, g_free);
	g_clear_pointer(&self->attention_icon_name, g_free);
	g_clear_pointer(&self->overlay_icon_name, g_free);

	g_clear_object(&self->icon_pixbuf);
	g_clear_object(&self->attention_icon_pixbuf);
	g_clear_object(&self->overlay_icon_pixbuf);

	G_OBJECT_CLASS(sn_proxy_parent_class)->finalize(object);
}

static void sn_proxy_get_property(GObject *object, uint prop_id, GValue *value, GParamSpec *pspec)
{
	SnProxy *self = SN_PROXY(object);

	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void sn_proxy_set_property(GObject *object, uint prop_id, const GValue *value,
                                  GParamSpec *pspec)
{
	SnProxy *self = SN_PROXY(object);

	switch (prop_id)
	{
	case PROP_BUS_NAME:
		g_free(self->bus_name);
		self->bus_name = g_value_dup_string(value);
		break;

	case PROP_OBJECT_PATH:
		g_free(self->object_path);
		self->object_path = g_value_dup_string(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

void sn_proxy_context_menu(SnProxy *self, gint x_root, gint y_root)
{
	g_return_if_fail(SN_IS_PROXY(self));
	g_return_if_fail(self->initialized);
	g_return_if_fail(self->self_proxy != NULL);

	g_dbus_proxy_call(self->self_proxy,
	                  "ContextMenu",
	                  g_variant_new("(ii)", x_root, y_root),
	                  G_DBUS_CALL_FLAGS_NONE,
	                  -1,
	                  NULL,
	                  NULL,
	                  NULL);
}

void sn_proxy_activate(SnProxy *self, gint x_root, gint y_root)
{
	g_return_if_fail(SN_IS_PROXY(self));
	g_return_if_fail(self->initialized);
	g_return_if_fail(self->self_proxy != NULL);

	g_dbus_proxy_call(self->self_proxy,
	                  "Activate",
	                  g_variant_new("(ii)", x_root, y_root),
	                  G_DBUS_CALL_FLAGS_NONE,
	                  -1,
	                  NULL,
	                  NULL,
	                  NULL);
}

void sn_proxy_secondary_activate(SnProxy *self, gint x_root, gint y_root)
{
	g_return_if_fail(SN_IS_PROXY(self));
	g_return_if_fail(self->initialized);
	g_return_if_fail(self->self_proxy != NULL);

	g_dbus_proxy_call(self->self_proxy,
	                  "SecondaryActivate",
	                  g_variant_new("(ii)", x_root, y_root),
	                  G_DBUS_CALL_FLAGS_NONE,
	                  -1,
	                  NULL,
	                  NULL,
	                  NULL);
}

void sn_proxy_ayatana_secondary_activate(SnProxy *self, u_int32_t event_time)
{
	g_return_if_fail(SN_IS_PROXY(self));
	g_return_if_fail(self->initialized);
	g_return_if_fail(self->self_proxy != NULL);

	g_dbus_proxy_call(self->self_proxy,
	                  "XAyatanaSecondaryActivate",
	                  g_variant_new("(u)", event_time),
	                  G_DBUS_CALL_FLAGS_NONE,
	                  -1,
	                  NULL,
	                  NULL,
	                  NULL);
}

void sn_proxy_scroll(SnProxy *self, gint delta_x, gint delta_y)
{
	g_return_if_fail(SN_IS_PROXY(self));
	g_return_if_fail(self->initialized);
	g_return_if_fail(self->self_proxy != NULL);

	if (delta_x != 0)
	{
		g_dbus_proxy_call(self->self_proxy,
		                  "Scroll",
		                  g_variant_new("(is)", delta_x, "horizontal"),
		                  G_DBUS_CALL_FLAGS_NONE,
		                  -1,
		                  NULL,
		                  NULL,
		                  NULL);
	}

	if (delta_y != 0)
	{
		g_dbus_proxy_call(self->self_proxy,
		                  "Scroll",
		                  g_variant_new("(is)", delta_y, "vertical"),
		                  G_DBUS_CALL_FLAGS_NONE,
		                  -1,
		                  NULL,
		                  NULL,
		                  NULL);
	}
}
