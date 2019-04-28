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

#include <inttypes.h>
#include <string.h>

#include "misc.h"

typedef struct
{
	pid_t pid;
} SpawnData;

void child_spawn_func(G_GNUC_UNUSED void *data)
{
	setpgid(0, getpgid(getppid()));
}

bool vala_panel_launch_with_context(GDesktopAppInfo *app_info, GAppLaunchContext *cxt, GList *uris)
{
	g_autoptr(GError) err = NULL;
	if (app_info == NULL)
		return false;
	bool ret = g_desktop_app_info_launch_uris_as_manager(G_DESKTOP_APP_INFO(app_info),
	                                                     uris,
	                                                     cxt,
	                                                     G_SPAWN_SEARCH_PATH,
	                                                     child_spawn_func,
	                                                     NULL,
	                                                     NULL,
	                                                     NULL,
	                                                     &err);
	if (err)
		g_warning("%s\n", err->message);
	return ret;
}

GAppInfo *vala_panel_get_default_for_uri(const char *uri)
{
	/* g_file_query_default_handler() calls
	 * g_app_info_get_default_for_uri_scheme() too, but we have to do it
	 * here anyway in case GFile can't parse @uri correctly.
	 */
	GAppInfo *app_info          = NULL;
	g_autofree char *uri_scheme = g_uri_parse_scheme(uri);
	if (uri_scheme != NULL && strlen(uri_scheme) <= 0)
		app_info = g_app_info_get_default_for_uri_scheme(uri_scheme);
	if (app_info == NULL)
	{
		g_autoptr(GFile) file = g_file_new_for_uri(uri);
		app_info              = g_file_query_default_handler(file, NULL, NULL);
	}
	return app_info;
}

void vala_panel_add_prop_as_action(GActionMap *map, const char *prop)
{
	g_autoptr(GAction) action = G_ACTION(g_property_action_new(prop, map, prop));
	g_action_map_add_action(map, action);
	g_clear_object(&map);
}

void vala_panel_add_gsettings_as_action(GActionMap *map, GSettings *settings, const char *prop)
{
	g_settings_bind(settings,
	                prop,
	                map,
	                prop,
	                (GSettingsBindFlags)(G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET |
	                                     G_SETTINGS_BIND_DEFAULT));
	g_autoptr(GAction) action = G_ACTION(g_settings_create_action(settings, prop));
	g_action_map_add_action(map, action);
}

void vala_panel_reset_schema(GSettings *settings)
{
	g_autoptr(GSettingsSchema) schema = NULL;
	g_object_get(settings, "settings-schema", &schema, NULL);
	g_auto(GStrv) keys = g_settings_schema_list_keys(schema);
	for (int i = 0; keys[i]; i++)
		g_settings_reset(settings, keys[i]);
}

void vala_panel_reset_schema_with_children(GSettings *settings)
{
	g_settings_delay(settings);
	vala_panel_reset_schema(settings);
	g_auto(GStrv) children = g_settings_list_children(settings);
	for (int i = 0; children[i]; i++)
	{
		g_autoptr(GSettings) child = g_settings_get_child(settings, children[i]);
		vala_panel_reset_schema(child);
	}
	g_settings_apply(settings);
}
