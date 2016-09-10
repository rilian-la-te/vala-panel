/*
 * vala-panel
 * Copyright (C) 2015-2016 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef RUNNERNEW_H
#define RUNNERNEW_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_DECLARE_FINAL_TYPE(ValaPanelRunner, vala_panel_runner, VALA_PANEL, RUNNER, GtkDialog)

ValaPanelRunner *vala_panel_runner_new(GtkApplication *app);
void gtk_run(ValaPanelRunner *self);

#endif // RUNNERNEW_H
