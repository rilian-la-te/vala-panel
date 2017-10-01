#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "applet-widget.h"
#include "constants.h"
#include "panel-platform.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelToplevel, vala_panel_toplevel, VALA_PANEL, TOPLEVEL,
                     GtkApplicationWindow)

#define VALA_PANEL_TYPE_TOPLEVEL vala_panel_toplevel_get_type()

ValaPanelCoreSettings *vala_panel_toplevel_get_core_settings();

ValaPanelToplevel *vala_panel_toplevel_new(GtkApplication *app, ValaPanelPlatform *plt,
                                           const char *uid);

#define vala_panel_toplevel_get_layout(self)                                                       \
	VALA_PANEL_LAYOUT(gtk_bin_get_child(gtk_bin_get_child(GTK_BIN(self))))

void vala_panel_toplevel_configure(ValaPanelToplevel *self, const char *page);
GtkMenu *vala_panel_toplevel_get_plugin_menu(ValaPanelToplevel *self, ValaPanelApplet *pl);
void vala_panel_toplevel_destroy_pref_dialog(ValaPanelToplevel *self);

G_END_DECLS

#endif // TOPLEVEL_H
