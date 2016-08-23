#ifndef PANELPOSITIONIFACE_H
#define PANELPOSITIONIFACE_H

#include <glib-object.h>
#include <stdbool.h>
#include "toplevel.h"

G_BEGIN_DECLS

G_DECLARE_INTERFACE(ValaPanelPosition,vala_panel_position,VALA_PANEL,POSITION,GObject)

typedef enum
{
    AH_HIDDEN,
    AH_WAITING,
    AH_VISIBLE
} AutohideState;

struct _ValaPanelPositionInterface
{
    GTypeInterface g_iface;
    /*struts*/
    long (*can_strut) (ValaPanelPosition* f, ValaPanelToplevelUnit* top);
    void (*update_strut) (ValaPanelPosition* f, ValaPanelToplevelUnit* top);
    /*autohide*/
    void (*ah_start)(ValaPanelPosition* f,ValaPanelToplevelUnit* top);
    void (*ah_stop)(ValaPanelPosition* f,ValaPanelToplevelUnit* top);
    void (*ah_state_set)(ValaPanelPosition* f,ValaPanelToplevelUnit* top);
    /*positioning requests*/
    void (*move_to_alloc)(ValaPanelPosition* f, ValaPanelToplevelUnit* top, GtkAllocation* alloc);
    gpointer padding [12];
};

long panel_position_can_strut(ValaPanelPosition* f, ValaPanelToplevelUnit* top);
void panel_position_update_strut(ValaPanelPosition* f, ValaPanelToplevelUnit* top);
void panel_position_ah_start(ValaPanelPosition* f,ValaPanelToplevelUnit* top);
void panel_position_ah_stop (ValaPanelPosition* f,ValaPanelToplevelUnit* top);
void panel_position_ah_state_set(ValaPanelPosition* f,ValaPanelToplevelUnit* top);
void panel_position_move_to_alloc(ValaPanelPosition* f, ValaPanelToplevelUnit* top, GtkAllocation* alloc);

G_END_DECLS

#endif // PANELPOSITIONIFACE_H
