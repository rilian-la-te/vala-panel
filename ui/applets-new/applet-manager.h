#ifndef APPLETMANAGER_H
#define APPLETMANAGER_H

#include "applet-plugin.h"
#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
G_DECLARE_FINAL_TYPE(ValaPanelAppletManager, vala_panel_applet_manager, VALA_PANEL, APPLET_MANAGER,
                     GObject)

ValaPanelAppletManager *vala_panel_applet_manager_new();

ValaPanelAppletPlugin *vala_panel_applet_manager_applet_ref(ValaPanelAppletManager *self,
                                                            const char *name);
ValaPanelAppletPlugin *vala_panel_applet_manager_applet_unref(ValaPanelAppletManager *self,
                                                              const char *name);

G_END_DECLS

#endif // APPLETMANAGER_H
