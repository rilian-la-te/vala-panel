#ifndef APPLETWIDGETAPI_H
#define APPLETWIDGETAPI_H

#include "applet-widget.h"
#include "toplevel.h"

#define vala_panel_applet_new(t, s, n) vala_panel_applet_construct(VALA_PANEL_TYPE_APPLET, t, s, n)
gpointer vala_panel_applet_construct(GType ex, ValaPanelToplevel *top, GSettings *settings,
                                     const char *uuid);

/**
 * vala_panel_applet_get_toplevel: (get-property toplevel)
 * @self: #ValaPanelApplet for which you need a toplevel
 * 
 * Returns: (nullable) (transfer none): #ValaPaneToplevel, for which applet belongs
 */
ValaPanelToplevel *vala_panel_applet_get_toplevel(ValaPanelApplet *self);
const char *vala_panel_get_current_platform_name();

#endif // APPLETWIDGETAPI_H
