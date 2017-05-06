#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "lib/constants.h"
#include "lib/panel-platform.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelToplevelUnit, vala_panel_toplevel_unit, VALA_PANEL, TOPLEVEL_UNIT,
                     GtkApplicationWindow)

ValaPanelToplevelUnit *vala_panel_toplevel_unit_new_from_position(GtkApplication *app,
                                                                  const char *uid, int mon,
                                                                  PanelGravity edge);
ValaPanelToplevelUnit *vala_panel_toplevel_unit_new_from_uid(GtkApplication *app,
                                                             ValaPanelPlatform *plt,
                                                             const char *uid);

G_END_DECLS

#endif // TOPLEVEL_H
