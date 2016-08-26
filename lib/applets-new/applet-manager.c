#include "applet-manager.h"
#include "applet-engine.h"
#include "applet-info.h"

struct _ValaPanelAppletManager
{

};

G_DEFINE_TYPE(ValaPanelAppletManager,vala_panel_applet_manager,G_TYPE_OBJECT)



void vala_panel_applet_manager_init(ValaPanelAppletManager* self)
{

}

void vala_panel_applet_manager_class_init(ValaPanelAppletManagerClass* klass)
{

}

char** vala_panel_applet_manager_get_available_types(ValaPanelAppletManager *self)
{

}

ValaPanelAppletInfo *vala_panel_applet_manager_get_applet_info_for_type(ValaPanelAppletManager *self, const char *applet_type)
{

}

ValaPanelAppletWidget *vala_panel_applet_manager_get_applet_widget_for_type(ValaPanelAppletManager *self, const char *applet_type, const char *uuid)
{

}
