/*
 * xfce4-sntray-plugin
 * Copyright (C) 2019 Konstantin Pugin <ria.freelander@gmail.com>
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

#include "snproxy.h"
#include "icon-pixmap.h"
#include "sni-enums.h"
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
	char *title;
	SnCategory category;
	SnStatus status;
	char *x_ayatana_label;
	char *x_ayatana_label_guide;
	GIcon *icon;
	GIcon *attention_icon;
	char *icon_theme_path;
	char *tooltip_text;
	GIcon *tooltip_icon;
	char *menu_object_path;
	uint x_ayatana_ordering_index;

	/* Internal now */
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
	PROP_TITLE,
	PROP_CATEGORY,
	PROP_STATUS,
	PROP_DESC,
	PROP_ICON,
	PROP_TOOLTIP_TEXT,
	PROP_TOOLTIP_ICON,
	PROP_MENU_OBJECT_PATH,
	PROP_LABEL,
	PROP_LABEL_GUIDE,
	PROP_ORDERING_INDEX,
	PROP_LAST
};

enum
{
	INITIALIZED,
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
	oclass->finalize     = sn_proxy_finalize;
	oclass->get_property = sn_proxy_get_property;
	oclass->set_property = sn_proxy_set_property;

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
	pspecs[PROP_CATEGORY]     = g_param_spec_enum(PROXY_PROP_CATEGORY,
                                                  PROXY_PROP_CATEGORY,
                                                  PROXY_PROP_CATEGORY,
                                                  sn_category_get_type(),
                                                  SN_CATEGORY_APPLICATION,
                                                  G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_STATUS]       = g_param_spec_enum(PROXY_PROP_STATUS,
                                                PROXY_PROP_STATUS,
                                                PROXY_PROP_STATUS,
                                                sn_status_get_type(),
                                                SN_STATUS_PASSIVE,
                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_ID]           = g_param_spec_string(PROXY_PROP_ID,
                                              PROXY_PROP_ID,
                                              PROXY_PROP_ID,
                                              NULL,
                                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_TITLE]        = g_param_spec_string(PROXY_PROP_TITLE,
                                                 PROXY_PROP_TITLE,
                                                 PROXY_PROP_TITLE,
                                                 NULL,
                                                 G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_DESC]         = g_param_spec_string(PROXY_PROP_DESC,
                                                PROXY_PROP_DESC,
                                                PROXY_PROP_DESC,
                                                NULL,
                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_ICON]         = g_param_spec_object(PROXY_PROP_ICON,
                                                PROXY_PROP_ICON,
                                                PROXY_PROP_ICON,
                                                G_TYPE_ICON,
                                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	pspecs[PROP_TOOLTIP_TEXT] = g_param_spec_string(PROXY_PROP_TOOLTIP_TITLE,
	                                                PROXY_PROP_TOOLTIP_TITLE,
	                                                PROXY_PROP_TOOLTIP_TITLE,
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

	signals[FAIL]        = g_signal_new(g_intern_static_string(PROXY_SIGNAL_FAIL),
                                     G_TYPE_FROM_CLASS(oclass),
                                     G_SIGNAL_RUN_LAST,
                                     0,
                                     NULL,
                                     NULL,
                                     g_cclosure_marshal_VOID__VOID,
                                     G_TYPE_NONE,
                                     0);
	signals[INITIALIZED] = g_signal_new(g_intern_static_string(PROXY_SIGNAL_INITIALIZED),
	                                    G_TYPE_FROM_CLASS(oclass),
	                                    G_SIGNAL_RUN_LAST,
	                                    0,
	                                    NULL,
	                                    NULL,
	                                    g_cclosure_marshal_VOID__VOID,
	                                    G_TYPE_NONE,
	                                    0);
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
	self->tooltip_text             = NULL;
	self->tooltip_icon             = NULL;
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

	if (GTK_IS_ICON_THEME(self->theme))
		g_signal_handlers_disconnect_by_data(self->theme, self);
	if (self->item_proxy)
		g_signal_handlers_disconnect_by_data(self->item_proxy, self);

	g_clear_object(&self->properties_proxy);
	g_clear_object(&self->item_proxy);

	g_clear_pointer(&self->bus_name, g_free);
	g_clear_pointer(&self->object_path, g_free);
	g_clear_pointer(&self->id, g_free);
	g_clear_object(&self->icon);
	g_clear_object(&self->attention_icon);
	g_clear_pointer(&self->icon_theme_path, g_free);
	g_clear_pointer(&self->tooltip_text, g_free);
	g_clear_object(&self->tooltip_icon);
	g_clear_pointer(&self->menu_object_path, g_free);
	g_clear_pointer(&self->x_ayatana_label, g_free);
	g_clear_pointer(&self->x_ayatana_label_guide, g_free);

	g_clear_pointer(&self->title, g_free);
	g_clear_pointer(&self->icon_desc, g_free);
	g_clear_pointer(&self->attention_desc, g_free);

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
	case PROP_TITLE:
		g_value_set_string(value, self->title);
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
	case PROP_TOOLTIP_TEXT:
		if (!string_empty(self->tooltip_text))
			g_value_set_string(value, self->tooltip_text);
		else if (!string_empty(self->attention_desc) && is_attention)
			g_value_set_string(value, self->attention_desc);
		else if (!string_empty(self->icon_desc))
			g_value_set_string(value, self->icon_desc);
		else if (!string_empty(self->title))
			g_value_set_string(value, self->title);
		break;
	case PROP_TOOLTIP_ICON:
		if (self->tooltip_icon)
			g_value_set_object(value, self->tooltip_icon);
		else if (self->attention_icon && is_attention)
			g_value_set_object(value, self->attention_icon);
		else if (self->icon)
			g_value_set_object(value, self->icon);
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
			if (self->initialized)
				sn_proxy_reload(self);
		}
		break;
	case PROP_SYMBOLIC:
		if (self->use_symbolic != g_value_get_boolean(value))
		{
			self->use_symbolic = g_value_get_boolean(value);
			if (self->initialized)
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
	if (string_empty(new_owner))
		g_signal_emit(self, signals[FAIL], 0);
}

static GIcon *sn_proxy_load_icon(SnProxy *self, const char *icon_name, IconPixmap *pixmap,
                                 const char *overlay, IconPixmap *opixmap)
{
	g_autoptr(GIcon) tmp_main_icon    = icon_pixmap_select_icon(icon_name,
                                                                 pixmap,
                                                                 self->theme,
                                                                 self->icon_theme_path,
                                                                 self->icon_size,
                                                                 self->use_symbolic);
	g_autoptr(GIcon) tmp_overlay_icon = icon_pixmap_select_icon(overlay,
	                                                            opixmap,
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
	if (!icon)
		return NULL;
	return g_object_ref(icon);
}

static void sn_proxy_reload_finish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	if (!SN_IS_PROXY(user_data))
		return;

	SnProxy *self                 = SN_PROXY(user_data);
	bool update_tooltip           = false;
	bool update_icon              = false;
	bool update_attention_icon    = false;
	bool update_desc              = false;
	bool update_menu              = false;
	ToolTip *new_tooltip          = NULL;
	g_autofree char *icon_name    = NULL;
	g_autofree char *att_name     = NULL;
	g_autofree char *overlay_name = NULL;
	IconPixmap *icon_pixmap       = NULL;
	IconPixmap *att_pixmap        = NULL;
	IconPixmap *overlay_pixmap    = NULL;
	g_autoptr(GError) error       = NULL;
	g_autoptr(GVariant) properties =
	    g_dbus_proxy_call_finish(G_DBUS_PROXY(source_object), res, &error);
	if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		return;
	if (!properties)
	{
		g_signal_emit(self, signals[FAIL], 0);
		return;
	}
	GVariantIter *iter;
	g_variant_get(properties, "(a{sv})", &iter);
	char *name;
	GVariant *value;
	while (g_variant_iter_loop(iter, "{&sv}", &name, &value))
	{
		if (!g_strcmp0(name, "XAyatanaLabel"))
		{
			if (g_strcmp0(g_variant_get_string(value, NULL), self->x_ayatana_label))
			{
				g_clear_pointer(&self->x_ayatana_label, g_free);
				self->x_ayatana_label = g_variant_dup_string(value, NULL);
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_LABEL]);
			}
		}
		else if (!g_strcmp0(name, "XAyatanaLabelGuide"))
		{
			if (g_strcmp0(g_variant_get_string(value, NULL),
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
			{
				self->status = new_st;
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_STATUS]);
			}
		}
		else if (!g_strcmp0(name, "Title"))
		{
			if (g_strcmp0(g_variant_get_string(value, NULL), self->title))
			{
				g_clear_pointer(&self->title, g_free);
				self->title = g_variant_dup_string(value, NULL);
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_TITLE]);
				update_tooltip = true;
			}
		}
		else if (!g_strcmp0(name, "Menu"))
		{
			if (g_strcmp0(g_variant_get_string(value, NULL), self->menu_object_path))
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
		else if (!g_strcmp0(name, "IconAccessibleDesc"))
		{
			if (g_strcmp0(g_variant_get_string(value, NULL), self->icon_desc))
			{
				g_clear_pointer(&self->icon_desc, g_free);
				self->icon_desc = g_variant_dup_string(value, NULL);
				update_desc     = true;
				update_tooltip  = true;
			}
		}
		else if (!g_strcmp0(name, "AttentionAccessibleDesc"))
		{
			if (g_strcmp0(g_variant_get_string(value, NULL), self->attention_desc))
			{
				g_clear_pointer(&self->attention_desc, g_free);
				self->attention_desc = g_variant_dup_string(value, NULL);
				if (self->status == SN_STATUS_ATTENTION)
				{
					update_desc    = true;
					update_tooltip = true;
				}
			}
		}
		else if (!g_strcmp0(name, "IconThemePath"))
		{
			if (g_strcmp0(g_variant_get_string(value, NULL), self->icon_theme_path))
			{
				g_clear_pointer(&self->icon_theme_path, g_free);
				self->icon_theme_path = g_variant_dup_string(value, NULL);
				gtk_icon_theme_append_search_path(self->theme,
				                                  self->icon_theme_path);
				update_icon           = true;
				update_attention_icon = true;
			}
		}
		else if (!g_strcmp0(name, "IconName"))
		{
			icon_name   = g_variant_dup_string(value, NULL);
			update_icon = true;
		}
		else if (!g_strcmp0(name, "AttentionIconName"))
		{
			att_name              = g_variant_dup_string(value, NULL);
			update_attention_icon = true;
		}
		else if (!g_strcmp0(name, "OverlayIconName"))
		{
			overlay_name          = g_variant_dup_string(value, NULL);
			update_icon           = true;
			update_attention_icon = true;
		}
		else if (!g_strcmp0(name, "IconPixmap"))
		{
			icon_pixmap = icon_pixmap_new_with_size(value, self->icon_size);
			update_icon = true;
		}
		else if (!g_strcmp0(name, "AttentionIconPixmap"))
		{
			att_pixmap            = icon_pixmap_new_with_size(value, self->icon_size);
			update_attention_icon = true;
		}
		else if (!g_strcmp0(name, "OverlayIconPixmap"))
		{
			overlay_pixmap        = icon_pixmap_new_with_size(value, self->icon_size);
			update_icon           = true;
			update_attention_icon = true;
		}
		else if (!g_strcmp0(name, "ToolTip"))
		{
			new_tooltip    = tooltip_new(value);
			update_tooltip = true;
		}
	}
	g_clear_pointer(&iter, g_variant_iter_free);
	g_clear_pointer(&name, g_free);
	g_clear_pointer(&value, g_variant_unref);
	if (update_desc)
		g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_DESC]);
	if (update_menu)
		g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_MENU_OBJECT_PATH]);
	if (update_tooltip)
	{
		char *markup     = NULL;
		GIcon *icon      = NULL;
		bool notify_text = false;
		bool notify_icon = false;
		if (new_tooltip)
		{
			unbox_tooltip(new_tooltip,
			              self->theme,
			              self->icon_theme_path,
			              &icon,
			              &markup);
			g_clear_pointer(&new_tooltip, tooltip_free);
		}
		if (g_strcmp0(markup, self->tooltip_text))
			notify_text = true;
		if (!g_icon_equal(self->tooltip_icon, icon))
			notify_icon = true;
		g_clear_pointer(&self->tooltip_text, g_free);
		self->tooltip_text = markup;
		g_clear_object(&self->tooltip_icon);
		self->tooltip_icon = icon;
		if (notify_text)
			g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_TOOLTIP_TEXT]);
		if (notify_icon)
			g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_TOOLTIP_ICON]);
	}
	if (update_icon)
	{
		g_autoptr(GIcon) new_icon =
		    sn_proxy_load_icon(self, icon_name, icon_pixmap, overlay_name, overlay_pixmap);
		if (!g_icon_equal(self->icon, new_icon))
		{
			g_clear_object(&self->icon);
			self->icon = g_object_ref(new_icon);
			g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_ICON]);
		}
	}
	if (update_attention_icon)
	{
		g_autoptr(GIcon) new_icon =
		    sn_proxy_load_icon(self, att_name, att_pixmap, overlay_name, overlay_pixmap);
		if (!g_icon_equal(self->attention_icon, new_icon))
		{
			g_clear_object(&self->attention_icon);
			self->attention_icon = g_object_ref(new_icon);
			if (self->status == SN_STATUS_ATTENTION)
				g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_ICON]);
		}
	}
	g_clear_pointer(&icon_pixmap, icon_pixmap_free);
	g_clear_pointer(&att_pixmap, icon_pixmap_free);
	g_clear_pointer(&overlay_pixmap, icon_pixmap_free);
	if (!self->initialized)
	{
		if (self->id != NULL)
		{
			self->initialized = true;
			g_signal_connect_swapped(self->theme,
			                         "changed",
			                         G_CALLBACK(sn_proxy_reload),
			                         self);
			g_signal_emit(self, signals[INITIALIZED], 0);
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
	if (!SN_IS_PROXY(user_data))
		return;

	SnProxy *self           = SN_PROXY(user_data);
	g_autoptr(GError) error = NULL;

	self->properties_proxy = g_dbus_proxy_new_for_bus_finish(res, &error);
	if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		return;
	if (self->properties_proxy == NULL)
	{
		g_signal_emit(self, signals[FAIL], 0);
		return;
	}
	sn_proxy_reload(self);
}

static void sn_proxy_signal_received(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name,
                                     GVariant *parameters, gpointer user_data)
{
	SnProxy *self = SN_PROXY(user_data);
	if (!self->initialized)
		return;

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
		g_autofree char *status = NULL;
		g_variant_get(parameters, "(s)", &status);
		self->status = sn_status_get_value_from_nick(status);
		g_object_notify_by_pspec(G_OBJECT(self), pspecs[PROP_STATUS]);
	}
}

static void sn_proxy_item_callback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	if (!SN_IS_PROXY(user_data))
		return;

	SnProxy *self           = SN_PROXY(user_data);
	g_autoptr(GError) error = NULL;

	self->item_proxy = g_dbus_proxy_new_for_bus_finish(res, &error);
	if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		return;
	if (self->item_proxy == NULL)
	{
		g_signal_emit(G_OBJECT(self), signals[FAIL], 0);
		return;
	}

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

SnProxy *sn_proxy_new(const char *bus_name, const char *object_path)
{
	return SN_PROXY(g_object_new(sn_proxy_get_type(),
	                             PROXY_PROP_BUS_NAME,
	                             bus_name,
	                             PROXY_PROP_OBJ_PATH,
	                             object_path,
	                             NULL));
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

void sn_proxy_context_menu(SnProxy *self, int x_root, int y_root)
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

void sn_proxy_activate(SnProxy *self, int x_root, int y_root)
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

void sn_proxy_secondary_activate(SnProxy *self, int x_root, int y_root)
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

bool sn_proxy_ayatana_secondary_activate(SnProxy *self, u_int32_t event_time)
{
	g_return_val_if_fail(SN_IS_PROXY(self), false);
	g_return_val_if_fail(self->initialized, false);
	g_return_val_if_fail(self->item_proxy != NULL, false);
	g_autoptr(GError) err = NULL;

	g_dbus_proxy_call_sync(self->item_proxy,
	                       "XAyatanaSecondaryActivate",
	                       g_variant_new("(u)", event_time),
	                       G_DBUS_CALL_FLAGS_NONE,
	                       -1,
	                       NULL,
	                       &err);
	if (err)
		return false;
	return true;
}

void sn_proxy_scroll(SnProxy *self, int delta_x, int delta_y)
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
