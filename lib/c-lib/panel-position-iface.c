#include "panel-position-iface.h"

G_DEFINE_INTERFACE(ValaPanelPosition,vala_panel_position,G_TYPE_OBJECT)

void vala_panel_position_default_init(ValaPanelPositionInterface* self)
{

}

long panel_position_can_strut(ValaPanelPosition *self, ValaPanelToplevelUnit *top)
{
    if(self)
        return VALA_PANEL_POSITION_GET_IFACE(self)->can_strut(self,top);
    return -1;
}

void panel_position_update_strut(ValaPanelPosition *self, ValaPanelToplevelUnit *top)
{
    if(self)
        VALA_PANEL_POSITION_GET_IFACE(self)->update_strut(self,top);
}

void panel_position_ah_start(ValaPanelPosition *self, ValaPanelToplevelUnit *top)
{
    if(self)
        VALA_PANEL_POSITION_GET_IFACE(self)->ah_start(self,top);
}

void panel_position_ah_stop(ValaPanelPosition *self, ValaPanelToplevelUnit *top)
{
    if(self)
        VALA_PANEL_POSITION_GET_IFACE(self)->ah_stop(self,top);
}

void panel_position_ah_state_set(ValaPanelPosition *self, ValaPanelToplevelUnit *top)
{
    if(self)
        VALA_PANEL_POSITION_GET_IFACE(self)->ah_state_set(self,top);
}

void panel_position_move_to_alloc(ValaPanelPosition *self, ValaPanelToplevelUnit *top, GtkAllocation *alloc)
{
    if(self)
        VALA_PANEL_POSITION_GET_IFACE(self)->move_to_alloc(self,top,alloc);
}
