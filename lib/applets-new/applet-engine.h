#ifndef APPLETENGINE_H
#define APPLETENGINE_H

#include <glib-object.h>
#include "applet-info.h"

G_DECLARE_DERIVABLE_TYPE(ValaPanelAppletEngine,vala_panel_applet_engine,VALA_PANEL,APPLET_ENGINE,GObject)

struct _ValaPanelAppletEngineClass
{
    GObjectClass parent_class;
    const char* const* (*get_available_types)(ValaPanelAppletEngine* self);
    ValaPanelAppletInfo* (*get_applet_info_for_type)(ValaPanelAppletEngine* self,const char* applet_type);

    gpointer padding [12];
};


#endif // APPLETENGINE_H
