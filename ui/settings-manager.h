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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "constants.h"

#include <gio/gio.h>
#include <glib.h>
#include <stdbool.h>

#define ROOT_NAME "profile"

G_BEGIN_DECLS

typedef struct
{
	GSettingsBackend *backend;
	GSettings *core_settings;
	char *root_schema;
	char *root_path;
	GHashTable *all_units;
} ValaPanelCoreSettings;

typedef struct
{
	GSettings *type;
	GSettings *common;
	GSettings *custom;
	char *uuid;
} ValaPanelUnitSettings;

typedef enum
{
	TOPLEVEL = 0,
	APPLET   = 1,
} ValaPanelType;

#define vp_core_settings_remove_unit_settings(s, n)                                                \
	vp_core_settings_remove_unit_settings_full(s, n, false)

void vp_unit_settings_free(ValaPanelUnitSettings *settings);
bool vala_panel_unit_settings_is_toplevel(ValaPanelUnitSettings *settings);
GType vp_unit_settings_get_type(void);
G_DEFINE_AUTOPTR_CLEANUP_FUNC(ValaPanelUnitSettings, vp_unit_settings_free)

G_GNUC_INTERNAL ValaPanelCoreSettings *vp_core_settings_new(const char *schema, const char *path,
                                                            GSettingsBackend *backend);
void vp_core_settings_free(ValaPanelCoreSettings *settings);
G_GNUC_INTERNAL ValaPanelUnitSettings *vp_core_settings_add_unit_settings(
    ValaPanelCoreSettings *settings, const char *name, bool is_toplevel);
G_GNUC_INTERNAL ValaPanelUnitSettings *vp_core_settings_add_unit_settings_full(
    ValaPanelCoreSettings *settings, const char *name, const char *uuid, bool is_toplevel);

G_GNUC_INTERNAL void vp_core_settings_remove_unit_settings_full(ValaPanelCoreSettings *settings,
                                                                const char *name, bool destroy);
G_GNUC_INTERNAL ValaPanelUnitSettings *vp_core_settings_get_by_uuid(ValaPanelCoreSettings *settings,
                                                                    const char *uuid);
G_GNUC_INTERNAL char *vp_core_settings_get_uuid(void);
G_GNUC_INTERNAL bool vp_core_settings_init_unit_list(ValaPanelCoreSettings *settings);
GType vp_core_settings_get_type(void);
G_DEFINE_AUTOPTR_CLEANUP_FUNC(ValaPanelCoreSettings, vp_core_settings_free)

G_END_DECLS

#endif // SETTINGSMANAGER_H
