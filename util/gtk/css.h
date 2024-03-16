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

void vala_panel_style_set_class(GtkWidget *widget, const char *css, const char *klass, bool remove);
void vala_panel_style_from_res(GtkWidget *widget, const char *file, const char *klass);
void vala_panel_style_class_toggle(GtkWidget *w, const char *klass, bool apply);
GtkCssProvider *vala_panel_style_from_file(const char *file, unsigned int priority);
void vala_panel_style_set_for_widget(GtkWidget *w, const char *css);
char *vala_panel_style_flat_button(GtkWidget *widget, GtkPositionType direction);

#endif
