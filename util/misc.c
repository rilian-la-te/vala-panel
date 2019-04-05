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

void vala_panel_add_prop_as_action(GActionMap *map, const char *prop)
{
	g_autoptr(GAction) action = G_ACTION(g_property_action_new(prop, map, prop));
	g_action_map_add_action(map, action);
}

void vala_panel_add_gsettings_as_action(GActionMap *map, GSettings *settings, const char *prop)
{
	g_settings_bind(settings,
	                prop,
	                map,
	                prop,
	                (GSettingsBindFlags)(G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET |
	                                     G_SETTINGS_BIND_DEFAULT));
	g_autoptr(GAction) action = g_settings_create_action(settings, prop);
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
