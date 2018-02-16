#ifndef APPLETWIDGETAPI_H
#define APPLETWIDGETAPI_H

#include "applet-widget.h"
#include "toplevel.h"

#define vala_panel_applet_new(t, s, n) vala_panel_applet_construct(VALA_PANEL_TYPE_APPLET, t, s, n)
gpointer vala_panel_applet_construct(GType ex, ValaPanelToplevel *top, GSettings *settings,
                                     const char *uuid);
ValaPanelToplevel *vala_panel_applet_get_toplevel(ValaPanelApplet *self);

#endif // APPLETWIDGETAPI_H
