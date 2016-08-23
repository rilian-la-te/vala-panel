#include "popup-manager.h"

G_DEFINE_INTERFACE(ValaPanelPopupManager,vala_panel_popup_manager,G_TYPE_OBJECT)

void vala_panel_popup_manager_register_popover(ValaPanelPopupManager *self, GtkWidget *widget, GtkPopover *popover)
{
    if(self)
        VALA_PANEL_POPUP_MANAGER_GET_IFACE(self)->register_popover(self,widget,popover);
}

void vala_panel_popup_manager_register_menu(ValaPanelPopupManager *self, GtkWidget *widget, GMenuModel *menu)
{
    if(self)
        VALA_PANEL_POPUP_MANAGER_GET_IFACE(self)->register_menu(self,widget,menu);
}

void vala_panel_popup_manager_unregister_popover(ValaPanelPopupManager *self, GtkWidget *widget)
{
    if(self)
        VALA_PANEL_POPUP_MANAGER_GET_IFACE(self)->unregister_popover(self,widget);
}

void vala_panel_popup_manager_unregister_menu(ValaPanelPopupManager *self, GtkWidget *widget)
{
    if(self)
        VALA_PANEL_POPUP_MANAGER_GET_IFACE(self)->unregister_menu(self,widget);
}

void vala_panel_popup_manager_show_popover(ValaPanelPopupManager *self, GtkWidget *widget)
{
    if(self)
        VALA_PANEL_POPUP_MANAGER_GET_IFACE(self)->show_popover(self,widget);
}

void vala_panel_popup_manager_show_menu(ValaPanelPopupManager *self, GtkWidget *widget)
{
    if(self)
        VALA_PANEL_POPUP_MANAGER_GET_IFACE(self)->show_menu(self,widget);
}

bool vala_panel_popup_manager_is_registered(ValaPanelPopupManager *self, GtkWidget *widget)
{
    if(self)
        return VALA_PANEL_POPUP_MANAGER_GET_IFACE(self)->is_registered(self,widget);
    return false;
}

void vala_panel_popup_manager_default_init(ValaPanelPopupManagerInterface* iface)
{

}
