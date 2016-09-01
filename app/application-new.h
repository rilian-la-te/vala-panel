#ifndef APPLICATIONNEW_H
#define APPLICATIONNEW_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelApplication, vala_panel_application, VALA_PANEL, APPLICATION,
                     GtkApplication)

G_END_DECLS

#endif // APPLICATIONNEW_H
