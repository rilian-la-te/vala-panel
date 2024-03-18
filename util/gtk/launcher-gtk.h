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

#ifndef LAUNCHER_GTK_H
#define LAUNCHER_GTK_H

#include <gdk/gdk.h>
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>
#include <stdbool.h>

bool vala_panel_launch(GDesktopAppInfo *app_info, GList *uris, GtkWidget *parent);
void vala_panel_activate_launch_id(GSimpleAction *action, GVariant *param, gpointer user_data);
void vala_panel_activate_launch_uri(GSimpleAction *action, GVariant *param, gpointer user_data);
void vala_panel_activate_launch_command(GSimpleAction *action, GVariant *param, gpointer user_data);

#endif // LAUNCHER_H
