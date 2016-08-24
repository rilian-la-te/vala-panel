#ifndef POPUPMANAGER_H
#define POPUPMANAGER_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS
G_DECLARE_INTERFACE(ValaPanelPopupManager, vala_panel_popup_manager, VALA_PANEL, POPUP_MANAGER,
                    GObject)

struct _ValaPanelPopupManagerInterface {
        GTypeInterface g_iface;
        void (*register_popover)(ValaPanelPopupManager *self, GtkWidget *widget,
                                 GtkPopover *popover);
        void (*register_menu)(ValaPanelPopupManager *self, GtkWidget *widget, GMenuModel *menu);
        void (*unregister_popover)(ValaPanelPopupManager *self, GtkWidget *widget);
        void (*unregister_menu)(ValaPanelPopupManager *self, GtkWidget *widget);
        void (*show_popover)(ValaPanelPopupManager *self, GtkWidget *widget);
        void (*show_menu)(ValaPanelPopupManager *self, GtkWidget *widget);
        bool (*is_registered)(ValaPanelPopupManager *self, GtkWidget *widget);
        gpointer padding[12];
};

void vala_panel_popup_manager_register_popover(ValaPanelPopupManager *self, GtkWidget *widget,
                                               GtkPopover *popover);
void vala_panel_popup_manager_register_menu(ValaPanelPopupManager *self, GtkWidget *widget,
                                            GMenuModel *menu);
void vala_panel_popup_manager_unregister_popover(ValaPanelPopupManager *self, GtkWidget *widget);
void vala_panel_popup_manager_unregister_menu(ValaPanelPopupManager *self, GtkWidget *widget);
void vala_panel_popup_manager_show_popover(ValaPanelPopupManager *self, GtkWidget *widget);
void vala_panel_popup_manager_show_menu(ValaPanelPopupManager *self, GtkWidget *widget);
bool vala_panel_popup_manager_is_registered(ValaPanelPopupManager *self, GtkWidget *widget);
G_END_DECLS

#endif // POPUPMANAGER_H
