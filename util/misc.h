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

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <stdbool.h>

G_BEGIN_DECLS

/**
 * vala_panel_get_default_for_uri: 
 * @uri: a URI to get default transfer from
 *
 * Use it to get default #GAppInfo to handle a file or URI.
 * Returns: (transfer full) (nullable): a #GAppInfo for handling a URI (desktop default)
 */

GAppInfo *vala_panel_get_default_for_uri(const char *uri);
/**
 * vala_panel_launch_with_context:
 * @app_info: (not nullable): a #GDesktopAppInfo
 * @cxt: (not nullable): a #GAppLaunchContext for launching application
 * @uris: (element-type utf8) (nullable): list of URI identifiers to passing to programs for launching
 *
 * Allow you to launch an #GDesktopAppInfo with list of URIs and with provided #GAppLaunchContext. You
 * most likely will not want this, and use #vala_panel_launch instead.
 * Returns: true on successful launch, false otherwise. 
 */
bool vala_panel_launch_with_context(GDesktopAppInfo *app_info, GAppLaunchContext *cxt, GList *uris);
void vala_panel_spawn_setup_func(void *data);
void vala_panel_add_prop_as_action(GActionMap *map, const char *prop);
void vala_panel_add_gsettings_as_action(GActionMap *map, GSettings *settings, const char *prop);
void vala_panel_reset_schema(GSettings *settings);
void vala_panel_reset_schema_with_children(GSettings *settings);

G_END_DECLS

#endif // MISC_H
