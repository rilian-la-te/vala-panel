/*
 * vala-panel
 * Copyright (C) 2020 Konstantin Pugin <ria.freelander@gmail.com>
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

#ifndef TASK_MODEL_H
#define TASK_MODEL_H

#include <gio/gio.h>
#include <glib-object.h>
#include <stdbool.h>

#include "task.h"

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(ValaPanelTaskModel, vala_panel_task_model, VALA_PANEL, TASK_MODEL, GObject)

#define VTM_KEY_SHOW_LAUNCHERS "show-launchers"
#define VTM_KEY_ONLY_LAUNCHERS "only-launchers"
#define VTM_KEY_ONLY_MINIMIZED "only-minimized"
#define VTM_KEY_DOCK_MODE "dock-mode"
#define VTM_KEY_CURRENT_OUTPUT "current-output"

#define VTM_ACTION_NEW_INSTANCE "new-instance"
#define VTM_ACTION_DESKTOP "desktop-action"

struct _ValaPanelTaskModelClass
{
	GObjectClass parent_class;
	void (*start_manager)(ValaPanelTaskModel *model);
	void (*stop_manager)(ValaPanelTaskModel *model);
	gpointer padding[12];
};

bool vala_panel_task_model_remove_task(ValaPanelTaskModel *self, const char *uuid);
ValaPanelTask *vala_panel_task_model_get_by_uuid(ValaPanelTaskModel *self, const char *uuid);
void vala_panel_task_model_add_task(ValaPanelTaskModel *self, ValaPanelTask *task);
int vala_panel_task_model_get_current_output_num(ValaPanelTaskModel *self);
void vala_panel_task_model_change_current_output(ValaPanelTaskModel *self, int output);

G_END_DECLS

#endif // INFODATA_H
