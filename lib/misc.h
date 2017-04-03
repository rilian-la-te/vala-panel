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

#ifndef MISC_H
#define MISC_H

#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

void vala_panel_setup_label(GtkLabel *label, const char *text, bool bold, double factor);
void vala_panel_setup_button(GtkButton *b, GtkImage *img, const char *label);
void vala_panel_scale_button_set_range(GtkScaleButton *b, gint lower, gint upper);
void vala_panel_scale_button_set_value_labeled(GtkScaleButton *b, gint value);
void vala_panel_add_prop_as_action(GActionMap *map, const char *prop);
void vala_panel_add_gsettings_as_action(GActionMap *map, GSettings *settings, const char *prop);
void vala_panel_apply_window_icon(GtkWindow *win);
void vala_panel_reset_schema(GSettings *settings);
void vala_panel_reset_schema_with_children(GSettings *settings);
void vala_panel_reset_path(GSettings *settings);
char *vala_panel_generate_new_hash();
G_END_DECLS

#endif // MISC_H
