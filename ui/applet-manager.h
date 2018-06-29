#ifndef APPLETMANAGER_H
#define APPLETMANAGER_H

#include "applet-info.h"
#include "applet-plugin.h"
#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
G_DECLARE_FINAL_TYPE(ValaPanelAppletManager, vala_panel_applet_manager, VALA_PANEL, APPLET_MANAGER,
                     GObject)

typedef struct
{
	ValaPanelAppletInfo *info;
	ValaPanelAppletPlugin *plugin;
	uint count;
} AppletInfoData;

ValaPanelAppletManager *vala_panel_applet_manager_new();

AppletInfoData *vala_panel_applet_manager_applet_ref(ValaPanelAppletManager *self,
                                                     const char *name);
void vala_panel_applet_manager_applet_unref(ValaPanelAppletManager *self, const char *name);
ValaPanelAppletPlugin *vala_panel_applet_manager_get_plugin(ValaPanelAppletManager *self,
                                                            ValaPanelApplet *pl,
                                                            ValaPanelCoreSettings *core_settings);
ValaPanelAppletInfo *vala_panel_applet_manager_get_plugin_info(
    ValaPanelAppletManager *self, ValaPanelApplet *pl, ValaPanelCoreSettings *core_settings);
GList *vala_panel_applet_manager_get_all_types(ValaPanelAppletManager *self);

G_END_DECLS

#endif // APPLETMANAGER_H
