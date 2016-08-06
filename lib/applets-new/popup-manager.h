#ifndef POPUPMANAGER_H
#define POPUPMANAGER_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
G_DECLARE_INTERFACE(ValaPanelPopupManager, vala_panel_popup_manager, VALA_PANEL,
                    POPUP_MANAGER, GObject)

struct _ValaPanelPopupManager
{
  GTypeInterface g_iface;
  void (*finalize)(ValaPanelPopupManager* self);
  void (*register_popover)(ValaPanelPopupManager* self, GtkWidget* widget,
                           GtkPopover* popover);
  void (*register_menu)(ValaPanelPopupManager* self, GtkWidget* widget,
                        GMenuModel* menu);
  void (*unregister_popover)(ValaPanelPopupManager* self, GtkWidget* widget);
  void (*unregister_menu)(ValaPanelPopupManager* self, GtkWidget* widget);
  void (*show_popover)(ValaPanelPopupManager* self, GtkWidget* widget);
  void (*show_menu)(ValaPanelPopupManager* self, GtkWidget* widget);
  gboolean (*popover_is_registered)(ValaPanelPopupManager* self,
                                    GtkWidget* widget);
  gboolean (*menu_is_registered)(ValaPanelPopupManager* self,
                                 GtkWidget* widget);
};
G_END_DECLS

#endif // POPUPMANAGER_H
