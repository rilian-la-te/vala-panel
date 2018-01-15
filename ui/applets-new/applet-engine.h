#ifndef APPLETENGINE_H
#define APPLETENGINE_H

#include "applet-info.h"
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

G_DECLARE_INTERFACE(ValaPanelAppletEngineIface, vala_panel_applet_engine_iface_iface, VALA_PANEL,
                    APPLET_ENGINE_IFACE, GObject)

struct _ValaPanelAppletEngineIfaceInterface
{
	GTypeInterface g_iface;
	GSList *(*get_available_types)(ValaPanelAppletEngineIface *self);
	ValaPanelAppletInfo *(*get_applet_info_for_type)(ValaPanelAppletEngineIface *self,
	                                                 const char *applet_type);
	ValaPanelApplet *(*get_applet_widget_for_type)(ValaPanelAppletEngineIface *self,
	                                               const char *applet_type, GSettings *settings,
	                                               const char *uuid);
	gpointer padding[12];
};

G_END_DECLS

#endif // APPLETENGINE_H
