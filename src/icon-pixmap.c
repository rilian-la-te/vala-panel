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

#include "icon-pixmap.h"
#include <cairo.h>
#include <stdbool.h>

G_GNUC_INTERNAL IconPixmap *icon_pixmap_new(GVariant *pixmap_variant)
{
	IconPixmap *self = g_new0(IconPixmap, 1);
	g_variant_get_child(pixmap_variant, 0, "i", self->width);
	g_variant_get_child(pixmap_variant, 1, "i", self->height);
	g_autoptr(GVariant) bytes_variant = g_variant_get_child_value(pixmap_variant, 2);
	self->bytes_size                  = g_variant_n_children(bytes_variant);
	const u_int8_t *bytes =
	    g_variant_get_fixed_array(bytes_variant, &self->bytes_size, sizeof(u_int8_t));
	self->bytes = g_memdup(bytes, self->bytes_size);
	return self;
}

G_GNUC_INTERNAL static IconPixmap *icon_pixmap_copy(IconPixmap *src)
{
	IconPixmap *dst = g_new0(IconPixmap, 1);
	dst->width      = src->width;
	dst->height     = src->height;
	dst->bytes_size = src->bytes_size;
	dst->bytes      = g_memdup(src->bytes, src->bytes_size);
	return dst;
}

G_GNUC_INTERNAL void icon_pixmap_free(IconPixmap *self)
{
	g_clear_pointer(&self->bytes, g_free);
	g_clear_pointer(&self, g_free);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(IconPixmap, icon_pixmap_free)
G_DEFINE_BOXED_TYPE(IconPixmap, icon_pixmap, icon_pixmap_copy, icon_pixmap_free)

G_GNUC_INTERNAL IconPixmap **unbox_pixmaps(GVariant *variant)
{
	size_t i             = 0;
	size_t size          = g_variant_n_children(variant);
	IconPixmap **pixmaps = g_new0(IconPixmap *, size + 1);
	GVariantIter pixmap_iter;
	g_variant_iter_init(&pixmap_iter, variant);
	GVariant *pixmap_variant;
	while ((pixmap_variant = g_variant_iter_next_value(&pixmap_iter)))
	{
		pixmaps[i] = icon_pixmap_new(pixmap_variant);
		i++;
	}
	return pixmaps;
}

G_GNUC_INTERNAL void icon_pixmap_freev(IconPixmap **pixmaps)
{
	for (size_t i = 0; pixmaps[i] != NULL; i++)
		g_clear_pointer(&pixmaps[i], icon_pixmap_free);
	g_clear_pointer(&pixmaps, g_free);
}

G_GNUC_INTERNAL GIcon *icon_pixmap_gicon(IconPixmap *self)
{
	if (!self->bytes)
		return NULL;
	for (size_t i = 0; i < self->bytes_size; i += 4)
	{
		u_int8_t alpha     = self->bytes[i];
		self->bytes[i]     = self->bytes[i + 1];
		self->bytes[i + 1] = self->bytes[i + 2];
		self->bytes[i + 2] = self->bytes[i + 3];
		self->bytes[i + 3] = alpha;
	}
	u_int8_t *pixbytes = g_memdup(self->bytes, self->bytes_size);
	return G_ICON(gdk_pixbuf_new_from_data(pixbytes,
	                                       GDK_COLORSPACE_RGB,
	                                       true,
	                                       8,
	                                       self->width,
	                                       self->height,
	                                       cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
	                                                                     self->width),
	                                       g_free,
	                                       NULL));
}

ToolTip *tooltip_new(GVariant *variant)
{
	ToolTip *self = g_new0(ToolTip, 1);
	g_variant_get_child(variant, 0, "s", self->icon_name);
	g_autoptr(GVariant) ch = g_variant_get_child_value(variant, 1);
	self->pixmaps          = unbox_pixmaps(ch);
	g_variant_get_child(variant, 2, "s", self->title);
	g_variant_get_child(variant, 3, "s", self->description);
	return self;
}

G_GNUC_INTERNAL static ToolTip *tooltip_copy(ToolTip *src)
{
	ToolTip *dst     = g_new0(ToolTip, 1);
	dst->icon_name   = g_strdup(src->icon_name);
	dst->title       = g_strdup(src->title);
	dst->description = g_strdup(src->description);
	dst->pixmaps     = src->pixmaps;
	return dst;
}

G_GNUC_INTERNAL void tooltip_free(ToolTip *self)
{
	g_clear_pointer(&self->icon_name, g_free);
	g_clear_pointer(&self->title, g_free);
	g_clear_pointer(&self->description, g_free);
	g_clear_pointer(&self->pixmaps, icon_pixmap_freev);
	g_clear_pointer(&self, g_free);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(ToolTip, tooltip_free)
G_DEFINE_BOXED_TYPE(ToolTip, tooltip, tooltip_copy, tooltip_free)

GIcon *icon_pixmap_find_file_icon(char *icon_name, char *path)
{
	if (path == NULL || strlen(path) == 0)
		return NULL;
	g_autoptr(GError) err = NULL;
	g_autoptr(GDir) dir   = g_dir_open(path, 0, &err);
	if (err)
		return NULL;
	for (const char *ch = g_dir_read_name(dir); ch != NULL; ch = g_dir_read_name(dir))
	{
		g_autofree char *new_path = g_build_filename(path, ch, NULL);
		g_autoptr(GFile) f        = g_file_new_for_path(new_path);
		char *sstr                = g_strrstr(new_path, ".");
		long index                = sstr != NULL ? labs(sstr - new_path) : -1;
		g_autofree char *icon =
		    index >= 0 ? g_strndup(new_path, (ulong)index) : g_strdup(new_path);
		if (!g_strcmp0(icon_name, icon))
			return g_file_icon_new(f);
		GIcon *ret  = NULL;
		GFileType t = g_file_query_file_type(f, G_FILE_QUERY_INFO_NONE, NULL);
		if (t == G_FILE_TYPE_DIRECTORY)
			ret = icon_pixmap_find_file_icon(icon_name, new_path);
		if (ret)
			return ret;
	}
	return NULL;
}
