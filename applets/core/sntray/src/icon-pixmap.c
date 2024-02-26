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
#include "rtparser.h"
#include "sn-common.h"
#include <gtk/gtk.h>
#include <stdbool.h>

G_GNUC_INTERNAL IconPixmap *icon_pixmap_new_with_size(GVariant *pixmaps, int icon_size) // Format:
                                                                                        // a@(ii@ay)
{
	IconPixmap *self = g_new0(IconPixmap, 1);
	if (!pixmaps)
		return self;
	GVariantIter pixmap_iter;
	g_variant_iter_init(&pixmap_iter, pixmaps);
	size_t i = 0, max = g_variant_n_children(pixmaps);
	g_autoptr(GVariant) pixmap_variant = NULL;
	while ((pixmap_variant = g_variant_iter_next_value(&pixmap_iter)))
	{
		int width;
		int height;
		g_variant_get(pixmap_variant, "(ii@ay)", &width, &height, NULL);
		if ((height >= icon_size && width >= icon_size) || i >= max - 1)
		{
			self->height = height;
			self->width  = width;
			g_autoptr(GVariant) bytes_var =
			    g_variant_get_child_value(pixmap_variant, 2);
			self->bytes    = (u_int8_t *)g_variant_get_fixed_array(bytes_var,
                                                                            &self->bytes_size,
                                                                            sizeof(u_int8_t));
			self->bytes    = g_memdup(self->bytes, self->bytes_size);
			self->is_gicon = false;
			break;
		}
		g_clear_pointer(&pixmap_variant, g_variant_unref);
		i++;
	}
	return self;
}

G_GNUC_INTERNAL void icon_pixmap_free(IconPixmap *self)
{
	if (!self->is_gicon)
		g_clear_pointer(&self->bytes, g_free);
	g_clear_pointer(&self, g_free);
}

G_GNUC_INTERNAL GIcon *icon_pixmap_to_gicon(IconPixmap *self)
{
	if (!self->bytes)
		return NULL;
	{
		u_int32_t *data = (u_int32_t *)self->bytes;
		for (size_t i = 0; i < self->bytes_size / 4; i++)
		{
			data[i]    = GUINT32_FROM_BE(data[i]);
			u_int8_t a = (data[i] >> 24) & 0xFF;
			u_int8_t b = (data[i] >> 16) & 0xFF;
			u_int8_t g = (data[i] >> 8) & 0xFF;
			u_int8_t r = data[i] & 0xFF;
			data[i]    = a << 24 | r << 16 | g << 8 | b;
		}
	}
	GIcon *icon =
	    G_ICON(gdk_pixbuf_new_from_data(self->bytes,
	                                    GDK_COLORSPACE_RGB,
	                                    true,
	                                    8,
	                                    self->width,
	                                    self->height,
	                                    cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
	                                                                  self->width),
	                                    g_free,
	                                    NULL));
	self->is_gicon = true;
	self->bytes    = NULL;
	return icon;
}

static GIcon *icon_pixmap_find_file_icon(const char *icon_name, const char *path)
{
	if (string_empty(path))
		return NULL;
	g_autoptr(GError) err = NULL;
	g_autoptr(GDir) dir   = g_dir_open(path, 0, &err);
	if (err)
		return NULL;
	for (const char *ch = g_dir_read_name(dir); ch != NULL; ch = g_dir_read_name(dir))
	{
		g_autofree char *new_path = g_build_filename(path, ch, NULL);
		g_autoptr(GFile) f        = g_file_new_for_path(new_path);
		char *sstr                = g_strrstr(ch, ".");
		long diff                 = -1;
		if (sstr != NULL)
			diff = sstr - ch;
		long index            = labs(diff);
		g_autofree char *icon = index >= 0 ? g_strndup(ch, (ulong)index) : g_strdup(ch);
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

GIcon *icon_pixmap_select_icon(const char *icon_name, IconPixmap *pixmap, GtkIconTheme *theme,
                               const char *icon_theme_path, const int icon_size,
                               const bool use_symbolic)
{
	if (!string_empty(icon_name))
	{
		g_autofree char *new_name = (use_symbolic && !g_strrstr(icon_name, "-symbolic"))
		                                ? g_strdup_printf("%s-symbolic", icon_name)
		                                : g_strdup(icon_name);
		if (icon_name[0] == '/')
		{
			g_autoptr(GFile) f = g_file_new_for_path(icon_name);
			return g_file_icon_new(f);
		}
		else if (string_empty(icon_theme_path) || gtk_icon_theme_has_icon(theme, new_name))
			return g_themed_icon_new_with_default_fallbacks(new_name);
		else
			return icon_pixmap_find_file_icon(icon_name, icon_theme_path);
	}
	else if (pixmap != NULL)
	{
		GdkPixbuf *pixbuf = GDK_PIXBUF(icon_pixmap_to_gicon(pixmap));
		if (pixbuf && (gdk_pixbuf_get_width(pixbuf) > icon_size ||
		               gdk_pixbuf_get_height(pixbuf) > icon_size))
		{
			GdkPixbuf *tmp = gdk_pixbuf_scale_simple(pixbuf,
			                                         icon_size,
			                                         icon_size,
			                                         GDK_INTERP_BILINEAR);
			g_clear_object(&pixbuf);
			pixbuf = tmp;
		}
		return G_ICON(pixbuf);
	}
	return NULL;
}
G_DEFINE_AUTOPTR_CLEANUP_FUNC(IconPixmap, icon_pixmap_free)

G_GNUC_INTERNAL ToolTip *tooltip_new(GVariant *variant)
{
	ToolTip *self = g_new0(ToolTip, 1);
	if (!variant)
		return self;
	const char *type_string = g_variant_get_type_string(variant);
	bool is_full            = !g_strcmp0(type_string, "(sa(iiay)ss)");
	if (is_full)
	{
		g_variant_get_child(variant, 0, "s", &self->icon_name, NULL);
		g_autoptr(GVariant) ch = g_variant_get_child_value(variant, 1);
		self->pixmap           = icon_pixmap_new_with_size(ch, GTK_ICON_SIZE_DIALOG);
		g_variant_get_child(variant, 2, "s", &self->title, NULL);
		g_variant_get_child(variant, 3, "s", &self->description, NULL);
	}
	else if (!g_strcmp0(g_variant_get_type_string(variant), "s"))
		self->title = g_variant_dup_string(variant, NULL);
	return self;
}

G_GNUC_INTERNAL void unbox_tooltip(ToolTip *tooltip, GtkIconTheme *theme,
                                   const char *icon_theme_path, GIcon **icon, char **markup)
{
	g_autofree char *raw_text = g_strdup_printf("%s\n%s", tooltip->title, tooltip->description);
	bool is_pango_markup      = true;
	g_autoptr(GString) bldr   = g_string_new("");
	if (raw_text)
	{
		g_autoptr(GError) err = NULL;
		pango_parse_markup(raw_text, -1, '\0', NULL, NULL, NULL, &err);
		if (err)
			is_pango_markup = false;
	}
	if (!is_pango_markup)
	{
		g_string_append(bldr, "<markup>");
		if (!string_empty(tooltip->title))
			g_string_append(bldr, tooltip->title);
		if (!string_empty(tooltip->description))
		{
			if (bldr->len > 8)
				g_string_append(bldr, "<br/>");
			g_string_append(bldr, tooltip->description);
		}
		g_string_append(bldr, "</markup>");
		g_autoptr(QRichTextParser) markup_parser = qrich_text_parser_new(bldr->str);
		qrich_text_parser_translate_markup(markup_parser);
		*markup = (!string_empty(markup_parser->pango_markup))
		              ? g_strdup(markup_parser->pango_markup)
		              : g_strdup(raw_text);
		g_autoptr(GIcon) res_icon = icon_pixmap_select_icon(tooltip->icon_name,
		                                                    tooltip->pixmap,
		                                                    theme,
		                                                    icon_theme_path,
		                                                    GTK_ICON_SIZE_DIALOG,
		                                                    false);
		*icon = (markup_parser->icon != NULL) ? g_object_ref(markup_parser->icon)
		                                      : g_object_ref(res_icon);
	}
	else
	{
		*markup = g_strdup(raw_text);
		*icon   = icon_pixmap_select_icon(tooltip->icon_name,
                                                tooltip->pixmap,
                                                theme,
                                                icon_theme_path,
                                                48,
                                                false);
	}
}
G_GNUC_INTERNAL void tooltip_free(ToolTip *self)
{
	g_clear_pointer(&self->title, g_free);
	g_clear_pointer(&self->description, g_free);
	g_clear_pointer(&self->icon_name, g_free);
	g_clear_pointer(&self->pixmap, icon_pixmap_free);
	g_clear_pointer(&self, g_free);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(ToolTip, tooltip_free)
