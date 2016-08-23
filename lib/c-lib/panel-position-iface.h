#ifndef PANELPOSITIONIFACE_H
#define PANELPOSITIONIFACE_H

#include <glib-object.h>
#include <stdbool.h>
#include "toplevel.h"

G_BEGIN_DECLS

G_DECLARE_INTERFACE(ValaPanelPositionIface,vala_panel_position_iface,VALA_PANEL,POSITION_IFACE,GObject)

typedef enum
{
    AH_HIDDEN,
    AH_WAITING,
    AH_VISIBLE
} AutohideState;

struct _ValaPanelPositionIfaceInterface
{
    GTypeInterface g_iface;
    /*struts*/
    bool (*can_strut) (ValaPanelPositionIface* f, ValaPanelToplevelUnit* top);
    void (*update_strut) (ValaPanelPositionIface* f, ValaPanelToplevelUnit* top);
    /*autohide*/
    bool (*ah_start)(ValaPanelPositionIface* f,ValaPanelToplevelUnit* top);
    bool (*ah_stop)(ValaPanelPositionIface* f,ValaPanelToplevelUnit* top);
    bool (*ah_state_set)(ValaPanelPositionIface* f,ValaPanelToplevelUnit* top);
    gpointer padding [12];
};

G_END_DECLS

#endif // PANELPOSITIONIFACE_H
