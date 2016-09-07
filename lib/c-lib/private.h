#ifndef APPLETSPRIVATE_H
#define APPLETSPRIVATE_H
#include "lib/applets-new/applet-api.h"

G_GNUC_INTERNAL char **vala_panel_applet_engine_get_available_types(ValaPanelAppletEngine *self);
G_GNUC_INTERNAL ValaPanelAppletInfo *vala_panel_applet_engine_get_applet_info_for_type(
    ValaPanelAppletEngine *self, const char *applet_type);
G_GNUC_INTERNAL ValaPanelAppletWidget *vala_panel_applet_engine_get_applet_widget_for_type(
    ValaPanelAppletEngine *self, const char *applet_type, GSettings *settings, const char *uuid);

#endif // APPLETSPRIVATE_H
