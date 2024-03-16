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

GType vala_panel_unit_settings_get_type();
G_GNUC_INTERNAL void vala_panel_unit_settings_free(ValaPanelUnitSettings *settings);
bool vala_panel_unit_settings_is_toplevel(ValaPanelUnitSettings *settings);
G_DEFINE_AUTOPTR_CLEANUP_FUNC(ValaPanelUnitSettings, vala_panel_unit_settings_free)

GType vala_panel_core_settings_get_type();
G_GNUC_INTERNAL void vala_panel_core_settings_free(ValaPanelCoreSettings *settings);
G_DEFINE_AUTOPTR_CLEANUP_FUNC(ValaPanelCoreSettings, vala_panel_core_settings_free)

G_END_DECLS

#endif // SETTINGSMANAGER_H
