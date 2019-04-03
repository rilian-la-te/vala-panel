/*
 * xfce4-sntray-plugin
 * Copyright (C) 2015-2019 Konstantin Pugin <ria.freelander@gmail.com>
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

#ifndef SN_COMMON_H
#define SN_COMMON_H

#include <gio/gio.h>
#include <stdbool.h>

G_BEGIN_DECLS

typedef enum
{
	SN_CATEGORY_APPLICATION,    /*< nick=ApplicationStatus >*/
	SN_CATEGORY_COMMUNICATIONS, /*< nick=Communications >*/
	SN_CATEGORY_SYSTEM,         /*< nick=SystemServices >*/
	SN_CATEGORY_HARDWARE,       /*< nick=Hardware >*/
	SN_CATEGORY_OTHER           /*< nick=Other >*/
} SnCategory;

typedef enum
{
	SN_STATUS_PASSIVE,  /*< nick=Passive >*/
	SN_STATUS_ACTIVE,   /*< nick=Active >*/
	SN_STATUS_ATTENTION /*< nick=NeedsAttention >*/
} SnStatus;

static inline bool string_empty(const char *path)
{
	return (path == NULL || strlen(path) == 0);
}

G_END_DECLS

#endif
