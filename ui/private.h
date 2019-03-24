#ifndef PRIVATE_H
#define PRIVATE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#include "applet-plugin.h"

G_GNUC_INTERNAL ValaPanelApplet *vala_panel_applet_plugin_get_applet_widget(
    ValaPanelAppletPlugin *self, ValaPanelToplevel *top, GSettings *settings, const char *uuid);

/* From toplevel.h */
G_GNUC_INTERNAL ValaPanelPlatform *vala_panel_toplevel_get_current_platform(void);
G_GNUC_INTERNAL ValaPanelCoreSettings *vala_panel_toplevel_get_core_settings(void);
G_GNUC_INTERNAL const char *vala_panel_toplevel_get_uuid(ValaPanelToplevel *self);
G_GNUC_INTERNAL bool vala_panel_toplevel_is_initialized(ValaPanelToplevel *self);
G_GNUC_INTERNAL void vala_panel_toplevel_destroy_pref_dialog(ValaPanelToplevel *self);
G_GNUC_INTERNAL bool vala_panel_toplevel_release_event_helper(GtkWidget *_sender, GdkEventButton *b,
                                                              gpointer obj);

G_END_DECLS

#endif
