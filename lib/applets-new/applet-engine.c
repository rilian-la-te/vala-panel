#include "applet-engine.h"

G_DEFINE_INTERFACE(ValaPanelAppletEngine, vala_panel_applet_engine, G_TYPE_OBJECT)

GSList *vala_panel_applet_engine_get_available_types(ValaPanelAppletEngine *self)
{
	if (self)
		return VALA_PANEL_APPLET_ENGINE_GET_IFACE(self)->get_available_types(self);
	return NULL;
}

ValaPanelAppletInfo *vala_panel_applet_engine_get_applet_info_for_type(ValaPanelAppletEngine *self,
                                                                       const char *applet_type)
{
	if (self && g_slist_find(vala_panel_applet_engine_get_available_types(self), applet_type))
		return VALA_PANEL_APPLET_ENGINE_GET_IFACE(self)
		    ->get_applet_info_for_type(self, applet_type);
	return NULL;
}

ValaPanelAppletWidget *vala_panel_applet_engine_get_applet_widget_for_type(
    ValaPanelAppletEngine *self, const char *applet_type, GSettings *settings, const char *uuid)
{
	if (self && g_slist_find(vala_panel_applet_engine_get_available_types(self), applet_type))
		return VALA_PANEL_APPLET_ENGINE_GET_IFACE(self)
		    ->get_applet_widget_for_type(self, applet_type, settings, uuid);
	return NULL;
}

bool vala_panel_applet_engine_contains_applet(ValaPanelAppletEngine *self, const char *applet_type)
{
	return g_slist_find(vala_panel_applet_engine_get_available_types(self), applet_type) !=
	       NULL;
}

void vala_panel_applet_engine_default_init(ValaPanelAppletEngineInterface *iface)
{
}
