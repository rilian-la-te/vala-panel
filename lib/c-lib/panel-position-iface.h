#ifndef PANELPOSITIONIFACE_H
#define PANELPOSITIONIFACE_H

#include <glib-object.h>
#include "toplevel.h"

G_BEGIN_DECLS

G_DECLARE_INTERFACE(ValaPanelPositionIface,vala_panel_position_iface,VALA_PANEL,POSITION_IFACE,GObject)

struct _ValaPanelPositionIfaceInterface
{
    GTypeInterface g_iface;
    void (*update_strut) (ValaPanelToplevelUnit* top);
    gpointer padding [12];
};

G_END_DECLS

#endif // PANELPOSITIONIFACE_H
