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
#include <xcb/xcb_ewmh.h>

#include "sn-common.h"

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
	SN_ICON = 0,
	SN_TOOLTIP_ICON,
	SN_ICONS_NUM /*< skip >*/
} StatusNotifierIcon;

struct _StatusNotifierItemClass
{
	GObjectClass parent_class;

	/* signals */
	void (*registration_failed)(StatusNotifierItem *sn, GError *error);
};

StatusNotifierItem *status_notifier_item_new_from_xcb_window(const char *id, SnCategory category,
                                                             xcb_connection_t *conn,
                                                             xcb_window_t window);

G_END_DECLS

#endif /* __SN_H__ */
