#include "applet-engine.h"


const char * const *vala_panel_applet_engine_get_available_types(ValaPanelAppletEngine *self)
{
    if(self)
        return VALA_PANEL_APPLET_ENGINE_GET_IFACE(self)->get_available_types(self);
    return NULL;
}

ValaPanelAppletInfo *vala_panel_applet_engine_get_applet_info_for_type(ValaPanelAppletEngine *self, const char *applet_type)
{
    if(self && g_strv_contains(vala_panel_applet_engine_get_available_types(self),applet_type))
        return VALA_PANEL_APPLET_ENGINE_GET_IFACE(self)->get_applet_info_for_type(self,applet_type);
    return NULL;
}

ValaPanelAppletWidget *vala_panel_applet_engine_get_applet_widget_for_type(ValaPanelAppletEngine *self, const char *applet_type, const char *uuid, const char *scheme, const char *path, const char *filename)
{
    if(self && g_strv_contains(vala_panel_applet_engine_get_available_types(self),applet_type))
        return VALA_PANEL_APPLET_ENGINE_GET_IFACE(self)->get_applet_widget_for_type(self,applet_type,uuid,scheme,path,filename);
    return NULL;
}
