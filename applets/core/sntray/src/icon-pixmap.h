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
	size_t bytes_size;
	bool is_gicon;
	u_int8_t *bytes;
} IconPixmap;

G_GNUC_INTERNAL IconPixmap *icon_pixmap_new_with_size(GVariant *pixmaps, int icon_size);
G_GNUC_INTERNAL void icon_pixmap_free(IconPixmap *self);
G_GNUC_INTERNAL GIcon *icon_pixmap_to_gicon(IconPixmap *self);
G_GNUC_INTERNAL GIcon *icon_pixmap_select_icon(const char *icon_name, IconPixmap *pixmap,
                                               GtkIconTheme *theme, const char *icon_theme_path,
                                               const int icon_size, const bool use_symbolic);
typedef struct
{
	char *icon_name;
	IconPixmap *pixmap;
	char *title;
	char *description;
} ToolTip;

G_GNUC_INTERNAL ToolTip *tooltip_new(GVariant *variant);
G_GNUC_INTERNAL void unbox_tooltip(ToolTip *tooltip, GtkIconTheme *theme,
                                   const char *icon_theme_path, GIcon **icon, char **markup);
G_GNUC_INTERNAL void tooltip_free(ToolTip *self);
G_END_DECLS

#endif // ICONPIXMAP_H
