#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "lib/constants.h"
#include "lib/panel-platform.h"

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

G_END_DECLS

#endif // TOPLEVEL_H
