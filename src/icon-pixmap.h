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

#ifndef ICONPIXMAP_H
#define ICONPIXMAP_H

#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

typedef struct
{
	int width;
	int height;
	u_int8_t *bytes;
	size_t bytes_size;
} IconPixmap;

G_GNUC_INTERNAL IconPixmap *icon_pixmap_new(GVariant *pixmap_variant);
G_GNUC_INTERNAL IconPixmap *icon_pixmap_copy(IconPixmap *src);
G_GNUC_INTERNAL IconPixmap **unbox_pixmaps(const GVariant *variant);
G_GNUC_INTERNAL void icon_pixmap_free(IconPixmap *self);
G_GNUC_INTERNAL void icon_pixmap_freev(IconPixmap **pixmaps);
G_GNUC_INTERNAL GIcon *icon_pixmap_gicon(const IconPixmap *self);
G_GNUC_INTERNAL GIcon *icon_pixmap_select_icon(const char *icon_name, const IconPixmap **pixmaps,
                                               const GtkIconTheme *theme,
                                               const char *icon_theme_path, const int icon_size,
                                               const bool use_symbolic);
typedef struct
{
	char *icon_name;
	IconPixmap **pixmaps;
	char *title;
	char *description;
} ToolTip;

G_GNUC_INTERNAL ToolTip *tooltip_new(GVariant *variant);
G_GNUC_INTERNAL ToolTip *tooltip_copy(ToolTip *src);
G_GNUC_INTERNAL bool tooltip_equal(const void *src, const void *dst);
G_GNUC_INTERNAL void unbox_tooltip(ToolTip *tooltip, const GtkIconTheme *theme,
                                   const char *icon_theme_path, GIcon **icon, char **markup);
G_GNUC_INTERNAL void tooltip_free(ToolTip *self);
typedef enum
{
	SN_CATEGORY_APPLICATION,
	SN_CATEGORY_COMMUNICATIONS,
	SN_CATEGORY_SYSTEM,
	SN_CATEGORY_HARDWARE,
	SN_CATEGORY_OTHER
} SnCategory;
G_GNUC_INTERNAL GType sn_category_get_type(void);
#define SN_TYPE_CATEGORY sn_category_get_type()
G_GNUC_INTERNAL const char *sn_category_get_nick(SnCategory value);
G_GNUC_INTERNAL SnCategory sn_category_get_value_from_nick(const char *nick);

typedef enum
{
	SN_STATUS_PASSIVE,
	SN_STATUS_ACTIVE,
	SN_STATUS_ATTENTION
} SnStatus;
G_GNUC_INTERNAL GType sn_status_get_type(void);
#define SN_TYPE_STATUS sn_status_get_type()
G_GNUC_INTERNAL const char *sn_status_get_nick(SnStatus value);
G_GNUC_INTERNAL SnStatus sn_status_get_value_from_nick(const char *nick);

static inline bool string_empty(const char *path)
{
	return (path == NULL || strlen(path) == 0);
}

G_END_DECLS

#endif // ICONPIXMAP_H
