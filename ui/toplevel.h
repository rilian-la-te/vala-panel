/*
 * vala-panel
 * Copyright (C) 2017 Konstantin Pugin <ria.freelander@gmail.com>
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

#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "applet-widget.h"
#include "constants.h"
#include "panel-platform.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelToplevel, vala_panel_toplevel, VALA_PANEL, TOPLEVEL,
                     GtkApplicationWindow)

#define VALA_PANEL_TYPE_TOPLEVEL vala_panel_toplevel_get_type()

ValaPanelCoreSettings *vala_panel_toplevel_get_core_settings();
bool vala_panel_toplevel_is_initialized(ValaPanelToplevel *self);
void vala_panel_toplevel_update_geometry(ValaPanelToplevel *self);
G_GNUC_INTERNAL ValaPanelPlatform *vala_panel_toplevel_get_current_platform();

ValaPanelToplevel *vala_panel_toplevel_new(GtkApplication *app, ValaPanelPlatform *plt,
                                           const char *uid);

#define vala_panel_toplevel_get_layout(self)                                                       \
	VALA_PANEL_LAYOUT(gtk_bin_get_child(GTK_BIN(gtk_bin_get_child(GTK_BIN(self)))))

void start_ui(ValaPanelToplevel *self);
void stop_ui(ValaPanelToplevel *self);
void vala_panel_toplevel_configure(ValaPanelToplevel *self, const char *page);
GtkMenu *vala_panel_toplevel_get_plugin_menu(ValaPanelToplevel *self, ValaPanelApplet *pl);
void vala_panel_toplevel_destroy_pref_dialog(ValaPanelToplevel *self);

G_END_DECLS

#endif // TOPLEVEL_H
