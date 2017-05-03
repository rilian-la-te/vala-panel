#ifndef APPLETWIDGETAPI_H
#define APPLETWIDGETAPI_H

#include "applet-widget.h"
#include "vala-panel-compat.h"

ValaPanelToplevel *vala_panel_applet_new(ValaPanelToplevel *top, GSettings *settings,
                                         const char *uuid);
#define vala_panel_applet_construct(type, t, s, n) vala_panel_applet_new(t, s, n)
ValaPanelToplevel *vala_panel_applet_get_toplevel(ValaPanelApplet *self);

#endif // APPLETWIDGETAPI_H
