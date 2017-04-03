#include "applet-manager.h"
#include "applet-engine-module.h"
#include "lib/applets-new/applet-api.h"
#include "lib/definitions.h"
#include "private.h"

#define PLUGIN_SETTINGS_SCHEMA_BASE "org.valapanel.toplevel.%s"

struct _ValaPanelAppletManager
{
	ValaPanelPlatform *mgr;
	char *profile;
	GHashTable *available_engines;
};

G_DEFINE_TYPE(ValaPanelAppletManager, vala_panel_applet_manager, G_TYPE_OBJECT)

void vala_panel_applet_manager_init(ValaPanelAppletManager *self)
{
	g_autoptr(GError) err = NULL;
	g_autoptr(GDir) dir   = g_dir_open(CONFIG_PLUGINS_DATA, 0, &err);
	for (const char *file = g_dir_read_name(dir); file != NULL; file = g_dir_read_name(dir))
	{
		ValaPanelAppletEngineModule *module =
		    vala_panel_applet_engine_module_new_from_ini(file);
		g_hash_table_add(self->available_engines, module);
	}
}

void vala_panel_applet_manager_finalize(GObject *obj)
{
	ValaPanelAppletManager *module = VALA_PANEL_APPLET_MANAGER(obj);

	g_free(module->profile);

	(*G_OBJECT_CLASS(vala_panel_applet_manager_parent_class)->finalize)(obj);
}

void vala_panel_applet_manager_class_init(ValaPanelAppletManagerClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->finalize     = vala_panel_applet_manager_finalize;
}

GSList *vala_panel_applet_manager_get_available_types(ValaPanelAppletManager *self)
{
	GSList *available_types = NULL;
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, self->available_engines);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		GSList *engine_types =
		    vala_panel_applet_engine_get_available_types((ValaPanelAppletEngine *)key);
		available_types = g_slist_concat(available_types, engine_types);
	}
	return available_types;
}

ValaPanelAppletInfo *vala_panel_applet_manager_get_applet_info_for_type(
    ValaPanelAppletManager *self, const char *applet_type)
{
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, self->available_engines);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		ValaPanelAppletInfo *info =
		    vala_panel_applet_engine_get_applet_info_for_type((ValaPanelAppletEngine *)key,
		                                                      applet_type);
		if (info)
			return info;
	}
	return NULL;
}

ValaPanelAppletWidget *vala_panel_applet_manager_get_applet_widget_for_type(
    ValaPanelAppletManager *self, const char *path, const char *applet_type, const char *uuid)
{
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, self->available_engines);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		g_autofree char *scheme = g_strdup_printf(PLUGIN_SETTINGS_SCHEMA_BASE, applet_type);
		g_autofree char *cpath  = g_strconcat(path, uuid, "/", NULL);
		GSettings *settings =
		    vala_panel_platform_get_settings_for_scheme(self->mgr, scheme, cpath);
		ValaPanelAppletWidget *widget =
		    vala_panel_applet_engine_get_applet_widget_for_type((ValaPanelAppletEngine *)
		                                                            key,
		                                                        applet_type,
		                                                        settings,
		                                                        uuid);
		if (widget)
			return widget;
	}
	return NULL;
}

ValaPanelAppletManager *vala_panel_applet_manager_new(ValaPanelPlatform *mgr)
{
	ValaPanelAppletManager *ret =
	    VALA_PANEL_APPLET_MANAGER(g_object_new(vala_panel_applet_manager_get_type(), NULL));
	ret->mgr = mgr;
	return ret;
}

ValaPanelPlatform *vala_panel_applet_manager_get_manager(ValaPanelAppletManager *self)
{
	return self->mgr;
}
