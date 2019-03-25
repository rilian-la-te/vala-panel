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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>

typedef struct
{
	int width;
	int height;
	u_int8_t *bytes;
	size_t bytes_size;
} IconPixmap;

G_GNUC_INTERNAL IconPixmap *icon_pixmap_new(GVariant *pixmap_variant);
G_GNUC_INTERNAL IconPixmap **unbox_pixmaps(GVariant *variant);
G_GNUC_INTERNAL void icon_pixmap_free(IconPixmap *self);
G_GNUC_INTERNAL void icon_pixmap_freev(IconPixmap **pixmaps);
G_GNUC_INTERNAL GIcon *icon_pixmap_gicon(IconPixmap *self);

typedef struct
{
	char *icon_name;
	IconPixmap **pixmaps;
	char *title;
	char *description;
} ToolTip;

#endif // ICONPIXMAP_H
