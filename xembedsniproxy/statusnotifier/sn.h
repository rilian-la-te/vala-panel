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
#include <xcb/xcb.h>

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

StatusNotifierItem *status_notifier_item_new_from_xcb_widnow(const char *id,
                                                             StatusNotifierCategory category,
                                                             xcb_connection_t *conn,
                                                             xcb_window_t window);

G_END_DECLS

#endif /* __SN_H__ */
