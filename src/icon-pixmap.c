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
#include <gtk/gtk.h>
#include <stdbool.h>

G_GNUC_INTERNAL IconPixmap *icon_pixmap_new(GVariant *pixmap_variant)
{
	IconPixmap *self = g_new0(IconPixmap, 1);
	if (!pixmap_variant)
		return self;
	g_variant_get_child(pixmap_variant, 0, "i", self->width);
	g_variant_get_child(pixmap_variant, 1, "i", self->height);
	g_autoptr(GVariant) bytes_variant = g_variant_get_child_value(pixmap_variant, 2);
	self->bytes_size                  = g_variant_n_children(bytes_variant);
	const u_int8_t *bytes =
	    g_variant_get_fixed_array(bytes_variant, &self->bytes_size, sizeof(u_int8_t));
	self->bytes = g_memdup(bytes, self->bytes_size);
	return self;
}

G_GNUC_INTERNAL IconPixmap *icon_pixmap_copy(IconPixmap *src)
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

G_GNUC_INTERNAL IconPixmap **unbox_pixmaps(const GVariant *variant)
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
	pixmaps[size] = NULL;
	return pixmaps;
}

G_GNUC_INTERNAL void icon_pixmap_freev(IconPixmap **pixmaps)
{
	for (size_t i = 0; pixmaps[i] != NULL; i++)
		g_clear_pointer(&pixmaps[i], icon_pixmap_free);
	g_clear_pointer(&pixmaps, g_free);
}

G_GNUC_INTERNAL GIcon *icon_pixmap_gicon(const IconPixmap *self)
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

static bool icon_pixmap_equal(IconPixmap *src, IconPixmap *dst)
{
	if (!src && !dst)
		return true;
	if (!src || !dst)
		return false;
	if (src->height != dst->height)
		return false;
	if (src->width != dst->width)
		return false;
	if (src->bytes_size != dst->bytes_size)
		return false;
	for (size_t i = 0; i < src->bytes_size; i++)
		if (src->bytes[i] != dst->bytes[i])
			return false;
	return true;
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

GIcon *icon_pixmap_select_icon(const char *icon_name, const IconPixmap **pixmaps,
                               const GtkIconTheme *theme, const char *icon_theme_path,
                               const int icon_size, const bool use_symbolic)
{
	if (!string_empty(icon_name))
	{
		g_autofree char *new_name = (use_symbolic)
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
	else if (pixmaps != NULL && pixmaps[0] != NULL)
	{
		GdkPixbuf *pixbuf = NULL;
		size_t i;
		for (i = 0; pixmaps[i] != NULL; i++)
		{
			if (pixmaps[i]->height >= icon_size && pixmaps[i]->width >= icon_size)
				break;
		}
		pixbuf = GDK_PIXBUF(icon_pixmap_gicon(pixmaps[i]));
		if (gdk_pixbuf_get_width(pixbuf) > icon_size ||
		    gdk_pixbuf_get_height(pixbuf) > icon_size)
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

G_GNUC_INTERNAL ToolTip *tooltip_new(GVariant *variant)
{
	ToolTip *self = g_new0(ToolTip, 1);
	if (!variant)
		return self;
	g_variant_get_child(variant, 0, "s", self->icon_name);
	g_autoptr(GVariant) ch = g_variant_get_child_value(variant, 1);
	self->pixmaps          = unbox_pixmaps(ch);
	self->icon             = NULL;
	g_variant_get_child(variant, 2, "s", self->title);
	g_variant_get_child(variant, 3, "s", self->description);
	return self;
}

G_GNUC_INTERNAL ToolTip *tooltip_copy(ToolTip *src)
{
	ToolTip *dst     = g_new0(ToolTip, 1);
	dst->icon_name   = g_strdup(src->icon_name);
	dst->title       = g_strdup(src->title);
	dst->description = g_strdup(src->description);
	dst->pixmaps     = src->pixmaps;
	dst->icon        = g_object_ref(src->icon);
	return dst;
}
G_GNUC_INTERNAL void unbox_tooltip(ToolTip *tooltip, const GtkIconTheme *theme,
                                   const char *icon_theme_path, GIcon **icon, char **markup)
{
	g_autofree char *raw_text = g_strdup_printf("%s\n%s", tooltip->title, tooltip->description);
	bool is_pango_markup      = true;
	g_autoptr(GString) bldr   = g_string_new("");
	if (raw_text)
	{
		GError *err = NULL;
		pango_parse_markup(raw_text, -1, '\0', NULL, NULL, NULL, &err);
		if (err)
			is_pango_markup = false;
	}
	if (!is_pango_markup)
	{
		g_string_append(bldr, "<markup>");
		if (!string_empty(tooltip->title))
			g_string_append(bldr, tooltip->title);
		if (!string_empty(tooltip->description) > 0)
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
		                                                    tooltip->pixmaps,
		                                                    theme,
		                                                    icon_theme_path,
		                                                    48,
		                                                    false);
		*icon = (markup_parser->icon != NULL) ? g_object_ref(markup_parser->icon)
		                                      : g_object_ref(res_icon);
	}
	else
	{
		*markup = g_strdup(raw_text);
		*icon   = icon_pixmap_select_icon(tooltip->icon_name,
                                                tooltip->pixmaps,
                                                theme,
                                                icon_theme_path,
                                                48,
                                                false);
	}
}
G_GNUC_INTERNAL bool tooltip_equal(const void *src, const void *dst)
{
	const ToolTip *t1 = (const ToolTip *)src;
	const ToolTip *t2 = (const ToolTip *)dst;
	if (!src && !dst)
		return true;
	if (!src || !dst)
		return false;
	int i = 0;
	if (g_strcmp0(t1->icon_name, t2->icon_name))
		return false;
	if (g_strcmp0(t1->title, t2->title))
		return false;
	if (g_strcmp0(t1->description, t2->description))
		return false;
	if (!g_icon_equal(t1->icon, t2->icon))
		return false;
	for (i = 0; icon_pixmap_equal(t1->pixmaps[i], t2->pixmaps[i]) && t2->pixmaps[i] != NULL;
	     i++)
		;
	if (t1->pixmaps[i] != NULL || t2->pixmaps[i] != NULL)
		return false;
	return true;
}

G_GNUC_INTERNAL void tooltip_free(ToolTip *self)
{
	g_clear_pointer(&self->title, g_free);
	g_clear_pointer(&self->description, g_free);
	g_clear_object(&self->icon);
	g_clear_pointer(&self->icon_name, g_free);
	g_clear_pointer(&self->pixmaps, icon_pixmap_freev);
	g_clear_pointer(&self, g_free);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(ToolTip, tooltip_free)
/*
 * This part is required, because we use non-standard nicks.
 */

G_GNUC_INTERNAL GType sn_category_get_type(void)
{
	static GType the_type = 0;

	if (the_type == 0)
	{
		static const GEnumValue values[] = {
			{ SN_CATEGORY_APPLICATION, "SN_CATEGORY_APPLICATION", "ApplicationStatus" },
			{ SN_CATEGORY_COMMUNICATIONS,
			  "SN_CATEGORY_COMMUNICATIONS",
			  "Communications" },
			{ SN_CATEGORY_SYSTEM, "SN_CATEGORY_SYSTEM_SERVICES", "SystemServices" },
			{ SN_CATEGORY_HARDWARE, "SN_CATEGORY_HARDWARE", "Hardware" },
			{ SN_CATEGORY_OTHER, "SN_CATEGORY_OTHER", "Other" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static(g_intern_static_string("SnCategory"), values);
	}
	return the_type;
}

G_GNUC_INTERNAL const char *sn_category_get_nick(SnCategory value)
{
	GEnumClass *class = G_ENUM_CLASS(g_type_class_ref(sn_category_get_type()));
	g_return_val_if_fail(class != NULL, NULL);

	const char *ret = NULL;
	GEnumValue *val = g_enum_get_value(class, value);
	if (val != NULL)
		ret = val->value_nick;

	g_type_class_unref(class);
	return ret;
}

G_GNUC_INTERNAL SnCategory sn_category_get_value_from_nick(const char *nick)
{
	GEnumClass *class = G_ENUM_CLASS(g_type_class_ref(sn_category_get_type()));
	g_return_val_if_fail(class != NULL, 0);

	SnCategory ret  = 0;
	GEnumValue *val = g_enum_get_value_by_nick(class, nick);
	if (val != NULL)
		ret = val->value;

	g_type_class_unref(class);
	return ret;
}
G_GNUC_INTERNAL GType sn_status_get_type(void)
{
	static GType the_type = 0;

	if (the_type == 0)
	{
		static const GEnumValue values[] = {
			{ SN_STATUS_PASSIVE, "SN_STATUS_PASSIVE", "Passive" },
			{ SN_STATUS_ACTIVE, "SN_STATUS_ACTIVE", "Active" },
			{ SN_STATUS_ATTENTION, "SN_STATUS_NEEDS_ATTENTION", "NeedsAttention" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static(g_intern_static_string("SnStatus"), values);
	}
	return the_type;
}

G_GNUC_INTERNAL const char *sn_status_get_nick(SnStatus value)
{
	GEnumClass *class = G_ENUM_CLASS(g_type_class_ref(sn_status_get_type()));
	g_return_val_if_fail(class != NULL, NULL);

	const char *ret = NULL;
	GEnumValue *val = g_enum_get_value(class, value);
	if (val != NULL)
		ret = val->value_nick;

	g_type_class_unref(class);
	return ret;
}

G_GNUC_INTERNAL SnStatus sn_status_get_value_from_nick(const char *nick)
{
	GEnumClass *class = G_ENUM_CLASS(g_type_class_ref(sn_status_get_type()));
	g_return_val_if_fail(class != NULL, 0);

	SnStatus ret    = 0;
	GEnumValue *val = g_enum_get_value_by_nick(class, nick);
	if (val != NULL)
		ret = val->value;

	g_type_class_unref(class);
	return ret;
}
