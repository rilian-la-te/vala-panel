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

#ifndef __VALA_PANEL_CSS_PRIVATE_H__
#define __VALA_PANEL_CSS_PRIVATE_H__

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "css.h"

G_GNUC_INTERNAL char *css_generate_background(const char *filename, GdkRGBA *color);
G_GNUC_INTERNAL char *css_generate_font_color(GdkRGBA color);
G_GNUC_INTERNAL char *css_generate_font_size(gint size);
G_GNUC_INTERNAL char *css_generate_font_label(double size, bool is_bold);
G_GNUC_INTERNAL GtkCssProvider *css_add_css_with_provider(GtkWidget *widget, const char *css);

#endif
