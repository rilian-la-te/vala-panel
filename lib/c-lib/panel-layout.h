#ifndef PANELLAYOUT_H
#define PANELLAYOUT_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VALA_PANEL_PLUGIN_SCHEMA "org.valapanel.toplevel.plugin"

#define VALA_PANEL_KEY_NAME "plugin-type"
#define VALA_PANEL_KEY_EXPAND "is-expanded"
#define VALA_PANEL_KEY_CAN_EXPAND "can-expand"
#define VALA_PANEL_KEY_PACK "pack-type"
#define VALA_PANEL_KEY_POSITION "position"

G_DECLARE_FINAL_TYPE(ValaPanelAppletLayout, vala_panel_applet_layout, VALA_PANEL, APPLET_LAYOUT,
                     GtkBox)

G_END_DECLS

#endif // PANELLAYOUT_H
