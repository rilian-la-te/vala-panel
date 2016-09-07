#ifndef APPLETENGINE_H
#define APPLETENGINE_H

#include "applet-info.h"
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

G_DECLARE_INTERFACE(ValaPanelAppletEngine, vala_panel_applet_engine, VALA_PANEL, APPLET_ENGINE,
                    GObject)

struct _ValaPanelAppletEngineInterface
{
	GTypeInterface g_iface;
	GSList *(*get_available_types)(ValaPanelAppletEngine *self);
	ValaPanelAppletInfo *(*get_applet_info_for_type)(ValaPanelAppletEngine *self,
	                                                 const char *applet_type);
	ValaPanelAppletWidget *(*get_applet_widget_for_type)(ValaPanelAppletEngine *self,
	                                                     const char *applet_type,
	                                                     GSettings *settings, const char *uuid);
	gpointer padding[12];
};

G_END_DECLS

#endif // APPLETENGINE_H
