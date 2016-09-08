#ifndef RUNNERAPP_H
#define RUNNERAPP_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelRunApplication, vala_panel_run_application, VALA_PANEL,
                     RUN_APPLICATION, GtkApplication)

G_END_DECLS

#endif // RUNNERAPP_H
