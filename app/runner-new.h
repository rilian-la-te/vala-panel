#ifndef RUNNERNEW_H
#define RUNNERNEW_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_DECLARE_FINAL_TYPE(ValaPanelRunner, vala_panel_runner, VALA_PANEL, RUNNER, GtkDialog)

ValaPanelRunner *vala_panel_runner_new(GtkApplication *app);
void gtk_run(ValaPanelRunner *self);

#endif // RUNNERNEW_H
