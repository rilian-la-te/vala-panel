#ifndef PRIVATE_H
#define PRIVATE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#include "applet-manager.h"

/* From toplevel.h */
G_GNUC_INTERNAL ValaPanelPlatform *vp_toplevel_get_current_platform(void);
G_GNUC_INTERNAL ValaPanelCoreSettings *vp_toplevel_get_core_settings(void);
G_GNUC_INTERNAL VPManager *vp_toplevel_get_manager(void);
G_GNUC_INTERNAL const char *vp_toplevel_get_uuid(ValaPanelToplevel *self);
G_GNUC_INTERNAL bool vp_toplevel_is_initialized(ValaPanelToplevel *self);
G_GNUC_INTERNAL void vp_toplevel_destroy_pref_dialog(ValaPanelToplevel *self);
G_GNUC_INTERNAL bool vp_toplevel_release_event_helper(GtkWidget *_sender, GdkEventButton *b,
                                                      gpointer obj);

/* From platform.h */
G_GNUC_INTERNAL VPManager *vp_platform_get_manager(ValaPanelPlatform *self);

G_END_DECLS

#endif
