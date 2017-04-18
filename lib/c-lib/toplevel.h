#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GAP 2

G_DECLARE_FINAL_TYPE(ValaPanelToplevelUnit, vala_panel_toplevel_unit, VALA_PANEL, TOPLEVEL_UNIT,
                     GtkApplicationWindow)

ValaPanelToplevelUnit *vala_panel_toplevel_unit_new_from_position(GtkApplication *app,
                                                                  const char *uid, int mon,
                                                                  GtkPositionType edge);
ValaPanelToplevelUnit *vala_panel_toplevel_unit_new_from_uid(GtkApplication *app, char *uid);

G_END_DECLS

#endif // TOPLEVEL_H
