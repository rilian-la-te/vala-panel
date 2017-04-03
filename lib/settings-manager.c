/*
 * vala-panel
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
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

#include "settings-manager.h"
#include "config.h"
#include "definitions.h"
#include "misc.h"

#ifndef G252
#include "guuid.h"
#endif

ValaPanelUnitSettings *vala_panel_unit_settings_new(ValaPanelCoreSettings *settings,
                                                    const char *name, const char *uuid)
{
	ValaPanelUnitSettings *created_settings = g_new(ValaPanelUnitSettings, 1);
	created_settings->uuid                  = g_strdup(uuid);
	g_autofree gchar *path =
	    g_strdup_printf("%s%s/", settings->root_path, created_settings->uuid);
	created_settings->default_settings =
	    g_settings_new_with_backend_and_path(VALA_PANEL_PLUGIN_SCHEMA, settings->backend, path);
	g_autofree char *tname = g_strdup(name);
	if (tname == NULL)
		g_settings_get(created_settings->default_settings,
		               VALA_PANEL_KEY_NAME,
		               "s",
		               &tname,
		               NULL);
	created_settings->path_elem = g_strdup(tname);
	g_autofree gchar *id =
	    g_strdup_printf("%s.%s", settings->root_schema, created_settings->path_elem);

	GSettingsSchemaSource *source     = g_settings_schema_source_get_default();
	g_autoptr(GSettingsSchema) schema = g_settings_schema_source_lookup(source, id, true);
	if (schema != NULL)
		created_settings->custom_settings =
		    g_settings_new_with_backend_and_path(id, settings->backend, path);
	return created_settings;
}

void vala_panel_unit_settings_free(ValaPanelUnitSettings *settings)
{
	if (!settings)
		return;
	if (settings->custom_settings)
	{
		g_object_unref(settings->custom_settings);
		settings->custom_settings = NULL;
	}
	if (settings->default_settings)
		g_object_unref(settings->default_settings);
	if (settings->path_elem)
	{
		g_free(settings->path_elem);
		settings->path_elem = NULL;
	}
	g_free(settings);
	settings = NULL;
}

ValaPanelCoreSettings *vala_panel_core_settings_new(const char *schema, const char *path,
                                                    const char *root, GSettingsBackend *backend)
{
	ValaPanelCoreSettings *new_settings = g_new0(ValaPanelCoreSettings, 1);
	new_settings->all_units =
	    g_hash_table_new_full(g_str_hash,
	                          g_str_equal,
	                          g_free,
	                          (GDestroyNotify)vala_panel_unit_settings_free);
	new_settings->root_name   = g_strdup(root);
	new_settings->root_path   = g_strdup(path);
	new_settings->root_schema = g_strdup(schema);
	new_settings->backend     = g_object_ref(backend);
	return new_settings;
}

void vala_panel_core_settings_free(ValaPanelCoreSettings *settings)
{
	g_free0(settings->root_name);
	g_free0(settings->root_path);
	g_free0(settings->root_schema);
	g_object_unref0(settings->backend);
	g_hash_table_unref(settings->all_units);
	g_free0(settings);
}

ValaPanelUnitSettings *vala_panel_core_settings_add_unit_settings_full(
    ValaPanelCoreSettings *settings, const char *name, const char *uuid)
{
	ValaPanelUnitSettings *usettings = vala_panel_unit_settings_new(settings, name, uuid);
	g_hash_table_insert(settings->all_units, g_strdup(uuid), usettings);
	return usettings;
}

ValaPanelUnitSettings *vala_panel_core_settings_add_unit_settings(ValaPanelCoreSettings *settings,
                                                                  const char *name)
{
	g_autofree char *uuid = vala_panel_core_settings_get_uuid();
	return vala_panel_core_settings_add_unit_settings_full(settings, name, uuid);
}

void vala_panel_core_settings_remove_unit_settings(ValaPanelCoreSettings *settings,
                                                   const char *name)
{
	g_auto(ValaPanelUnitSettingsPointer) removing_unit =
	    (ValaPanelUnitSettings *)g_hash_table_lookup(settings->all_units, name);
	vala_panel_reset_schema_with_children(removing_unit->default_settings);
	if (removing_unit->custom_settings != NULL)
		vala_panel_reset_schema_with_children(removing_unit->custom_settings);
	g_free(removing_unit);
}

ValaPanelUnitSettings *vala_panel_core_settings_get_by_uuid(ValaPanelCoreSettings *settings,
                                                            const char *uuid)
{
	return (ValaPanelUnitSettings *)g_hash_table_lookup(settings->all_units, uuid);
}

bool vala_panel_core_settings_init_toplevel_plugin_list(ValaPanelCoreSettings *settings,
                                                        ValaPanelUnitSettings *toplevel_settings)
{
	g_auto(GStrv) applets_list = NULL;
	GSettings *tsettings       = toplevel_settings->default_settings;
	g_settings_get(tsettings, "applets", "as", &applets_list, NULL);
	for (int i = 0; applets_list[i] != NULL; i++)
	{
		g_autofree char *applet_uuid = g_strdup(applets_list[i]);
		vala_panel_core_settings_add_unit_settings_full(settings, NULL, applet_uuid);
	}
	return true;
}

char *vala_panel_core_settings_get_uuid()
{
	return g_uuid_string_random();
}
