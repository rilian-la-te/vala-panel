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

#include <string.h>
#include <unistd.h>

#include "launcher-gtk.h"
#include "util/misc.h"

bool vala_panel_launch(GDesktopAppInfo *app_info, GList *uris, GtkWidget *parent)
{
	g_autoptr(GAppLaunchContext) cxt = G_APP_LAUNCH_CONTEXT(
	    gdk_display_get_app_launch_context(gtk_widget_get_display(parent)));
	return vala_panel_launch_with_context(app_info, cxt, uris);
}

void vala_panel_activate_launch_id(G_GNUC_UNUSED GSimpleAction *action, GVariant *param,
                             gpointer user_data)
{
	const char *id                  = g_variant_get_string(param, NULL);
	g_autoptr(GDesktopAppInfo) info = g_desktop_app_info_new(id);
	GtkApplication *app             = GTK_APPLICATION(user_data);
	GtkWidget *window               = GTK_WIDGET(gtk_application_get_windows(app)->data);
	vala_panel_launch(info, NULL, GTK_WIDGET(window));
}

void vala_panel_activate_launch_uri(G_GNUC_UNUSED GSimpleAction *action, GVariant *param,
                              gpointer user_data)
{
	const char *uri                 = g_variant_get_string(param, NULL);
	g_autoptr(GList) uris           = g_list_append(NULL, (gpointer)uri);
	g_autoptr(GDesktopAppInfo) info = G_DESKTOP_APP_INFO(vala_panel_get_default_for_uri(uri));
	GtkApplication *app             = GTK_APPLICATION(user_data);
	GtkWidget *window               = GTK_WIDGET(gtk_application_get_windows(app)->data);
	vala_panel_launch(info, uris, GTK_WIDGET(window));
}

void vala_panel_activate_launch_command(G_GNUC_UNUSED GSimpleAction *action, GVariant *param,
                                  gpointer user_data)
{
	g_autoptr(GError) err           = NULL;
	const char *commandline         = g_variant_get_string(param, NULL);
	g_autoptr(GDesktopAppInfo) info = G_DESKTOP_APP_INFO(
	    g_app_info_create_from_commandline(commandline, NULL, G_APP_INFO_CREATE_NONE, &err));
	if (err)
		g_warning("%s\n", err->message);
	GtkApplication *app = GTK_APPLICATION(user_data);
	GtkWidget *window   = GTK_WIDGET(gtk_application_get_windows(app)->data);
	vala_panel_launch(info, NULL, GTK_WIDGET(window));
}
