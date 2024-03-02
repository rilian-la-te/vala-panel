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

#ifndef MISC_GTK_H
#define MISC_GTK_H

#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

void vp_setup_label(GtkLabel *label, const char *text, bool bold, double factor);
void vp_setup_button(GtkButton *b, GtkImage *img, const char *label);
void vp_setup_icon(GtkImage *img, GIcon *icon, GObject *top, int size);
void vp_setup_icon_button(GtkButton *btn, GIcon *icon, const char *label, GObject *top);
void vp_scale_button_set_range(GtkScaleButton *b, gint lower, gint upper);
void vp_scale_button_set_value_labeled(GtkScaleButton *b, gint value);
void vp_apply_window_icon(GtkWindow *win);
int vp_monitor_num_from_mon(GdkDisplay *disp, GdkMonitor *mon);
void vp_generate_error_dialog(GtkWindow *parent, const char *error);
bool vp_generate_confirmation_dialog(GtkWindow *parent, const char *error);
G_END_DECLS

#endif // MISC_H
