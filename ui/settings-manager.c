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

#include "config.h"

#include <glib-object.h>

#include "definitions.h"
#include "misc.h"
#include "panel-platform.h"
#include "settings-manager.h"

static ValaPanelUnitSettings *vp_unit_settings_new(ValaPanelCoreSettings *settings,
                                                   const char *name, const char *uuid,
                                                   bool is_toplevel)
{
	ValaPanelUnitSettings *created_settings = g_slice_new(ValaPanelUnitSettings);
	created_settings->uuid                  = g_strdup(uuid);
	g_autofree char *path =
	    g_strdup_printf("%s%s/", settings->root_path, created_settings->uuid);
	created_settings->type =
	    g_settings_new_with_backend_and_path(VALA_PANEL_OBJECT_SCHEMA, settings->backend, path);
	g_settings_set_enum(created_settings->type,
	                    VALA_PANEL_OBJECT_TYPE,
	                    is_toplevel ? TOPLEVEL : APPLET);
	created_settings->common =
	    g_settings_new_with_backend_and_path(is_toplevel ? VALA_PANEL_TOPLEVEL_SCHEMA
	                                                     : VALA_PANEL_PLUGIN_SCHEMA,
	                                         settings->backend,
	                                         path);
	g_autofree char *tname = is_toplevel ? g_strdup("toplevel") : g_strdup(name);
	if (tname == NULL)
		tname = g_settings_get_string(created_settings->common, VP_KEY_NAME);

	GSettingsSchemaSource *source = g_settings_schema_source_get_default();

	/* New way to find a schema */
	g_autoptr(GSettingsSchema) new_schema =
	    g_settings_schema_source_lookup(source, tname, true);

	if (new_schema != NULL)
		created_settings->custom =
		    g_settings_new_with_backend_and_path(tname, settings->backend, path);
	else
		created_settings->custom = NULL;
	return created_settings;
}

static ValaPanelUnitSettings *vp_unit_settings_copy(ValaPanelUnitSettings *source)
{
	ValaPanelUnitSettings *created_settings = g_slice_new0(ValaPanelUnitSettings);
	created_settings->uuid                  = g_strdup(source->uuid);
	created_settings->common                = G_SETTINGS(g_object_ref(source->common));
	created_settings->type                  = G_SETTINGS(g_object_ref(source->type));
	created_settings->custom                = NULL;
	if (source->custom)
		created_settings->custom = G_SETTINGS(g_object_ref(source->custom));
	return created_settings;
}

void vp_unit_settings_free(ValaPanelUnitSettings *settings)
{
	if (!settings)
		return;
	g_clear_object(&settings->custom);
	g_clear_object(&settings->common);
	g_clear_object(&settings->type);
	g_clear_pointer(&settings->uuid, g_free);
	g_slice_free(ValaPanelUnitSettings, settings);
}

G_DEFINE_BOXED_TYPE(ValaPanelUnitSettings, vp_unit_settings, vp_unit_settings_copy,
                    vp_unit_settings_free)

bool vala_panel_unit_settings_is_toplevel(ValaPanelUnitSettings *settings)
{
	g_autofree char *id;
	g_object_get(settings->common, "schema-id", &id, NULL);
	return !g_strcmp0(id, VALA_PANEL_TOPLEVEL_SCHEMA);
}

G_GNUC_INTERNAL ValaPanelCoreSettings *vp_core_settings_new(const char *schema, const char *path,
                                                            GSettingsBackend *backend)
{
	ValaPanelCoreSettings *new_settings = g_slice_new(ValaPanelCoreSettings);
	new_settings->all_units             = g_hash_table_new_full(g_str_hash,
                                                        g_str_equal,
                                                        g_free,
                                                        (GDestroyNotify)vp_unit_settings_free);
	new_settings->root_path             = g_strdup(path);
	new_settings->root_schema           = g_strdup(schema);
	new_settings->backend               = g_object_ref_sink(backend);
	g_autofree char *core_path =
	    g_strdup_printf("%s%s/", new_settings->root_path, VALA_PANEL_CORE_PATH_ELEM);
	new_settings->core_settings = g_settings_new_with_backend_and_path(VALA_PANEL_CORE_SCHEMA,
	                                                                   new_settings->backend,
	                                                                   core_path);
	return new_settings;
}

G_GNUC_INTERNAL ValaPanelCoreSettings *vp_core_settings_copy(ValaPanelCoreSettings *settings)
{
	ValaPanelCoreSettings *new_settings = g_slice_new0(ValaPanelCoreSettings);
	new_settings->root_path             = g_strdup(settings->root_path);
	new_settings->root_schema           = g_strdup(settings->root_schema);
	new_settings->core_settings         = g_object_ref(settings->core_settings);
	new_settings->backend               = g_object_ref(settings->backend);
	new_settings->all_units             = g_hash_table_ref(settings->all_units);
	return new_settings;
}

void vp_core_settings_free(ValaPanelCoreSettings *settings)
{
	g_free0(settings->root_path);
	g_free0(settings->root_schema);
	g_clear_object(&settings->core_settings);
	g_hash_table_unref(settings->all_units);
	g_clear_object(&settings->backend);
	g_slice_free(ValaPanelCoreSettings, settings);
}

G_DEFINE_BOXED_TYPE(ValaPanelCoreSettings, vp_core_settings, vp_core_settings_copy,
                    vp_core_settings_free)

static void vp_core_settings_sync(ValaPanelCoreSettings *settings)
{
	g_autofree GStrv unit_list =
	    (GStrv)g_hash_table_get_keys_as_array(settings->all_units, NULL); // We should free only
	                                                                      // container here
	g_settings_set_strv(settings->core_settings, VALA_PANEL_CORE_UNITS, unit_list);
}

static ValaPanelUnitSettings *vp_core_settings_load_unit_settings(ValaPanelCoreSettings *settings,
                                                                  const char *uuid)
{
	g_autofree char *path = g_strdup_printf("%s%s/", settings->root_path, uuid);
	g_autoptr(GSettings) s =
	    g_settings_new_with_backend_and_path(VALA_PANEL_OBJECT_SCHEMA, settings->backend, path);
	int en = g_settings_get_enum(s, VALA_PANEL_OBJECT_TYPE);
	ValaPanelUnitSettings *usettings =
	    vp_unit_settings_new(settings, NULL, uuid, en == TOPLEVEL);
	g_hash_table_insert(settings->all_units, g_strdup(uuid), usettings);
	return usettings;
}

G_GNUC_INTERNAL ValaPanelUnitSettings *vp_core_settings_add_unit_settings_full(
    ValaPanelCoreSettings *settings, const char *name, const char *uuid, bool is_toplevel)
{
	ValaPanelUnitSettings *usettings = vp_unit_settings_new(settings, name, uuid, is_toplevel);
	g_hash_table_insert(settings->all_units, g_strdup(uuid), usettings);
	vp_core_settings_sync(settings);
	return usettings;
}

G_GNUC_INTERNAL ValaPanelUnitSettings *vp_core_settings_add_unit_settings(
    ValaPanelCoreSettings *settings, const char *name, bool is_toplevel)
{
	g_autofree char *uuid = g_uuid_string_random();
	return vp_core_settings_add_unit_settings_full(settings, name, uuid, is_toplevel);
}

G_GNUC_INTERNAL void vp_core_settings_remove_unit_settings_full(ValaPanelCoreSettings *settings,
                                                                const char *name, bool destroy)
{
	if (destroy)
	{
		ValaPanelUnitSettings *removing_unit =
		    (ValaPanelUnitSettings *)g_hash_table_lookup(settings->all_units, name);
		vala_panel_reset_schema_with_children(removing_unit->common);
		vala_panel_reset_schema_with_children(removing_unit->type);
		if (removing_unit->custom != NULL)
			vala_panel_reset_schema_with_children(removing_unit->custom);
	}
	g_hash_table_remove(settings->all_units, name);
	if (destroy)
	{
		vp_core_settings_sync(settings);
		g_settings_sync();
	}
}

G_GNUC_INTERNAL ValaPanelUnitSettings *vp_core_settings_get_by_uuid(ValaPanelCoreSettings *settings,
                                                                    const char *uuid)
{
	return (ValaPanelUnitSettings *)g_hash_table_lookup(settings->all_units, uuid);
}

G_GNUC_INTERNAL bool vp_core_settings_init_unit_list(ValaPanelCoreSettings *settings)
{
	g_auto(GStrv) unit_list =
	    g_settings_get_strv(settings->core_settings, VALA_PANEL_CORE_UNITS);
	for (int i = 0; unit_list[i] != NULL; i++)
		vp_core_settings_load_unit_settings(settings, unit_list[i]);
	return g_hash_table_size(settings->all_units);
}
