#ifndef APPLETWIDGETAPI_H
#define APPLETWIDGETAPI_H

#include "applet-widget.h"
#include "toplevel.h"

/**
 * vala_panel_applet_new_with_type: (constructor)
 * @ex: type of new Applet. It should be a subclass of #ValaPanelApplet
 * @top: #ValaPanelToplevel parent
 * @settings:  #GSettings for these applet type (not generic one)
 * @uuid: UUID for these applet
 * 
 * Returns: (nullable) (transfer full) (type ValaPanelApplet): a new #ValaPaneApplet
 */
gpointer vala_panel_applet_new_with_type(GType ex, ValaPanelToplevel *top, GSettings *settings,
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
