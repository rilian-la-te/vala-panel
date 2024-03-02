#ifndef APPLETWIDGETAPI_H
#define APPLETWIDGETAPI_H

#include "applet-widget.h"
#include "toplevel.h"

#define vp_applet_new(t, s, n) vp_applet_construct(VALA_PANEL_TYPE_APPLET, t, s, n)
gpointer vp_applet_construct(GType ex, ValaPanelToplevel *top, GSettings *settings,
                                     const char *uuid);
ValaPanelToplevel *vp_applet_get_toplevel(ValaPanelApplet *self);
const char *vp_get_current_platform_name();

#endif // APPLETWIDGETAPI_H
