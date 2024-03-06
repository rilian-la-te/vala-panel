#ifndef PRIVATE_H
#define PRIVATE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#include "applet-manager.h"
#include "panel-layout.h"

/* From toplevel.h */
G_GNUC_INTERNAL ValaPanelPlatform *vp_toplevel_get_current_platform(void);
G_GNUC_INTERNAL ValaPanelCoreSettings *vp_toplevel_get_core_settings(void);
G_GNUC_INTERNAL ValaPanelAppletManager *vp_toplevel_get_manager(void);
G_GNUC_INTERNAL const char *vp_toplevel_get_uuid(ValaPanelToplevel *self);
G_GNUC_INTERNAL bool vp_toplevel_is_initialized(ValaPanelToplevel *self);
G_GNUC_INTERNAL void vp_toplevel_destroy_pref_dialog(ValaPanelToplevel *self);
G_GNUC_INTERNAL bool vp_toplevel_release_event_helper(GtkWidget *_sender, GdkEventButton *b,
                                                      gpointer obj);

/* From platform.h */
G_GNUC_INTERNAL ValaPanelAppletManager *vp_platform_get_manager(ValaPanelPlatform *self);

/* From panel-layout.h */
G_GNUC_INTERNAL ValaPanelLayout *vp_layout_new(ValaPanelToplevel *top, GtkOrientation orient,
                                               gint spacing);
G_GNUC_INTERNAL void vp_layout_init_applets(ValaPanelLayout *self);
G_GNUC_INTERNAL ValaPanelApplet *vp_layout_insert_applet(ValaPanelLayout *self, const char *type,
                                                         ValaPanelAppletPackType pack, uint pos);
G_GNUC_INTERNAL void vp_layout_remove_applet(ValaPanelLayout *self, ValaPanelApplet *applet);
G_GNUC_INTERNAL void vp_layout_update_applet_positions(ValaPanelLayout *self);
G_GNUC_INTERNAL bool vp_layout_can_move_to_direction(ValaPanelLayout *self, ValaPanelApplet *prev,
                                                     ValaPanelApplet *next, GtkPackType direction);
G_GNUC_INTERNAL void vp_layout_move_applet_one_step(ValaPanelLayout *self, ValaPanelApplet *prev,
                                                    ValaPanelApplet *next, GtkPackType direction);
G_GNUC_INTERNAL ValaPanelUnitSettings *vp_layout_get_applet_settings(ValaPanelApplet *pl);
G_GNUC_INTERNAL ValaPanelAppletPackType vp_layout_get_applet_pack_type(ValaPanelApplet *pl);
G_GNUC_INTERNAL unsigned int vp_layout_get_applet_position(ValaPanelLayout *self,
                                                           ValaPanelApplet *pl);
G_GNUC_INTERNAL ValaPanelAppletManager *vp_layout_get_manager(void);

/* From settings-manager.h */
#define vp_core_settings_remove_unit_settings(s, n)                                                \
	vp_core_settings_remove_unit_settings_full(s, n, false)

G_GNUC_INTERNAL ValaPanelUnitSettings *vp_core_settings_add_unit_settings(
    ValaPanelCoreSettings *settings, const char *name, bool is_toplevel);
G_GNUC_INTERNAL ValaPanelUnitSettings *vp_core_settings_add_unit_settings_full(
    ValaPanelCoreSettings *settings, const char *name, const char *uuid, bool is_toplevel);
G_GNUC_INTERNAL void vp_core_settings_remove_unit_settings_full(ValaPanelCoreSettings *settings,
                                                                const char *name, bool destroy);
G_GNUC_INTERNAL ValaPanelUnitSettings *vp_core_settings_get_by_uuid(ValaPanelCoreSettings *settings,
                                                                    const char *uuid);
G_GNUC_INTERNAL char *vp_core_settings_get_uuid(void);
G_GNUC_INTERNAL bool vp_core_settings_init_unit_list(ValaPanelCoreSettings *settings);

G_END_DECLS

#endif
