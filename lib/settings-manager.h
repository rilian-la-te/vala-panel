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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "config.h"
#include "constants.h"

#include <gio/gio.h>
#include <glib.h>
#include <stdbool.h>

#define ROOT_NAME "profile"

G_BEGIN_DECLS

typedef struct
{
	GSettingsBackend *backend;
	char *root_schema;
	char *root_path;
	GHashTable *all_units;
} ValaPanelCoreSettings;

typedef struct
{
	GSettings *default_settings;
	GSettings *custom_settings;
	char *schema_elem;
	char *uuid;
} ValaPanelUnitSettings;

typedef ValaPanelUnitSettings *ValaPanelUnitSettingsPointer;
typedef ValaPanelCoreSettings *ValaPanelCoreSettingsPointer;

ValaPanelUnitSettings *vala_panel_unit_settings_new(ValaPanelCoreSettings *settings,
                                                    const char *name, const char *uuid,
                                                    bool is_toplevel);
void vala_panel_unit_settings_free(ValaPanelUnitSettings *settings);
GType vala_panel_unit_settings_get_type();
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(ValaPanelUnitSettingsPointer, vala_panel_unit_settings_free, NULL);

ValaPanelCoreSettings *vala_panel_core_settings_new(const char *schema, const char *path,
                                                    GSettingsBackend *backend);
void vala_panel_core_settings_free(ValaPanelCoreSettings *settings);
ValaPanelUnitSettings *vala_panel_core_settings_add_unit_settings(ValaPanelCoreSettings *settings,
                                                                  const char *name,
                                                                  bool is_toplevel);
ValaPanelUnitSettings *vala_panel_core_settings_add_unit_settings_full(
    ValaPanelCoreSettings *settings, const char *name, const char *uuid, bool is_toplevel);
void vala_panel_core_settings_remove_unit_settings(ValaPanelCoreSettings *settings,
                                                   const char *name);
void vala_panel_core_settings_destroy_unit_settings(ValaPanelCoreSettings *settings,
                                                    const char *name);
ValaPanelUnitSettings *vala_panel_core_settings_get_by_uuid(ValaPanelCoreSettings *settings,
                                                            const char *uuid);
char *vala_panel_core_settings_get_uuid();
bool vala_panel_core_settings_init_toplevel_plugin_list(ValaPanelCoreSettings *settings,
                                                        ValaPanelUnitSettings *toplevel_settings);
GType vala_panel_core_settings_get_type();
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(ValaPanelCoreSettingsPointer, vala_panel_core_settings_free, NULL);

G_END_DECLS

#endif // SETTINGSMANAGER_H
