#ifndef PRIVATE_H
#define PRIVATE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#include "applet-plugin.h"

G_GNUC_INTERNAL ValaPanelApplet *vala_panel_applet_plugin_get_applet_widget(
    ValaPanelAppletPlugin *self, ValaPanelToplevel *top, GSettings *settings, const char *uuid);
G_GNUC_INTERNAL ValaPanelPlatform *vala_panel_toplevel_get_current_platform();
G_GNUC_INTERNAL bool vala_panel_toplevel_release_event_helper(GtkWidget *_sender, GdkEventButton *b,
                                                              gpointer obj);

G_END_DECLS

#endif
