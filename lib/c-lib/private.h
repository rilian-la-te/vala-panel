#ifndef APPLETSPRIVATE_H
#define APPLETSPRIVATE_H
#include "lib/applets-new/applet-api.h"

GSList *vala_panel_applet_engine_get_available_types(ValaPanelAppletEngine *self);
ValaPanelAppletInfo *vala_panel_applet_engine_get_applet_info_for_type(ValaPanelAppletEngine *self,
                                                                       const char *applet_type);
ValaPanelAppletWidget *vala_panel_applet_engine_get_applet_widget_for_type(
    ValaPanelAppletEngine *self, const char *applet_type, GSettings *settings, const char *uuid);
bool vala_panel_applet_engine_contains_applet(ValaPanelAppletEngine *self, const char *applet_type);

#endif // APPLETSPRIVATE_H
