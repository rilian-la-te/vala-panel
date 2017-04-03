#include "settings-manager.h"

ValaPanelUnitSettings *vala_panel_unit_settings_new(ValaPanelCoreSettings *settings, const char *name, const char *uuid)
{

}

void vala_panel_unit_settings_free(ValaPanelUnitSettings *settings)
{
    if (!settings) return;
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

ValaPanelCoreSettings *vala_panel_core_settings_new(const char *schema, GSettingsBackend *backend, const char *path)
{

}

void vala_panel_core_settings_free(ValaPanelCoreSettings *settings)
{

}

ValaPanelUnitSettings *vala_panel_core_settings_add_unit_settings(const char *name)
{

}

void vala_panel_core_settings_remove_unit_settings(const char *uuid)
{

}

ValaPanelUnitSettings *vala_panel_core_settings_get_by_uuid(const char *uuid)
{

}

bool init_plugin_list()
{

}
