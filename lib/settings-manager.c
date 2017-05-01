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

#include <glib-object.h>

#include "definitions.h"
#include "misc.h"
#include "panel-platform.h"
#include "settings-manager.h"

#ifndef G252
#include "guuid.h"
#endif

ValaPanelUnitSettings *vala_panel_unit_settings_new(ValaPanelCoreSettings *settings,
                                                    const char *name, const char *uuid,
                                                    bool is_toplevel)
{
	ValaPanelUnitSettings *created_settings = g_slice_new(ValaPanelUnitSettings);
	created_settings->uuid                  = g_strdup(uuid);
	g_autofree gchar *path =
	    g_strdup_printf("%s%s/", settings->root_path, created_settings->uuid);
	created_settings->type_settings =
	    g_settings_new_with_backend_and_path(VALA_PANEL_OBJECT_SCHEMA, settings->backend, path);
	g_settings_set_enum(created_settings->type_settings,
	                    VALA_PANEL_OBJECT_TYPE,
	                    is_toplevel ? TOPLEVEL : APPLET);
	created_settings->default_settings =
	    g_settings_new_with_backend_and_path(is_toplevel ? VALA_PANEL_TOPLEVEL_SCHEMA
	                                                     : VALA_PANEL_PLUGIN_SCHEMA,
	                                         settings->backend,
	                                         path);
	g_autofree char *tname = is_toplevel ? g_strdup("toplevel") : g_strdup(name);
	if (tname == NULL)
		g_settings_get(created_settings->default_settings,
		               VALA_PANEL_KEY_NAME,
		               "s",
		               &tname,
		               NULL);
	created_settings->schema_elem = g_strdup(tname);
	g_autofree gchar *id =
	    g_strdup_printf("%s.%s", settings->root_schema, created_settings->schema_elem);

	GSettingsSchemaSource *source     = g_settings_schema_source_get_default();
	g_autoptr(GSettingsSchema) schema = g_settings_schema_source_lookup(source, id, true);
	if (schema != NULL)
		created_settings->custom_settings =
		    g_settings_new_with_backend_and_path(id, settings->backend, path);
	else
		created_settings->custom_settings = NULL;
	return created_settings;
}

static ValaPanelUnitSettings *vala_panel_unit_settings_copy(ValaPanelUnitSettings *source)
{
	ValaPanelUnitSettings *created_settings = g_new(ValaPanelUnitSettings, 1);
	created_settings->uuid                  = g_strdup(source->uuid);
	created_settings->schema_elem           = g_strdup(source->schema_elem);
	created_settings->default_settings = G_SETTINGS(g_object_ref(source->default_settings));
	created_settings->type_settings    = G_SETTINGS(g_object_ref(source->type_settings));
	created_settings->custom_settings  = NULL;
	if (source->custom_settings)
		created_settings->custom_settings =
		    G_SETTINGS(g_object_ref(source->custom_settings));
	return created_settings;
}

void vala_panel_unit_settings_free(ValaPanelUnitSettings *settings)
{
	if (!settings)
		return;
	g_object_unref0(settings->custom_settings);
	g_object_unref0(settings->default_settings);
	g_object_unref0(settings->type_settings);
	g_free0(settings->schema_elem);
	g_slice_free(ValaPanelUnitSettings, settings);
}

G_DEFINE_BOXED_TYPE(ValaPanelUnitSettings, vala_panel_unit_settings, vala_panel_unit_settings_copy,
                    vala_panel_unit_settings_free)

ValaPanelCoreSettings *vala_panel_core_settings_new(const char *schema, const char *path,
                                                    GSettingsBackend *backend)
{
	ValaPanelCoreSettings *new_settings = g_slice_new(ValaPanelCoreSettings);
	new_settings->all_units =
	    g_hash_table_new_full(g_str_hash,
	                          g_str_equal,
	                          g_free,
	                          (GDestroyNotify)vala_panel_unit_settings_free);
	new_settings->root_path   = g_strdup(path);
	new_settings->root_schema = g_strdup(schema);
	new_settings->backend     = g_object_ref(backend);
	g_autofree char *core_path =
	    g_strdup_printf("%s%s/", new_settings->root_path, VALA_PANEL_CORE_PATH_ELEM);
	new_settings->core_settings = g_settings_new_with_backend_and_path(VALA_PANEL_CORE_SCHEMA,
	                                                                   new_settings->backend,
	                                                                   core_path);
	return new_settings;
}

ValaPanelCoreSettings *vala_panel_core_settings_copy(ValaPanelCoreSettings *settings)
{
	ValaPanelCoreSettings *new_settings = g_slice_new0(ValaPanelCoreSettings);
	new_settings->root_path             = g_strdup(settings->root_path);
	new_settings->root_schema           = g_strdup(settings->root_schema);
	new_settings->core_settings         = g_object_ref(settings->core_settings);
	new_settings->backend               = g_object_ref(settings->backend);
	new_settings->all_units             = g_hash_table_ref(settings->all_units);
	return new_settings;
}

void vala_panel_core_settings_free(ValaPanelCoreSettings *settings)
{
	g_free0(settings->root_path);
	g_free0(settings->root_schema);
	g_object_unref0(settings->core_settings);
	g_object_unref0(settings->backend);
	g_hash_table_unref(settings->all_units);
	g_slice_free(ValaPanelCoreSettings, settings);
}

G_DEFINE_BOXED_TYPE(ValaPanelCoreSettings, vala_panel_core_settings, vala_panel_core_settings_copy,
                    vala_panel_core_settings_free)

static void vala_panel_core_settings_sync(ValaPanelCoreSettings *settings)
{
	g_autofree GStrv unit_list =
	    (GStrv)g_hash_table_get_keys_as_array(settings->all_units, NULL);
	g_settings_set_strv(settings->core_settings, VALA_PANEL_CORE_UNITS, unit_list);
}

static ValaPanelUnitSettings *vala_panel_core_settings_load_unit_settings(
    ValaPanelCoreSettings *settings, const char *name, const char *uuid)
{
	g_autofree gchar *path = g_strdup_printf("%s%s/", settings->root_path, uuid);
	g_autoptr(GSettings) s =
	    g_settings_new_with_backend_and_path(VALA_PANEL_OBJECT_SCHEMA, settings->backend, path);
	int en = g_settings_get_enum(s, VALA_PANEL_OBJECT_TYPE);
	ValaPanelUnitSettings *usettings =
	    vala_panel_unit_settings_new(settings, name, uuid, en == TOPLEVEL);
	g_hash_table_insert(settings->all_units, g_strdup(uuid), usettings);
	return usettings;
}

ValaPanelUnitSettings *vala_panel_core_settings_add_unit_settings_full(
    ValaPanelCoreSettings *settings, const char *name, const char *uuid, bool is_toplevel)
{
	ValaPanelUnitSettings *usettings =
	    vala_panel_unit_settings_new(settings, name, uuid, is_toplevel);
	g_hash_table_insert(settings->all_units, g_strdup(uuid), usettings);
	vala_panel_core_settings_sync(settings);
	return usettings;
}

ValaPanelUnitSettings *vala_panel_core_settings_add_unit_settings(ValaPanelCoreSettings *settings,
                                                                  const char *name,
                                                                  bool is_toplevel)
{
	g_autofree char *uuid = vala_panel_core_settings_get_uuid();
	return vala_panel_core_settings_add_unit_settings_full(settings, name, uuid, is_toplevel);
}

void vala_panel_core_settings_remove_unit_settings_full(ValaPanelCoreSettings *settings,
                                                        const char *name, bool destroy)
{
	if (destroy)
	{
		ValaPanelUnitSettings *removing_unit =
		    (ValaPanelUnitSettings *)g_hash_table_lookup(settings->all_units, name);
		vala_panel_reset_schema_with_children(removing_unit->default_settings);
		vala_panel_reset_schema_with_children(removing_unit->type_settings);
		if (removing_unit->custom_settings != NULL)
			vala_panel_reset_schema_with_children(removing_unit->custom_settings);
	}
	g_hash_table_remove(settings->all_units, name);
	if (destroy)
		vala_panel_core_settings_sync(settings);
}

ValaPanelUnitSettings *vala_panel_core_settings_get_by_uuid(ValaPanelCoreSettings *settings,
                                                            const char *uuid)
{
	return (ValaPanelUnitSettings *)g_hash_table_lookup(settings->all_units, uuid);
}

bool vala_panel_core_settings_init_unit_list(ValaPanelCoreSettings *settings)
{
	g_auto(GStrv) unit_list =
	    g_settings_get_strv(settings->core_settings, VALA_PANEL_CORE_UNITS);
	for (int i = 0; unit_list[i] != NULL; i++)
	{
		g_autofree char *applet_uuid = g_strdup(unit_list[i]);
		vala_panel_core_settings_load_unit_settings(settings, NULL, applet_uuid);
	}
	GList *keys = g_hash_table_get_keys(settings->all_units);
	uint len    = g_list_length(keys);
	g_list_free(keys);
	return len;
}

char *vala_panel_core_settings_get_uuid()
{
	return g_uuid_string_random();
}
