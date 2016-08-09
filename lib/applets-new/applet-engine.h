#ifndef APPLETENGINE_H
#define APPLETENGINE_H

#include <glib-object.h>
#include "applet-info.h"

G_DECLARE_INTERFACE(ValaPanelAppletEngine,vala_panel_applet_engine,VALA_PANEL,APPLET_ENGINE,GObject)

struct _ValaPanelAppletEngineInterface
{
    GObjectClass parent_class;
    const char* const* (*get_available_types)(ValaPanelAppletEngine* self);
    ValaPanelAppletInfo* (*get_applet_info_for_type)(ValaPanelAppletEngine* self,const char* applet_type);
    ValaPanelAppletWidget* (*get_applet_widget_for_type)(ValaPanelAppletEngine* self,const char* applet_type,const char* uuid, const char* scheme, const char* path, const char* filename);
    gpointer padding [12];
};

const char* const* vala_panel_applet_engine_get_available_types(ValaPanelAppletEngine* self);
ValaPanelAppletInfo* vala_panel_applet_engine_get_applet_info_for_type(ValaPanelAppletEngine* self,const char* applet_type);
ValaPanelAppletWidget* vala_panel_applet_engine_get_applet_widget_for_type(ValaPanelAppletEngine* self,const char* applet_type,const char* uuid, const char* scheme, const char* path, const char* filename);

#endif // APPLETENGINE_H
