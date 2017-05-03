#ifndef APPLETSPRIVATE_H
#define APPLETSPRIVATE_H
#include "lib/applets-new/applet-api.h"

GSList *vala_panel_applet_engine_iface_get_available_types(ValaPanelAppletEngineIface *self);
ValaPanelAppletInfo *vala_panel_applet_engine_iface_get_applet_info_for_type(
    ValaPanelAppletEngineIface *self, const char *applet_type);
ValaPanelApplet *vala_panel_applet_engine_iface_get_applet_widget_for_type(
    ValaPanelAppletEngineIface *self, const char *applet_type, GSettings *settings,
    const char *uuid);
bool vala_panel_applet_engine_iface_contains_applet(ValaPanelAppletEngineIface *self,
                                                    const char *applet_type);

#endif // APPLETSPRIVATE_H
