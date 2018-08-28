/*
 * statusnotifier - Copyright (C) 2014-2017 Olivier Brunel
 *
 * statusnotifier.h
 * Copyright (C) 2014-2017 Olivier Brunel <jjk@jjacky.com>
 *
 * This file is part of statusnotifier.
 *
 * statusnotifier is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * statusnotifier is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * statusnotifier. If not, see http://www.gnu.org/licenses/
 */

#ifndef __SN_H__
#define __SN_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(StatusNotifierItem, sn_item, SN, ITEM, GObject)

#define SN_ERROR g_quark_from_static_string("StatusNotifier error")
typedef enum
{
	SN_ERROR_NO_CONNECTION = 0,
	SN_ERROR_NO_NAME,
	SN_ERROR_NO_WATCHER,
	SN_ERROR_NO_HOST
} StatusNotifierError;

typedef enum
{
	SN_STATE_NOT_REGISTERED = 0,
	SN_STATE_REGISTERING,
	SN_STATE_REGISTERED,
	SN_STATE_FAILED
} StatusNotifierState;

typedef enum
{
	SN_ICON = 0,
	SN_ATTENTION_ICON,
	SN_OVERLAY_ICON,
	SN_TOOLTIP_ICON,
    SN_ICONS_NUM /*< skip >*/
} StatusNotifierIcon;

typedef enum
{
	SN_CATEGORY_APPLICATION_STATUS = 0,
	SN_CATEGORY_COMMUNICATIONS,
	SN_CATEGORY_SYSTEM_SERVICES,
	SN_CATEGORY_HARDWARE,
	SN_CATEGORY_OTHER
} StatusNotifierCategory;

typedef enum
{
	SN_STATUS_PASSIVE = 0,
	SN_STATUS_ACTIVE,
	SN_STATUS_NEEDS_ATTENTION
} StatusNotifierStatus;

typedef enum
{
	SN_SCROLL_ORIENTATION_HORIZONTAL = 0,
	SN_SCROLL_ORIENTATION_VERTICAL
} StatusNotifierScrollOrientation;

struct _StatusNotifierItemClass
{
	GObjectClass parent_class;

	/* signals */
	void (*registration_failed)(StatusNotifierItem *sn, GError *error);

	bool (*context_menu)(StatusNotifierItem *sn, gint x, gint y);
	bool (*activate)(StatusNotifierItem *sn, gint x, gint y);
	bool (*secondary_activate)(StatusNotifierItem *sn, gint x, gint y);
	bool (*scroll)(StatusNotifierItem *sn, gint delta,
	               StatusNotifierScrollOrientation orientation);
};

StatusNotifierItem *status_notifier_item_new_from_pixbuf(const char *id,
                                                         StatusNotifierCategory category,
                                                         GdkPixbuf *pixbuf);
StatusNotifierItem *status_notifier_item_new_from_icon_name(const char *id,
                                                            StatusNotifierCategory category,
                                                            const char *icon_name);
const char *status_notifier_item_get_id(StatusNotifierItem *sn);
StatusNotifierCategory status_notifier_item_get_category(StatusNotifierItem *sn);
void status_notifier_item_set_from_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon,
                                          GdkPixbuf *pixbuf);
void status_notifier_item_set_from_icon_name(StatusNotifierItem *sn, StatusNotifierIcon icon,
                                             const char *icon_name);
bool status_notifier_item_has_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon);
GdkPixbuf *status_notifier_item_get_pixbuf(StatusNotifierItem *sn, StatusNotifierIcon icon);
char *status_notifier_item_get_icon_name(StatusNotifierItem *sn, StatusNotifierIcon icon);
void status_notifier_item_set_attention_movie_name(StatusNotifierItem *sn, const char *movie_name);
char *status_notifier_item_get_attention_movie_name(StatusNotifierItem *sn);
void status_notifier_item_set_title(StatusNotifierItem *sn, const char *title);
char *status_notifier_item_get_title(StatusNotifierItem *sn);
void status_notifier_item_set_status(StatusNotifierItem *sn, StatusNotifierStatus status);
StatusNotifierStatus status_notifier_item_get_status(StatusNotifierItem *sn);
void status_notifier_item_set_window_id(StatusNotifierItem *sn, guint32 window_id);
guint32 status_notifier_item_get_window_id(StatusNotifierItem *sn);
void status_notifier_item_freeze_tooltip(StatusNotifierItem *sn);
void status_notifier_item_thaw_tooltip(StatusNotifierItem *sn);
void status_notifier_item_set_tooltip(StatusNotifierItem *sn, const char *icon_name,
                                      const char *title, const char *body);
void status_notifier_item_set_tooltip_title(StatusNotifierItem *sn, const char *title);
char *status_notifier_item_get_tooltip_title(StatusNotifierItem *sn);
void status_notifier_item_set_tooltip_body(StatusNotifierItem *sn, const char *body);
char *status_notifier_item_get_tooltip_body(StatusNotifierItem *sn);
void status_notifier_item_register(StatusNotifierItem *sn);
StatusNotifierState status_notifier_item_get_state(StatusNotifierItem *sn);
void status_notifier_item_set_item_is_menu(StatusNotifierItem *sn, bool is_menu);
bool status_notifier_item_get_item_is_menu(StatusNotifierItem *sn);
bool status_notifier_item_set_context_menu(StatusNotifierItem *sn, GObject *menu);
GObject *status_notifier_item_get_context_menu(StatusNotifierItem *sn);

G_END_DECLS

#endif /* __SN_H__ */
