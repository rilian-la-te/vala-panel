#ifndef PANELLAYOUT_H
#define PANELLAYOUT_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "applet-manager.h"
#include "misc.h"

G_BEGIN_DECLS
#define VALA_PANEL_PLUGIN_SCHEMA "org.valapanel.toplevel.plugin"

#define VALA_PANEL_KEY_NAME "plugin-type"
#define VALA_PANEL_KEY_EXPAND "is-expanded"
#define VALA_PANEL_KEY_CAN_EXPAND "can-expand"
#define VALA_PANEL_KEY_PACK "pack-type"
#define VALA_PANEL_KEY_POSITION "position"

typedef enum {
	PACK_START  = 0,
	PACK_CENTER = 2,
	PACK_END    = 1,
} PanelAppletPackType;

#define vala_panel_applet_set_position_metadata(applet, pos)                                       \
	g_object_set_qdata(G_OBJECT(applet),                                                       \
	                   g_quark_from_static_string("position"),                                 \
	                   GINT_TO_POINTER(pos));

#define vala_panel_applet_get_position_metadata(applet)                                            \
	GPOINTER_TO_INT(                                                                           \
	    g_object_get_qdata(G_OBJECT(applet), g_quark_from_static_string("position")));

G_DECLARE_FINAL_TYPE(ValaPanelAppletLayout, vala_panel_applet_layout, VALA_PANEL, APPLET_LAYOUT,
                     GtkBox)

ValaPanelAppletLayout *vala_panel_applet_layout_new(GtkOrientation orient, int spacing);
void vala_panel_applet_layout_update_views(ValaPanelAppletLayout *self);
void vala_panel_applet_layout_place_applet(ValaPanelAppletLayout *self, ValaPanelPlatform *gmgr,
                                           GSettings *toplevel_settings,
                                           ValaPanelAppletManager *mgr, const char *applet_type,
                                           PanelAppletPackType pack, int pos);
void vala_panel_applet_layout_load_applets(ValaPanelAppletLayout *self, ValaPanelAppletManager *mgr,
                                           GSettings *settings);

G_END_DECLS

#endif // PANELLAYOUT_H
