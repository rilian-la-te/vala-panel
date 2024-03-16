#ifndef APPLETWIDGETAPI_H
#define APPLETWIDGETAPI_H

#include "applet-widget.h"
#include "toplevel.h"

/**
 * vala_panel_applet_get_toplevel: (get-property toplevel)
 * @self: #ValaPanelApplet for which you need a toplevel
 * 
 * Returns: (nullable) (transfer none): #ValaPaneToplevel, for which applet belongs
 */
ValaPanelToplevel *vala_panel_applet_get_toplevel(ValaPanelApplet *self);
const char *vala_panel_get_current_platform_name();

#endif // APPLETWIDGETAPI_H
