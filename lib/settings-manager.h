#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <gio/gio.h>
#include <glib.h>
#include <stdbool.h>

#define ROOT_NAME "profile"

#define VALA_PANEL_PLUGIN_SCHEMA "org.valapanel.toplevel.plugin"

#define VALA_PANEL_KEY_NAME "plugin-type"
#define VALA_PANEL_KEY_EXPAND "is-expanded"
#define VALA_PANEL_KEY_CAN_EXPAND "can-expand"
#define VALA_PANEL_KEY_PACK "pack-type"
#define VALA_PANEL_KEY_POSITION "position"

G_BEGIN_DECLS

typedef struct
{
	GSettingsBackend *backend;
	char *root_name;
	char *root_schema;
	char *root_path;
	GHashTable *all_units;
} ValaPanelCoreSettings;

typedef struct
{
	GSettings *default_settings;
	GSettings *custom_settings;
	char *path_elem;
	char *uuid;
} ValaPanelUnitSettings;

typedef ValaPanelUnitSettings *ValaPanelUnitSettingsPointer;
typedef ValaPanelCoreSettings *ValaPanelCoreSettingsPointer;

ValaPanelUnitSettings *vala_panel_unit_settings_new(ValaPanelCoreSettings *settings,
                                                    const char *name, const char *uuid);
void vala_panel_unit_settings_free(ValaPanelUnitSettings *settings);
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(ValaPanelUnitSettingsPointer, vala_panel_unit_settings_free, NULL);

ValaPanelCoreSettings *vala_panel_core_settings_new(const char *schema, const char *path,
                                                    const char *root, GSettingsBackend *backend);
void vala_panel_core_settings_free(ValaPanelCoreSettings *settings);
ValaPanelUnitSettings *vala_panel_core_settings_add_unit_settings(ValaPanelCoreSettings *settings,
                                                                  const char *name);
void vala_panel_core_settings_remove_unit_settings(ValaPanelCoreSettings *settings,
                                                   const char *name);
ValaPanelUnitSettings *vala_panel_core_settings_get_by_uuid(ValaPanelCoreSettings *settings,
                                                            const char *uuid);
char *vala_panel_core_settings_get_uuid();
bool vala_panel_core_settings_init_plugin_list(ValaPanelCoreSettings *settings);
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(ValaPanelCoreSettingsPointer, vala_panel_core_settings_free, NULL);

G_END_DECLS

#endif // SETTINGSMANAGER_H
