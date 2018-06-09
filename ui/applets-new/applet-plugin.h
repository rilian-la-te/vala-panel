#ifndef APPLETPLUGIN_H
#define APPLETPLUGIN_H

#include "applet-widget-api.h"
#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VALA_PANEL_APPLET_EXPANDABLE "ValaPanel-Expandable"
#define VALA_PANEL_APPLET_EXCLUSIVE "ValaPanel-Exclusive"
#define VALA_PANEL_APPLET_EXTENSION_POINT "vala-panel-applet-module"

G_DECLARE_DERIVABLE_TYPE(ValaPanelAppletPlugin, vala_panel_applet_plugin, VALA_PANEL, APPLET_PLUGIN,
                         GObject)

struct _ValaPanelAppletPluginClass
{
	ValaPanelApplet *(*get_applet_widget)(ValaPanelToplevel *top, GSettings *settings,
	                                      const char *uuid);
};

G_END_DECLS

#endif // APPLETPLUGIN_H