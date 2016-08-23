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
    bool (*can_strut) (ValaPanelPosition* f, ValaPanelToplevelUnit* top);
    void (*update_strut) (ValaPanelPosition* f, ValaPanelToplevelUnit* top);
    /*autohide*/
    bool (*ah_start)(ValaPanelPosition* f,ValaPanelToplevelUnit* top);
    bool (*ah_stop)(ValaPanelPosition* f,ValaPanelToplevelUnit* top);
    bool (*ah_state_set)(ValaPanelPosition* f,ValaPanelToplevelUnit* top);
    gpointer padding [12];
};



G_END_DECLS

#endif // PANELPOSITIONIFACE_H
