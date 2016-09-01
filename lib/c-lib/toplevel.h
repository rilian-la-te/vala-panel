#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelToplevelUnit, vala_panel_toplevel_unit, VALA_PANEL, TOPLEVEL_UNIT,
		     GtkApplicationWindow)

G_END_DECLS

#endif // TOPLEVEL_H
