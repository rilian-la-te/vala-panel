#ifndef APPLETMANAGER_H
#define APPLETMANAGER_H

#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
G_DECLARE_FINAL_TYPE(ValaPanelAppletManager, vala_panel_applet_manager, VALA_PANEL, APPLET_MANAGER,
                     GObject)

ValaPanelAppletManager *vala_panel_applet_manager_new();

G_END_DECLS

#endif // APPLETMANAGER_H
