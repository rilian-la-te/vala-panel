#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <glib.h>
#include <gio/gio.h>
#include <stdbool.h>

#define ROOT_NAME "profile"

G_BEGIN_DECLS

typedef struct {
    GSettingsBackend* backend;
    char* root_name;
    char* root_schema;
    char* root_path;
    GHashTable* all_units;
} ValaPanelCoreSettings;

typedef struct {
    GSettings* default_settings;
    GSettings* custom_settings;
    char* path_elem;
} ValaPanelUnitSettings;

typedef ValaPanelUnitSettings* ValaPanelUnitSettingsPointer;
typedef ValaPanelCoreSettings* ValaPanelCoreSettingsPointer;

ValaPanelUnitSettings* vala_panel_unit_settings_new(ValaPanelCoreSettings* settings, const char* name, const char* uuid);
void vala_panel_unit_settings_free(ValaPanelUnitSettings* settings);
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(ValaPanelUnitSettingsPointer,vala_panel_unit_settings_free,NULL);

ValaPanelCoreSettings* vala_panel_core_settings_new(const char* schema, GSettingsBackend* backend, const char* path);
void vala_panel_core_settings_free(ValaPanelCoreSettings* settings);
ValaPanelUnitSettings* vala_panel_core_settings_add_unit_settings(const char* name);
void vala_panel_core_settings_remove_unit_settings(const char* uuid);
ValaPanelUnitSettings* vala_panel_core_settings_get_by_uuid(const char* uuid);
bool init_plugin_list();
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(ValaPanelCoreSettingsPointer,vala_panel_core_settings_free,NULL);

G_END_DECLS

#endif // SETTINGSMANAGER_H
