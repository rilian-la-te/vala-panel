/*
 * vala-panel
 * Copyright (C) 2015-2016 Konstantin Pugin <ria.freelander@gmail.com>
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

#ifndef __VALA_PANEL_CSS_H__
#define __VALA_PANEL_CSS_H__

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

void css_apply_with_class(GtkWidget *widget, const char *css, const char *klass, bool remove);
char *css_generate_background(const char *filename, GdkRGBA *color);
char *css_generate_font_color(GdkRGBA color);
char *css_generate_font_size(gint size);
char *css_generate_font_label(gfloat size, bool is_bold);
char *css_apply_from_file(GtkWidget *widget, const char *file);
char *css_apply_from_resource(GtkWidget *widget, const char *file, const char *klass);
char *css_apply_from_file_to_app(const char *file);
void css_toggle_class(GtkWidget *w, const char *klass, bool apply);
GtkCssProvider *css_apply_from_file_to_app_with_provider(const char *file);
GtkCssProvider *css_add_css_to_widget(GtkWidget *w, const char *css);
char *css_generate_flat_button(GtkWidget *widget, GtkPositionType direction);

#endif
