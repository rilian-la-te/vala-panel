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

void vala_panel_setup_label(GtkLabel *label, const char *text, bool bold, double factor);
/**
 * vala_panel_setup_button:
 * @b: (not nullable): a #GtkButton
 * @img: (nullable): a #GtkImage to be added to button
 * @label: (nullable): a text to be added to button
 *
 */
void vala_panel_setup_button(GtkButton *b, GtkImage *img, const char *label);
/**
 * vala_panel_setup_icon:
 * @img: (not nullable): a #GtkImage to setup
 * @icon: (not nullable): a #Gicon for setup
 * @top: (nullable): a #ValaPanelToplevel to bind icon size to
 * @size: an explicitly provided icon size
 *
 */
void vala_panel_setup_icon(GtkImage *img, GIcon *icon, GObject *top, int size);
void vala_panel_setup_icon_button(GtkButton *btn, GIcon *icon, const char *label, GObject *top);
void vala_panel_scale_button_set_range(GtkScaleButton *b, gint lower, gint upper);
void vala_panel_scale_button_set_value_labeled(GtkScaleButton *b, gint value);
void vala_panel_apply_window_icon(GtkWindow *win);
int vala_panel_monitor_num_from_mon(GdkDisplay *disp, GdkMonitor *mon);
void vala_panel_generate_error_dialog(GtkWindow *parent, const char *error);
bool vala_panel_generate_confirmation_dialog(GtkWindow *parent, const char *error);
G_END_DECLS

#endif // MISC_H
