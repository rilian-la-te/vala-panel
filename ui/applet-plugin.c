#include "applet-plugin.h"

G_DEFINE_ABSTRACT_TYPE(ValaPanelAppletPlugin, vala_panel_applet_plugin, G_TYPE_OBJECT)

static void vala_panel_applet_plugin_init(ValaPanelAppletPlugin *self)
{
}

static void vala_panel_applet_plugin_class_init(ValaPanelAppletPluginClass *klass)
{
}

ValaPanelApplet *vala_panel_applet_plugin_get_applet_widget(ValaPanelAppletPlugin *self,
                                                            ValaPanelToplevel *top,
                                                            GSettings *settings, const char *uuid)
{
	if (self)
		return VALA_PANEL_APPLET_PLUGIN_GET_CLASS(self)->get_applet_widget(top,
		                                                                   settings,
		                                                                   uuid);
	return NULL;
}
