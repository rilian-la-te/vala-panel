
#include "snproxy.h"
#include <gtk/gtk.h>
#include <math.h>

struct _SnProxy
{
	GObject __parent__;

	bool started;
	bool initialized;

	GCancellable *cancellable;
	GDBusProxy *item_proxy;
	GDBusProxy *properties_proxy;
	uint properties_timeout;

	/* Exposed as properties */
	char *bus_name;
	char *object_path;
	char *id;
	SnCategory category;
	SnStatus status;
	char *x_ayatana_label;
	char *x_ayatana_label_guide;
	GIcon *icon;
	GIcon *attention_icon;
	char *icon_theme_path;
	ToolTip *tooltip;
	char *menu_object_path;
	uint x_ayatana_ordering_index;

	/* Internal now */
	char *title;
	char *icon_desc;
	char *attention_desc;
	int icon_size;

	bool item_is_menu;

	GtkIconTheme *theme;
	bool use_symbolic;
};

G_DEFINE_TYPE(SnProxy, sn_proxy, G_TYPE_OBJECT)

enum
{
	PROP_0,
	PROP_BUS_NAME,
	PROP_OBJECT_PATH,
	PROP_ICON_SIZE,
	PROP_SYMBOLIC,
	PROP_ID,
	PROP_CATEGORY,
	PROP_STATUS,
	PROP_DESC,
	PROP_ICON,
	PROP_ICON_THEME_PATH,
	PROP_TOOLTIP_TITLE,
	PROP_TOOLTIP_DESCRIPTION,
	PROP_TOOLTIP_ICON,
	PROP_MENU_OBJECT_PATH,
	PROP_LABEL,
	PROP_LABEL_GUIDE,
	PROP_ORDERING_INDEX,
	PROP_LAST
};

enum
{
	FAIL,
	LAST_SIGNAL
};

static GParamSpec *pspecs[PROP_LAST];
static uint signals[LAST_SIGNAL] = { 0 };

void sn_proxy_reload(SnProxy *self);
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
	pspecs[PROP_ICON_SIZE] =
	    g_param_spec_int(PROXY_PROP_ICON_SIZE,
	                     PROXY_PROP_ICON_SIZE,
	                     PROXY_PROP_ICON_SIZE,
	                     0,
	                     256,
	                     0,
	                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_SYMBOLIC] =
	    g_param_spec_boolean(PROXY_PROP_SYMBOLIC,
	                         PROXY_PROP_SYMBOLIC,
	                         PROXY_PROP_SYMBOLIC,
	                         true,
	                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_CATEGORY]      = g_param_spec_enum(PROXY_PROP_CATEGORY,
                                                  PROXY_PROP_CATEGORY,
                                                  PROXY_PROP_CATEGORY,
                                                  sn_category_get_type(),
                                                  SN_CATEGORY_APPLICATION,
                                                  G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_STATUS]        = g_param_spec_enum(PROXY_PROP_STATUS,
                                                PROXY_PROP_STATUS,
                                                PROXY_PROP_STATUS,
                                                sn_status_get_type(),
                                                SN_STATUS_PASSIVE,
                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_ID]            = g_param_spec_string(PROXY_PROP_ID,
                                              PROXY_PROP_LABEL,
                                              PROXY_PROP_LABEL,
                                              NULL,
                                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_DESC]          = g_param_spec_string(PROXY_PROP_DESC,
                                                PROXY_PROP_DESC,
                                                PROXY_PROP_DESC,
                                                NULL,
                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_ICON]          = g_param_spec_object(PROXY_PROP_ICON,
                                                PROXY_PROP_ICON,
                                                PROXY_PROP_ICON,
                                                G_TYPE_ICON,
                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_TOOLTIP_TITLE] = g_param_spec_string(PROXY_PROP_TOOLTIP_TITLE,
	                                                 PROXY_PROP_TOOLTIP_TITLE,
	                                                 PROXY_PROP_TOOLTIP_TITLE,
	                                                 NULL,
	                                                 G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_TOOLTIP_DESCRIPTION] =
	    g_param_spec_string(PROXY_PROP_TOOLTIP_DESCRIPTION,
	                        PROXY_PROP_TOOLTIP_DESCRIPTION,
	                        PROXY_PROP_TOOLTIP_DESCRIPTION,
	                        NULL,
	                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_TOOLTIP_ICON] = g_param_spec_object(PROXY_PROP_TOOLTIP_ICON,
	                                                PROXY_PROP_TOOLTIP_ICON,
	                                                PROXY_PROP_TOOLTIP_ICON,
	                                                G_TYPE_ICON,
	                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_MENU_OBJECT_PATH] =
	    g_param_spec_string(PROXY_PROP_MENU_OBJECT_PATH,
	                        PROXY_PROP_MENU_OBJECT_PATH,
	                        PROXY_PROP_MENU_OBJECT_PATH,
	                        NULL,
	                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	pspecs[PROP_LABEL]          = g_param_spec_string(PROXY_PROP_LABEL,
                                                 PROXY_PROP_LABEL,
                                                 PROXY_PROP_LABEL,
                                                 NULL,
                                                 G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_LABEL_GUIDE]    = g_param_spec_string(PROXY_PROP_LABEL_GUIDE,
                                                       PROXY_PROP_LABEL_GUIDE,
                                                       PROXY_PROP_LABEL_GUIDE,
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

	signals[FAIL] = g_signal_new(g_intern_static_string(PROXY_SIGNAL_FAIL),
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
	self->item_proxy         = NULL;
	self->properties_proxy   = NULL;
	self->properties_timeout = 0;

	self->bus_name                 = NULL;
	self->object_path              = NULL;
	self->id                       = NULL;
	self->category                 = SN_CATEGORY_APPLICATION;
	self->status                   = SN_STATUS_PASSIVE;
	self->attention_icon           = NULL;
	self->icon                     = NULL;
	self->icon_theme_path          = NULL;
	self->tooltip                  = tooltip_new(NULL);
	self->menu_object_path         = NULL;
	self->x_ayatana_label          = NULL;
	self->x_ayatana_label_guide    = NULL;
	self->x_ayatana_ordering_index = 0;

	self->title          = NULL;
	self->icon_desc      = NULL;
	self->attention_desc = NULL;

	self->theme = gtk_icon_theme_get_default();

	/*DBusMenu ones is much more likely than Activate ones */
	self->item_is_menu = true;
}

static void sn_proxy_finalize(GObject *object)
{
	SnProxy *self = SN_PROXY(object);

	g_clear_object(&self->cancellable);

	if (self->properties_timeout != 0)
		g_source_remove(self->properties_timeout);

	g_clear_object(&self->properties_proxy);
	g_clear_object(&self->item_proxy);

	g_clear_pointer(&self->bus_name, g_free);
	g_clear_pointer(&self->object_path, g_free);
	g_clear_pointer(&self->id, g_free);
	g_clear_object(&self->icon);
	g_clear_object(&self->attention_icon);
	g_clear_pointer(&self->icon_theme_path, g_free);
	g_clear_pointer(&self->tooltip, tooltip_free);
	g_clear_pointer(&self->menu_object_path, g_free);
	g_clear_pointer(&self->x_ayatana_label, g_free);
	g_clear_pointer(&self->x_ayatana_label_guide, g_free);

	g_clear_pointer(&self->title, g_free);
	g_clear_pointer(&self->icon_desc, g_free);
	g_clear_pointer(&self->attention_desc, g_free);

	g_clear_object(&self->theme);

	G_OBJECT_CLASS(sn_proxy_parent_class)->finalize(object);
}

static void sn_proxy_get_property(GObject *object, uint prop_id, GValue *value, GParamSpec *pspec)
{
	SnProxy *self     = SN_PROXY(object);
	bool is_attention = self->status == SN_STATUS_ATTENTION;

	switch (prop_id)
	{
	case PROP_ID:
		g_value_set_string(value, self->id);
		break;
	case PROP_CATEGORY:
		g_value_set_enum(value, self->category);
		break;
	case PROP_STATUS:
		g_value_set_enum(value, self->status);
		break;
	case PROP_SYMBOLIC:
		g_value_set_uint(value, self->use_symbolic);
		break;
	case PROP_ICON_SIZE:
		g_value_set_int(value, self->icon_size);
		break;
	case PROP_DESC:
		g_value_set_string(value,
		                   is_attention && !string_empty(self->attention_desc)
		                       ? self->attention_desc
		                       : self->icon_desc);
		break;
	case PROP_ICON:
		g_value_set_object(value,
		                   is_attention && self->attention_icon ? self->attention_icon
		                                                        : self->icon);
		break;
	case PROP_TOOLTIP_TITLE:
		g_value_set_string(value, self->tooltip->title);
		break;
	case PROP_LABEL:
		g_value_set_string(value, self->x_ayatana_label);
		break;
	case PROP_LABEL_GUIDE:
		g_value_set_string(value, self->x_ayatana_label_guide);
		break;
	case PROP_ORDERING_INDEX:
		g_value_set_uint(value, self->x_ayatana_ordering_index);
		break;
	case PROP_ICON_THEME_PATH:
		g_value_set_string(value, self->icon_theme_path);
		break;
	case PROP_MENU_OBJECT_PATH:
		g_value_set_string(value, self->menu_object_path);
		break;
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
	case PROP_ICON_SIZE:
		if (self->icon_size != g_value_get_int(value))
		{
			self->icon_size = g_value_get_int(value);
			sn_proxy_reload(self);
		}
		break;
	case PROP_SYMBOLIC:
		if (self->use_symbolic != g_value_get_boolean(value))
		{
			self->use_symbolic = g_value_get_boolean(value);
			sn_proxy_reload(self);
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void sn_proxy_unsubscribe(void *data, GObject *unused)
{
	SubscriptionContext *context = data;

	g_dbus_connection_signal_unsubscribe(context->connection, context->handler);
	g_clear_pointer(&context, g_free);
}

static void sn_proxy_name_owner_changed(GDBusConnection *connection, const char *sender_name,
                                        const char *object_path, const char *interface_name,
                                        const char *signal_name, GVariant *parameters,
                                        void *user_data)
{
	SnProxy *self              = user_data;
	g_autofree char *new_owner = NULL;

	g_variant_get(parameters, "(sss)", NULL, NULL, &new_owner);
	if ((new_owner == NULL || strlen(new_owner) == 0))
		g_signal_emit(self, signals[FAIL], 0);
}

static GIcon *sn_proxy_load_icon(SnProxy *self, const char *icon_name, const IconPixmap **pixmaps,
                                 const char *overlay, const IconPixmap **opixmaps)
{
	g_autoptr(GIcon) tmp_main_icon    = icon_pixmap_select_icon(icon_name,
                                                                 pixmaps,
                                                                 self->theme,
                                                                 self->icon_theme_path,
                                                                 self->icon_size,
                                                                 self->use_symbolic);
	g_autoptr(GIcon) tmp_overlay_icon = icon_pixmap_select_icon(overlay,
	                                                            opixmaps,
	                                                            self->theme,
	                                                            self->icon_theme_path,
	                                                            self->icon_size / 4,
	                                                            self->use_symbolic);
	g_autoptr(GEmblem) overlay_icon   = NULL;
	g_autoptr(GIcon) icon             = NULL;
	if (tmp_overlay_icon)
		overlay_icon = g_emblem_new(tmp_overlay_icon);
	if (tmp_main_icon)
		icon = g_emblemed_icon_new(tmp_main_icon, overlay_icon);
	g_autoptr(GtkIconInfo) info =
	    gtk_icon_theme_lookup_by_gicon(self->theme, icon, self->icon_size, 0);
	g_autoptr(GError) err = NULL;
	if (info)
	{
		g_autoptr(GdkPixbuf) first_pixbuf = gtk_icon_info_load_icon(info, &err);
		if (err)
			return g_object_ref(icon);
		double aspect_ratio =
		    gdk_pixbuf_get_width(first_pixbuf) / gdk_pixbuf_get_height(first_pixbuf);
		if (aspect_ratio - 1 > fabs(0.000000001))
		{
			g_autoptr(GtkIconInfo) sinfo =
			    gtk_icon_theme_lookup_by_gicon(self->theme,
			                                   icon,
			                                   (int)(round(self->icon_size *
			                                               aspect_ratio)),
			                                   0);
			g_autoptr(GdkPixbuf) spixbuf = gtk_icon_info_load_icon(sinfo, &err);
			if (err)
				return g_object_ref(icon);
			g_autoptr(GdkPixbuf) tpixbuf =
			    gdk_pixbuf_scale_simple(spixbuf,
			                            (int)(round(self->icon_size * aspect_ratio)),
			                            self->icon_size,
			                            GDK_INTERP_BILINEAR);
			return g_object_ref(G_ICON(tpixbuf));
		}
		else
		{
			return g_object_ref(G_ICON(first_pixbuf));
		}
	}
	return g_object_ref(icon);
}

static void sn_proxy_reload_finish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	SnProxy *self                      = user_data;
	bool update_tooltip                = false;
	bool update_tooltip_title          = false;
	bool update_icon                   = false;
	bool update_attention_icon         = false;
	bool update_desc                   = false;
	bool update_menu                   = false;
	ToolTip *new_tooltip               = NULL;
	char *icon_name                    = NULL;
	char *attention_icon_name          = NULL;
	char *overlay_icon_name            = NULL;
	IconPixmap **icon_pixmap           = NULL;
	IconPixmap **attention_icon_pixmap = NULL;
	IconPixmap **overlay_icon_pixmap   = NULL;
	g_autoptr(GError) error            = NULL;
	g_autoptr(GVariant) properties =
	    g_dbus_proxy_call_finish(G_DBUS_PROXY(source_object), res, &error);
	if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		return;
	if (!properties)
		g_signal_emit(self, signals[FAIL], 0);
	GVariantIter *iter;
	g_variant_get(properties, "(a{sv})", &iter);
	char *name;
	GVariant *value;
	while (g_variant_iter_loop(iter, "{&sv}", &name, &value))
	{
		if (!g_strcmp0(name, "XAyatanaLabel"))
		{
			if (!g_strcmp0(g_variant_get_string(value, NULL), self->x_ayatana_label))
			{
				g_clear_pointer(&self->x_ayatana_label, g_free);
				self->x_ayatana_label = g_variant_dup_string(value, NULL);
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_LABEL]);
			}
		}
		else if (!g_strcmp0(name, "XAyatanaLabelGuide"))
		{
			if (!g_strcmp0(g_variant_get_string(value, NULL),
			               self->x_ayatana_label_guide))
			{
				g_clear_pointer(&self->x_ayatana_label_guide, g_free);
				self->x_ayatana_label_guide = g_variant_dup_string(value, NULL);
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_LABEL_GUIDE]);
			}
		}
		else if (!g_strcmp0(name, "XAyatanaOrderingIndex"))
		{
			if (g_variant_get_uint32(value) != self->x_ayatana_ordering_index)
			{
				self->x_ayatana_ordering_index = g_variant_get_uint32(value);
				g_object_notify_by_pspec(G_OBJECT(self),
				                         pspecs[PROP_ORDERING_INDEX]);
			}
		}
		else if (!g_strcmp0(name, "Category"))
		{
			SnCategory new_cat =
			    sn_category_get_value_from_nick(g_variant_get_string(value, NULL));
			if (self->category != new_cat)
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_CATEGORY]);
		}
		else if (!g_strcmp0(name, "Id"))
		{
			if (!self->id || g_strcmp0(g_variant_get_string(value, NULL), self->id))
			{
				g_clear_pointer(&self->id, g_free);
				self->id = g_variant_dup_string(value, NULL);
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_ID]);
			}
		}
		else if (!g_strcmp0(name, "Status"))
		{
			SnStatus new_st =
			    sn_status_get_value_from_nick(g_variant_get_string(value, NULL));
			if (self->status != new_st)
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_STATUS]);
		}
		else if (!g_strcmp0(name, "Title"))
		{
			if (!g_strcmp0(g_variant_get_string(value, NULL), self->title))
			{
				g_clear_pointer(&self->title, g_free);
				self->title    = g_variant_dup_string(value, NULL);
				update_tooltip = true;
			}
		}
		else if (!g_strcmp0(name, "Menu"))
		{
			if (!g_strcmp0(g_variant_get_string(value, NULL), self->menu_object_path))
			{
				g_clear_pointer(&self->menu_object_path, g_free);
				self->menu_object_path = g_variant_dup_string(value, NULL);
				update_menu            = true;
			}
		}
		else if (!g_strcmp0(name, "ItemIsMenu"))
		{
			if (g_variant_get_boolean(value) != self->item_is_menu)
			{
				self->item_is_menu = g_variant_get_boolean(value);
				update_menu        = true;
			}
		}
	}
	if (update_desc)
		g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_DESC]);
	if (update_menu)
		g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_DESC]);
	if (update_tooltip)
	{
		if (!tooltip_equal(self->tooltip, new_tooltip))
		{
			if (new_tooltip)
			{
				g_clear_pointer(&self->tooltip, tooltip_free);
				self->tooltip = new_tooltip;
				if (!self->tooltip->title)
					self->tooltip->title = g_strdup(self->title);
				g_object_notify_by_pspec(G_OBJECT(self),
				                         pspecs[PROP_TOOLTIP_TITLE]);
			}
			if (!new_tooltip && g_strcmp0(self->tooltip->title, self->title))
			{
				g_clear_pointer(&self->tooltip->title, g_free);
				self->tooltip->title = g_strdup(self->title);
				g_object_notify_by_pspec(G_OBJECT(self),
				                         pspecs[PROP_TOOLTIP_TITLE]);
			}
		}
	}
	if (update_icon)
	{
		g_autoptr(GIcon) new_icon = sn_proxy_load_icon(self,
		                                               icon_name,
		                                               icon_pixmap,
		                                               overlay_icon_name,
		                                               overlay_icon_pixmap);
		if (!g_icon_equal(self->icon, new_icon))
		{
			g_clear_object(&self->icon);
			self->icon = g_object_ref(new_icon);
			g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_ICON]);
		}
	}
	if (update_attention_icon)
	{
		g_autoptr(GIcon) new_icon = sn_proxy_load_icon(self,
		                                               attention_icon_name,
		                                               attention_icon_pixmap,
		                                               overlay_icon_name,
		                                               overlay_icon_pixmap);
		if (!g_icon_equal(self->attention_icon, new_icon))
		{
			g_clear_object(&self->attention_icon);
			self->attention_icon = g_object_ref(new_icon);
			if (self->status == SN_STATUS_ATTENTION)
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_ICON]);
		}
	}
}

static int sn_proxy_reload_begin(gpointer user_data)
{
	SnProxy *self = user_data;

	self->properties_timeout = 0;

	g_dbus_proxy_call(self->properties_proxy,
	                  "GetAll",
	                  g_variant_new("(s)", "org.kde.StatusNotifierItem"),
	                  G_DBUS_CALL_FLAGS_NONE,
	                  -1,
	                  self->cancellable,
	                  sn_proxy_reload_finish,
	                  self);

	return G_SOURCE_REMOVE;
}

void sn_proxy_reload(SnProxy *self)
{
	g_return_if_fail(SN_IS_PROXY(self));
	g_return_if_fail(self->properties_proxy != NULL);

	/* same approach as in Plasma Workspace */
	if (self->properties_timeout != 0)
		g_source_remove(self->properties_timeout);
	self->properties_timeout = g_timeout_add(10, sn_proxy_reload_begin, self);
}

static void sn_proxy_properties_callback(GObject *source_object, GAsyncResult *res,
                                         gpointer user_data)
{
	SnProxy *self           = SN_PROXY(user_data);
	g_autoptr(GError) error = NULL;

	self->properties_proxy = g_dbus_proxy_new_for_bus_finish(res, &error);
	if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		return;
	if (self->properties_proxy == NULL)
		g_signal_emit(self, signals[FAIL], 0);
	sn_proxy_reload(self);
}

static void sn_proxy_signal_received(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name,
                                     GVariant *parameters, gpointer user_data)
{
	SnProxy *self = user_data;

	if (!g_strcmp0(signal_name, "NewTitle") || !g_strcmp0(signal_name, "NewIcon") ||
	    !g_strcmp0(signal_name, "NewAttentionIcon") ||
	    !g_strcmp0(signal_name, "NewOverlayIcon") || !g_strcmp0(signal_name, "NewToolTip"))
	{
		// Reload all properties in async mode.
		sn_proxy_reload(self);
	}
	else if (!g_strcmp0(signal_name, "NewStatus"))
	{
		// Just notify for status change, requires fast reaction.
		const char *status;
		g_variant_get(parameters, "(s)", &status);
		self->status = sn_status_get_value_from_nick(status);
		g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_STATUS]);
	}
}

static void sn_proxy_item_callback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	SnProxy *self           = SN_PROXY(user_data);
	g_autoptr(GError) error = NULL;

	self->item_proxy = g_dbus_proxy_new_for_bus_finish(res, &error);
	if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		return;
	if (self->item_proxy == NULL)
		;

	SubscriptionContext *context = g_new0(SubscriptionContext, 1);
	context->connection          = g_dbus_proxy_get_connection(self->item_proxy);
	context->handler =
	    g_dbus_connection_signal_subscribe(g_dbus_proxy_get_connection(self->item_proxy),
	                                       "org.freedesktop.DBus",
	                                       "org.freedesktop.DBus",
	                                       "NameOwnerChanged",
	                                       "/org/freedesktop/DBus",
	                                       g_dbus_proxy_get_name(self->item_proxy),
	                                       G_DBUS_SIGNAL_FLAGS_NONE,
	                                       sn_proxy_name_owner_changed,
	                                       self,
	                                       NULL);
	g_object_weak_ref(G_OBJECT(self->item_proxy), sn_proxy_unsubscribe, context);

	g_signal_connect(self->item_proxy, "g-signal", G_CALLBACK(sn_proxy_signal_received), self);

	g_dbus_proxy_new(g_dbus_proxy_get_connection(self->item_proxy),
	                 G_DBUS_PROXY_FLAGS_NONE,
	                 NULL,
	                 self->bus_name,
	                 self->object_path,
	                 "org.freedesktop.DBus.Properties",
	                 self->cancellable,
	                 sn_proxy_properties_callback,
	                 self);
}

static int sn_proxy_start_failed(void *user_data)
{
	SnProxy *self = user_data;

	/* start is failed, emit the signel in next loop iteration */
	g_signal_emit(G_OBJECT(self), signals[FAIL], 0);

	return G_SOURCE_REMOVE;
}

void sn_proxy_start(SnProxy *self)
{
	g_return_if_fail(SN_IS_PROXY(self));
	g_return_if_fail(!self->started);

	if (!g_dbus_is_name(self->bus_name))
	{
		g_idle_add(sn_proxy_start_failed, self);
		return;
	}

	self->started = true;
	g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
	                         G_DBUS_PROXY_FLAGS_NONE,
	                         NULL,
	                         self->bus_name,
	                         self->object_path,
	                         "org.kde.StatusNotifierItem",
	                         self->cancellable,
	                         sn_proxy_item_callback,
	                         self);
}

void sn_proxy_context_menu(SnProxy *self, gint x_root, gint y_root)
{
	g_return_if_fail(SN_IS_PROXY(self));
	g_return_if_fail(self->initialized);
	g_return_if_fail(self->item_proxy != NULL);

	g_dbus_proxy_call(self->item_proxy,
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
	g_return_if_fail(self->item_proxy != NULL);

	g_dbus_proxy_call(self->item_proxy,
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
	g_return_if_fail(self->item_proxy != NULL);

	g_dbus_proxy_call(self->item_proxy,
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
	g_return_if_fail(self->item_proxy != NULL);

	g_dbus_proxy_call(self->item_proxy,
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
	g_return_if_fail(self->item_proxy != NULL);

	if (delta_x != 0)
	{
		g_dbus_proxy_call(self->item_proxy,
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
		g_dbus_proxy_call(self->item_proxy,
		                  "Scroll",
		                  g_variant_new("(is)", delta_y, "vertical"),
		                  G_DBUS_CALL_FLAGS_NONE,
		                  -1,
		                  NULL,
		                  NULL,
		                  NULL);
	}
}
