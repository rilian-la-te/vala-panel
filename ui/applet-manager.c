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
#include "definitions.h"

static AppletInfoData *applet_info_data_new(ValaPanelAppletInfo *info, GType plugin_type)
{
	AppletInfoData *ret = (AppletInfoData *)g_slice_alloc0(sizeof(AppletInfoData));
	ret->info           = info;
	ret->applet_type    = plugin_type;
	ret->count          = 0;
	return ret;
}

static void applet_info_data_free(void *adata)
{
	AppletInfoData *data = (AppletInfoData *)adata;
	g_clear_pointer(&data->info, vala_panel_applet_info_free);
	g_slice_free(AppletInfoData, data);
}

GIOExtensionPoint *applet_point = NULL;

struct _ValaPanelAppletManager
{
	GObject parent;
	GHashTable *applet_info_table;
	GIOModuleScope *scope;
};

G_DEFINE_TYPE(ValaPanelAppletManager, vp_applet_manager, G_TYPE_OBJECT)

static AppletInfoData *vp_applet_manager_applet_ref(ValaPanelAppletManager *self, const char *name);

void vp_applet_manager_reload_applets(ValaPanelAppletManager *self)
{
	g_io_modules_scan_all_in_directory_with_scope(PLUGINS_DIRECTORY, self->scope);
	GList *loaded_applets = g_io_extension_point_get_extensions(applet_point);
	for (GList *i = loaded_applets; i != NULL; i = g_list_next(i))
	{
		const char *module_name = g_io_extension_get_name((GIOExtension *)i->data);
		if (!g_hash_table_contains(self->applet_info_table, module_name))
		{
			ValaPanelAppletInfo *info = vala_panel_applet_info_load(module_name);
			GType plugin_type    = g_io_extension_get_type((GIOExtension *)i->data);
			AppletInfoData *data = applet_info_data_new(info, plugin_type);
			if (info != NULL && plugin_type != G_TYPE_NONE)
			{
				g_hash_table_insert(self->applet_info_table,
				                    g_strdup(module_name),
				                    data);
			}
			else
			{
				if (info)
					vala_panel_applet_info_free(info);
				applet_info_data_free(data);
			}
		}
	}
}

static AppletInfoData *vp_applet_manager_applet_ref(ValaPanelAppletManager *self, const char *name)
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

void vp_applet_manager_applet_unref(ValaPanelAppletManager *self, const char *name)
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

G_GNUC_INTERNAL ValaPanelApplet *vp_applet_manager_get_applet_widget(ValaPanelAppletManager *self,
                                                                     const char *type,
                                                                     ValaPanelToplevel *top,
                                                                     ValaPanelUnitSettings *s)
{
	const char *uuid           = s->uuid;
	GSettings *applet_settings = s->custom;
	AppletInfoData *data       = vp_applet_manager_applet_ref(self, type);
	if (g_type_is_a(data->applet_type, VALA_PANEL_TYPE_APPLET))
	{
		ValaPanelApplet *applet =
		    vala_panel_applet_construct(data->applet_type, top, applet_settings, uuid);
		if (VALA_PANEL_IS_APPLET(applet))
			return applet;
	}
	else if (g_type_is_a(data->applet_type, VALA_PANEL_TYPE_APPLET_PLUGIN))
	{
		g_autoptr(ValaPanelAppletPlugin) pl =
		    VALA_PANEL_APPLET_PLUGIN(g_object_new(data->applet_type, NULL));
		if (VALA_PANEL_IS_APPLET_PLUGIN(pl))
			return VALA_PANEL_APPLET_PLUGIN_GET_CLASS(pl)
			    ->get_applet_widget(pl, top, applet_settings, uuid);
	}
	return NULL;
}

static AppletInfoData *vp_applet_manager_get_info_data(ValaPanelAppletManager *self,
                                                       ValaPanelApplet *pl,
                                                       ValaPanelCoreSettings *core_settings)
{
	ValaPanelUnitSettings *settings =
	    vp_core_settings_get_by_uuid(core_settings, vala_panel_applet_get_uuid(pl));
	g_autofree char *str = g_settings_get_string(settings->common, VP_KEY_NAME);
	return (AppletInfoData *)g_hash_table_lookup(self->applet_info_table, str);
}

ValaPanelAppletInfo *vp_applet_manager_get_applet_info(ValaPanelAppletManager *self,
                                                       ValaPanelApplet *pl,
                                                       ValaPanelCoreSettings *core_settings)
{
	AppletInfoData *data = vp_applet_manager_get_info_data(self, pl, core_settings);
	return data->info;
}

GList *vp_applet_manager_get_all_types(ValaPanelAppletManager *self)
{
	return g_hash_table_get_values(self->applet_info_table);
}

static void vp_applet_manager_finalize(GObject *data)
{
	ValaPanelAppletManager *self = VP_APPLET_MANAGER(data);
	g_hash_table_unref(self->applet_info_table);
	g_io_module_scope_free(self->scope);
	G_OBJECT_CLASS(vp_applet_manager_parent_class)->finalize(data);
}

static void vp_applet_manager_init(ValaPanelAppletManager *self)
{
	self->applet_info_table =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, applet_info_data_free);
	self->scope = g_io_module_scope_new(G_IO_MODULE_SCOPE_BLOCK_DUPLICATES);
	vp_applet_manager_reload_applets(self);
}

static void vp_applet_manager_class_init(ValaPanelAppletManagerClass *klass)
{
	applet_point = g_io_extension_point_register(VALA_PANEL_APPLET_EXTENSION_POINT);
	g_io_extension_point_set_required_type(applet_point, G_TYPE_OBJECT);
	G_OBJECT_CLASS(klass)->finalize = vp_applet_manager_finalize;
}

ValaPanelAppletManager *vp_applet_manager_new()
{
	return VP_APPLET_MANAGER(g_object_new(vp_applet_manager_get_type(), NULL));
}
