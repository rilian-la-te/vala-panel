#ifndef PANELLAYOUT_H
#define PANELLAYOUT_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "applet-manager.h"
#include "toplevel.h"
#include "util.h"

G_BEGIN_DECLS

typedef enum
{
	PACK_START  = 0,
	PACK_CENTER = 2,
	PACK_END    = 1,
} PanelAppletPackType;

G_DECLARE_FINAL_TYPE(ValaPanelAppletLayout, vala_panel_applet_layout, VALA_PANEL, APPLET_LAYOUT,
                     GtkBox)

ValaPanelAppletLayout *vala_panel_applet_layout_new(ValaPanelToplevel *top, GtkOrientation orient,
                                                    gint spacing);
ValaPanelAppletLayout *vala_panel_applet_layout_construct(GType object_type, ValaPanelToplevel *top,
                                                          GtkOrientation orient, gint spacing);
void vala_panel_applet_layout_init_applets(ValaPanelAppletLayout *self);
void vala_panel_applet_layout_add_applet(ValaPanelAppletLayout *self, const gchar *type);
void vala_panel_applet_layout_place_applet(ValaPanelAppletLayout *self, AppletInfoData *data,
                                           ValaPanelUnitSettings *s);
void vala_panel_applet_layout_remove_applet(ValaPanelAppletLayout *self, ValaPanelApplet *applet);
void vala_panel_applet_layout_applet_destroyed(ValaPanelAppletLayout *self, const gchar *uuid);
void vala_panel_applet_layout_update_applet_positions(ValaPanelAppletLayout *self);
GList *vala_panel_applet_layout_get_applets_list(ValaPanelAppletLayout *self);
ValaPanelUnitSettings *vala_panel_applet_layout_get_applet_settings(ValaPanelAppletLayout *self,
                                                                    ValaPanelApplet *pl);
guint vala_panel_applet_layout_get_applet_position(ValaPanelAppletLayout *self,
                                                   ValaPanelApplet *pl);
void vala_panel_applet_layout_set_applet_position(ValaPanelAppletLayout *self, ValaPanelApplet *pl,
                                                  gint pos);
const gchar *vala_panel_applet_layout_get_toplevel_id(ValaPanelAppletLayout *self);

G_END_DECLS

#endif // PANELLAYOUT_H
