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
#include "applet-manager.h"
#include "applet-info.h"
#include "applet-plugin.h"
#include "config.h"

static AppletInfoData *applet_info_data_new(ValaPanelAppletInfo *info,
                                            ValaPanelAppletPlugin *plugin)
{
	AppletInfoData *ret = (AppletInfoData *)g_slice_alloc0(sizeof(AppletInfoData));
	ret->info           = info;
	ret->plugin         = plugin;
	ret->count          = 0;
	return ret;
}

static void applet_info_data_free(void *adata)
{
	AppletInfoData *data = (AppletInfoData *)adata;
	vala_panel_applet_info_free(data->info);
	g_object_unref(data->plugin);
	g_slice_free(AppletInfoData, data);
}

GIOExtensionPoint *applet_point = NULL;

struct _ValaPanelAppletManager
{
	GObject parent;
	GHashTable *applet_info_table;
};

G_DEFINE_TYPE(ValaPanelAppletManager, vala_panel_applet_manager, G_TYPE_OBJECT)

void vala_panel_applet_manager_reload_applets(ValaPanelAppletManager *self)
{
	g_io_modules_scan_all_in_directory(PLUGINS_DIRECTORY);
	GList *loaded_applets = g_io_extension_point_get_extensions(applet_point);
	for (GList *i = loaded_applets; i != NULL; i = g_list_next(i))
	{
		const char *module_name = g_io_extension_get_name((GIOExtension *)i->data);
		if (!g_hash_table_contains(self->applet_info_table, module_name))
		{
			ValaPanelAppletInfo *info = vala_panel_applet_info_load(module_name);
			ValaPanelAppletPlugin *pl = VALA_PANEL_APPLET_PLUGIN(
			    g_object_new(g_io_extension_get_type((GIOExtension *)i->data), NULL));
			AppletInfoData *data = applet_info_data_new(info, pl);
			g_hash_table_insert(self->applet_info_table, g_strdup(module_name), data);
		}
	}
}

AppletInfoData *vala_panel_applet_manager_applet_ref(ValaPanelAppletManager *self, const char *name)
{
	if (g_hash_table_contains(self->applet_info_table, name))
	{
		AppletInfoData *data =
		    (AppletInfoData *)g_hash_table_lookup(self->applet_info_table, name);
		if (data != NULL)
		{
			data->count += 1;
			return data;
		}
	}
	return NULL;
}

void vala_panel_applet_manager_applet_unref(ValaPanelAppletManager *self, const char *name)
{
	if (g_hash_table_contains(self->applet_info_table, name))
	{
		AppletInfoData *data =
		    (AppletInfoData *)g_hash_table_lookup(self->applet_info_table, name);
		if (data != NULL && data->count > 0)
		{
			data->count -= 1;
			return;
		}
	}
	return;
}

static AppletInfoData *vala_panel_applet_manager_get_info_data(ValaPanelAppletManager *self,
                                                               ValaPanelApplet *pl,
                                                               ValaPanelCoreSettings *core_settings)
{
	ValaPanelUnitSettings *settings =
	    vala_panel_core_settings_get_by_uuid(core_settings, vala_panel_applet_get_uuid(pl));
	g_autofree char *str =
	    g_settings_get_string(settings->default_settings, VALA_PANEL_KEY_NAME);
	return (AppletInfoData *)g_hash_table_lookup(self->applet_info_table, str);
}

ValaPanelAppletPlugin *vala_panel_applet_manager_get_plugin(ValaPanelAppletManager *self,
                                                            ValaPanelApplet *pl,
                                                            ValaPanelCoreSettings *core_settings)
{
	AppletInfoData *data = vala_panel_applet_manager_get_info_data(self, pl, core_settings);
	return data->plugin;
}

ValaPanelAppletInfo *vala_panel_applet_manager_get_plugin_info(ValaPanelAppletManager *self,
                                                               ValaPanelApplet *pl,
                                                               ValaPanelCoreSettings *core_settings)
{
	AppletInfoData *data = vala_panel_applet_manager_get_info_data(self, pl, core_settings);
	return data->info;
}

GList *vala_panel_applet_manager_get_all_types(ValaPanelAppletManager *self)
{
	return g_hash_table_get_values(self->applet_info_table);
}

static void vala_panel_applet_manager_init(ValaPanelAppletManager *self)
{
	self->applet_info_table =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, applet_info_data_free);
	vala_panel_applet_manager_reload_applets(self);
}

static void vala_panel_applet_manager_class_init(ValaPanelAppletManagerClass *klass)
{
	applet_point = g_io_extension_point_register(VALA_PANEL_APPLET_EXTENSION_POINT);
	g_io_extension_point_set_required_type(applet_point, vala_panel_applet_plugin_get_type());
}

ValaPanelAppletManager *vala_panel_applet_manager_new()
{
	return VALA_PANEL_APPLET_MANAGER(g_object_new(vala_panel_applet_manager_get_type(), NULL));
}
