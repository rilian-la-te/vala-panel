/*
 * vala-panel
 * Copyright (C) 2015-2018 Konstantin Pugin <ria.freelander@gmail.com>
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
#ifndef APPLETINFO_H
#define APPLETINFO_H

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

typedef struct _ValaPanelAppeltInfo ValaPanelAppletInfo;

#define VALA_PANEL_APPLET_EXCLUSIVE "ValaPanel-Exclusive"
#define VALA_PANEL_APPLET_EXTENSION_POINT "vala-panel-applet-module"

GType vp_applet_info_get_type();
ValaPanelAppletInfo *vp_applet_info_load(const char *extension_name, GType plugin_type);
ValaPanelAppletInfo *vp_applet_info_duplicate(void *info);
void vp_applet_info_free(void *info);

GType vp_applet_info_get_stored_type(ValaPanelAppletInfo *info);
const char *vp_applet_info_get_module_name(ValaPanelAppletInfo *info);
const char *vp_applet_info_get_name(ValaPanelAppletInfo *info);
const char *vp_applet_info_get_description(ValaPanelAppletInfo *info);
const char *vp_applet_info_get_icon_name(ValaPanelAppletInfo *info);
const char *const *vp_applet_info_get_authors(ValaPanelAppletInfo *info);
const char *const *vp_applet_info_get_platforms(ValaPanelAppletInfo *info);
const char *vp_applet_info_get_website(ValaPanelAppletInfo *info);
const char *vp_applet_info_get_help_uri(ValaPanelAppletInfo *info);
GtkLicense vp_applet_info_get_license(ValaPanelAppletInfo *info);
const char *vp_applet_info_get_version(ValaPanelAppletInfo *info);
bool vp_applet_info_is_exclusive(ValaPanelAppletInfo *info);

GtkWidget *vp_applet_info_get_about_widget(ValaPanelAppletInfo *info);
void vp_applet_info_show_about_dialog(ValaPanelAppletInfo *info);

G_END_DECLS

#endif // APPLETINFO_H
