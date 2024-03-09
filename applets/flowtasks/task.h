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

#ifndef TASK_H_INCLUDED
#define TASK_H_INCLUDED

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define ALL_OUTPUTS -1

G_DECLARE_DERIVABLE_TYPE(ValaPanelTask, vala_panel_task, VALA_PANEL, TASK, GObject)

#define VT_KEY_UUID "uuid"
#define VT_KEY_TITLE "title"
#define VT_KEY_APP_ID "app-id"
#define VT_KEY_TOOLTIP "tooltip"
#define VT_KEY_ICON "icon"
#define VT_KEY_STATE "state"
#define VT_KEY_OUTPUT "output"
#define VT_KEY_REQUEST_REMOVE "request-remove"

#define VT_TASK_ACTION_PREFIX "vt-task"
#define VT_ACTION_ACTIVATE "activate"
#define VT_ACTION_FULLSCREEN "fullscreen"
#define VT_ACTION_MINIMIZE "minimize"
#define VT_ACTION_MAXIMIZE "maximize"
#define VT_ACTION_CLOSE "close"

typedef enum /*< flags >*/
{
	STATE_NORMAL       = 0,
	STATE_MINIMIZED    = 1 << 0,
	STATE_MAXIMIZED    = 1 << 1,
	STATE_ACTIVATED    = 1 << 2,
	STATE_FULLSCREEN   = 1 << 3,
	STATE_CLOSED       = 1 << 4,
	STATE_SKIP_TASKBAR = 1 << 5,
} ValaPanelTaskState;

#define STATE_ALL                                                                                  \
	(STATE_MINIMIZED | STATE_MAXIMIZED | STATE_ACTIVATED | STATE_FULLSCREEN | STATE_CLOSED |   \
	 STATE_SKIP_TASKBAR)

typedef enum
{
	NOTIFY_REQUEST_REMOVE,
} ValaPanelTaskNotify;

typedef struct
{
	char *title;
	char *app_id;
	char *tooltip;
	GIcon *icon;
	int64_t pid;
} ValaPanelTaskInfo;

struct _ValaPanelTaskClass
{
	GObjectClass parent_class;
	ValaPanelTaskInfo *(*get_info)(ValaPanelTask *self);
	ValaPanelTaskState (*get_state)(ValaPanelTask *self);
	int (*get_output)(ValaPanelTask *self);
	void (*set_state)(ValaPanelTask *self, ValaPanelTaskState state);
	GMenuModel *(*get_menu_model)(ValaPanelTask *self);
	gpointer padding[12];
};

ValaPanelTask *vala_panel_task_get_uuid_finder(const char *uuid);
const char *vala_panel_task_get_uuid(ValaPanelTask *self);
ValaPanelTaskInfo *vala_panel_task_get_info(ValaPanelTask *self);
ValaPanelTaskState vala_panel_task_get_state(ValaPanelTask *self);
int vala_panel_task_get_output(ValaPanelTask *self);
GActionMap *vala_panel_task_get_action_map(ValaPanelTask *self);
void vala_panel_task_set_state(ValaPanelTask *self, ValaPanelTaskState state);
void vala_panel_task_notify(ValaPanelTask *self, ValaPanelTaskNotify obj);
bool vala_panel_task_is_minimized(ValaPanelTask *self);
void vala_panel_task_toggle_minimize(ValaPanelTask *self);
void vala_panel_task_toggle_maximize(ValaPanelTask *self);
void vala_panel_task_toggle_fullscreen(ValaPanelTask *self);
void vala_panel_task_activate(ValaPanelTask *self);
void vala_panel_task_close(ValaPanelTask *self);

G_END_DECLS

#endif // TASK_H_INCLUDED
