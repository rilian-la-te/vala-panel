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

#ifndef __SN_PROXY_H__
#define __SN_PROXY_H__

#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define PROXY_PROP_BUS_NAME "bus-name"
#define PROXY_PROP_OBJ_PATH "object-path"
#define PROXY_PROP_ID "id"
#define PROXY_PROP_TITLE "title"
#define PROXY_PROP_CATEGORY "category"
#define PROXY_PROP_STATUS "status"
#define PROXY_PROP_LABEL "x-ayatana-label"
#define PROXY_PROP_LABEL_GUIDE "x-ayatana-label-guide"
#define PROXY_PROP_DESC "accessible-desc"
#define PROXY_PROP_ICON "icon"
#define PROXY_PROP_ICON_SIZE "icon-size"
#define PROXY_PROP_SYMBOLIC "use-symbolic"
#define PROXY_PROP_TOOLTIP_TITLE "tooltip-text"
#define PROXY_PROP_TOOLTIP_ICON "tooltip-icon"
#define PROXY_PROP_MENU_OBJECT_PATH "menu"
#define PROXY_PROP_ORDERING_INDEX "x-ayatana-ordering-index"

#define PROXY_SIGNAL_FAIL "fail"
#define PROXY_SIGNAL_INITIALIZED "initialized"

#define PROXY_DBUS_IFACE_DEFAULT "org.freedesktop.DBus"
#define PROXY_DBUS_PATH_DEFAULT "/org/freedesktop/DBus"
#define PROXY_DBUS_IFACE_PROPS "org.freedesktop.DBus.Properties"
#define PROXY_DBUS_IFACE_KDE "org.kde.StatusNotifierItem"

#define PROXY_SIGNAL_NAME_OWNER_CHANGED "NameOwnerChanged"
#define PROXY_KDE_METHOD_GET_ALL "GetAll"

G_DECLARE_FINAL_TYPE(SnProxy, sn_proxy, SN, PROXY, GObject)

G_GNUC_INTERNAL SnProxy *sn_proxy_new(const char *bus_name, const char *object_path);
G_GNUC_INTERNAL void sn_proxy_start(SnProxy *self);
G_GNUC_INTERNAL void sn_proxy_reload(SnProxy *self);

G_GNUC_INTERNAL void sn_proxy_context_menu(SnProxy *self, gint x_root, gint y_root);
G_GNUC_INTERNAL void sn_proxy_activate(SnProxy *self, gint x_root, gint y_root);
G_GNUC_INTERNAL void sn_proxy_secondary_activate(SnProxy *self, gint x_root, gint y_root);
G_GNUC_INTERNAL bool sn_proxy_ayatana_secondary_activate(SnProxy *self, u_int32_t event_time);
G_GNUC_INTERNAL void sn_proxy_scroll(SnProxy *self, gint delta_x, gint delta_y);
G_GNUC_INTERNAL void sn_proxy_provide_xdg_activation_token(SnProxy *self, const char *token);

G_END_DECLS

#endif /* !__SN_ITEM_H__ */
