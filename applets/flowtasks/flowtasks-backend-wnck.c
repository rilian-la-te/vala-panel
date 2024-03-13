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

#include <gdk/gdkx.h>
#include <libwnck/libwnck.h>

#include "flowtasks-backend-wnck.h"
#include "libwnck-aux.h"

struct _WnckTask
{
	ValaPanelTask parent;
	/* info for task */
	ValaPanelTaskInfo wnck_info;
	/* wnck information */
	WnckWindow *window;
	WnckClassGroup *class_group;
	int output;
};

G_DEFINE_TYPE(WnckTask, wnck_task, vala_panel_task_get_type())

static ValaPanelTaskInfo *wnck_task_get_info(ValaPanelTask *parent)
{
	g_return_val_if_fail(WNCK_IS_TASK(parent), NULL);
	WnckTask *self = WNCK_TASK(parent);
	return &self->wnck_info;
}

static ValaPanelTaskState wnck_task_get_state(ValaPanelTask *parent)
{
	g_return_val_if_fail(WNCK_IS_TASK(parent), STATE_NORMAL);
	WnckTask *self        = WNCK_TASK(parent);
	ValaPanelTaskState st = 0;
	if (!WNCK_IS_WINDOW(self->window))
		return st;
	WnckWindowState wnck_st = wnck_window_get_state(self->window);
	if (wnck_st & WNCK_WINDOW_STATE_FULLSCREEN)
		st |= STATE_FULLSCREEN;
	if (wnck_st & WNCK_WINDOW_STATE_MINIMIZED)
		st |= STATE_MINIMIZED;
	if (wnck_st & WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY &&
	    wnck_st & WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY)
		st |= STATE_MAXIMIZED;
	if (wnck_window_is_active(self->window))
		st |= STATE_ACTIVATED;
	return st;
}

static int wnck_task_get_output(ValaPanelTask *parent)
{
	g_return_val_if_fail(WNCK_IS_TASK(parent), ALL_OUTPUTS);
	WnckTask *self = WNCK_TASK(parent);
	if (!WNCK_IS_WINDOW(self->window))
		return ALL_OUTPUTS;
	WnckWorkspace *wsp = wnck_window_get_workspace(self->window);
	return wsp ? wnck_workspace_get_number(wsp) : ALL_OUTPUTS;
}

static void wnck_task_set_state(ValaPanelTask *parent, ValaPanelTaskState st)
{
	g_return_if_fail(WNCK_IS_TASK(parent));
	WnckTask *self               = WNCK_TASK(parent);
	ValaPanelTaskState old_state = wnck_task_get_state(parent);
	if (old_state != st)
	{
		if (old_state & STATE_MAXIMIZED && !(st & STATE_MAXIMIZED))
			wnck_window_unmaximize(self->window);
		if (st & STATE_MAXIMIZED && !(old_state & STATE_MAXIMIZED))
			wnck_window_maximize(self->window);
		if (old_state & STATE_MINIMIZED && !(st & STATE_MINIMIZED))
			wnck_window_unminimize(self->window, gtk_get_current_event_time());
		if (st & STATE_MINIMIZED && !(old_state & STATE_MINIMIZED))
			wnck_window_minimize(self->window);
		if (old_state & STATE_FULLSCREEN && !(st & STATE_FULLSCREEN))
			wnck_window_set_fullscreen(self->window, false);
		if (st & STATE_FULLSCREEN && !(old_state & STATE_FULLSCREEN))
			wnck_window_set_fullscreen(self->window, true);
		if (st & STATE_ACTIVATED && !(old_state & STATE_ACTIVATED))
			wnck_window_activate(self->window, gtk_get_current_event_time());
	}
}

static void wnck_task_state_changed(WnckWindow *window, WnckTask *self)
{
	g_object_notify(G_OBJECT(self), VT_KEY_STATE);
}

static void wnck_task_output_changed(WnckWindow *window, WnckTask *self)
{
	int output = wnck_task_get_output(VALA_PANEL_TASK(self));
	if (self->output != output)
	{
		self->output = output;
		g_object_notify(G_OBJECT(self), VT_KEY_OUTPUT);
	}
}

static void wnck_task_icon_changed(WnckWindow *window, WnckTask *child)
{
	g_return_if_fail(WNCK_IS_WINDOW(window));
	g_return_if_fail(WNCK_IS_TASK(child));
	if (wnck_window_has_icon_name(window))
	{
		g_clear_object(&child->wnck_info.icon);
		child->wnck_info.icon =
		    g_themed_icon_new_with_default_fallbacks(wnck_window_get_icon_name(window));
	}
	else
	{
		GdkPixbuf *pixbuf = NULL;
		pixbuf            = wnck_window_get_icon(window);
		if (pixbuf == NULL)
			pixbuf = wnck_window_get_mini_icon(window);

		/* leave when there is no valid pixbuf */
		if (G_UNLIKELY(pixbuf == NULL))
			return;
		g_clear_object(&child->wnck_info.icon);
		child->wnck_info.icon = G_ICON(pixbuf);
	}
	g_object_notify(G_OBJECT(child), VT_KEY_ICON);
}

static void wnck_task_set_tooltip(WnckTask *self)
{
	g_clear_pointer(&self->wnck_info.tooltip, g_free);
	ValaPanelMatcher *matcher = vala_panel_matcher_get();
	GDesktopAppInfo *info     = libwnck_aux_match_wnck_window(matcher, self->window);
	if (info)
		self->wnck_info.tooltip = (char *)g_app_info_get_description(G_APP_INFO(info));
	else
		self->wnck_info.tooltip = self->wnck_info.title;
	g_object_notify(G_OBJECT(self), VT_KEY_TOOLTIP);
}

static void wnck_task_title_changed(WnckWindow *window, WnckTask *self)
{
	g_return_if_fail(WNCK_IS_WINDOW(window));
	g_return_if_fail(WNCK_IS_TASK(self));
	g_clear_pointer(&self->wnck_info.title, g_free);
	self->wnck_info.title = g_strdup(wnck_window_get_name(window));
	g_object_freeze_notify(G_OBJECT(self));
	wnck_task_set_tooltip(self);
	g_object_notify(G_OBJECT(self), VT_KEY_TITLE);
	g_object_thaw_notify(G_OBJECT(self));
}

static void wnck_task_app_id_changed(WnckWindow *window, WnckTask *self)
{
	g_clear_pointer(&self->wnck_info.app_id, g_free);
	ValaPanelMatcher *matcher = vala_panel_matcher_get();
	GDesktopAppInfo *info     = libwnck_aux_match_wnck_window(matcher, self->window);
	if (info)
		self->wnck_info.app_id = (char *)g_app_info_get_description(G_APP_INFO(info));
	else
		self->wnck_info.tooltip = self->wnck_info.title;
	g_autofree char *gtk_app_id =
	    libwnck_aux_get_utf8_prop(wnck_window_get_xid(window), "_GTK_APPLICATION_ID");
	self->wnck_info.app_id =
	    gtk_app_id ? g_strdup(gtk_app_id) : g_strdup(wnck_window_get_class_group_name(window));
	g_object_notify(G_OBJECT(self), VT_KEY_APP_ID);
}

static ValaPanelTask *wnck_task_new(WnckWindow *window)
{
	WnckTask *task      = g_object_new(wnck_task_get_type(), NULL);
	task->window        = window;
	task->class_group   = wnck_window_get_class_group(window);
	task->wnck_info.pid = wnck_window_get_pid(window);

	g_signal_connect(G_OBJECT(window),
	                 "icon-changed",
	                 G_CALLBACK(wnck_task_icon_changed),
	                 task);
	g_signal_connect(G_OBJECT(window),
	                 "name-changed",
	                 G_CALLBACK(wnck_task_title_changed),
	                 task);
	g_signal_connect(G_OBJECT(window),
	                 "class-changed",
	                 G_CALLBACK(wnck_task_app_id_changed),
	                 task);
	g_signal_connect(G_OBJECT(window),
	                 "state-changed",
	                 G_CALLBACK(wnck_task_state_changed),
	                 task);
	g_signal_connect(G_OBJECT(window),
	                 "workspace-changed",
	                 G_CALLBACK(wnck_task_output_changed),
	                 task);
	g_signal_connect(G_OBJECT(window),
	                 "geometry-changed",
	                 G_CALLBACK(wnck_task_output_changed),
	                 task);
	g_object_freeze_notify(G_OBJECT(task));
	wnck_task_title_changed(window, task);
	wnck_task_app_id_changed(window, task);
	wnck_task_icon_changed(window, task);
	wnck_task_state_changed(window, task);
	wnck_task_output_changed(window, task);
	g_object_thaw_notify(G_OBJECT(task));
	return VALA_PANEL_TASK(task);
}

static void wnck_task_init(WnckTask *self)
{
	self->window            = NULL;
	self->class_group       = NULL;
	self->wnck_info.app_id  = NULL;
	self->wnck_info.title   = NULL;
	self->wnck_info.tooltip = NULL;
	self->wnck_info.icon    = NULL;
}

static void wnck_task_finalize(GObject *parent)
{
	g_return_if_fail(WNCK_IS_TASK(parent));
	WnckTask *self = WNCK_TASK(parent);
	g_signal_handlers_disconnect_by_data(self->window, self);
	g_clear_pointer(&self->wnck_info.app_id, g_free);
	g_clear_pointer(&self->wnck_info.title, g_free);

	g_clear_pointer(&self->wnck_info.tooltip, g_free);
	g_clear_object(&self->wnck_info.icon);
	G_OBJECT_CLASS(wnck_task_parent_class)->finalize(parent);
}

static void wnck_task_class_init(WnckTaskClass *klass)
{
	ValaPanelTaskClass *vclass = VALA_PANEL_TASK_CLASS(klass);
	vclass->get_state          = wnck_task_get_state;
	vclass->get_output         = wnck_task_get_output;
	vclass->get_info           = wnck_task_get_info;
	vclass->set_state          = wnck_task_set_state;
	GObjectClass *oclass       = G_OBJECT_CLASS(klass);
	oclass->finalize           = wnck_task_finalize;
}

static void wnck_task_model_start_manager(ValaPanelTaskModel *self);
static void wnck_task_model_stop_manager(ValaPanelTaskModel *self);

struct _WnckTaskModel
{
	ValaPanelTaskModel parent;
	/* wnck information */
	WnckScreen *screen;
	/*gdk information */
	GdkDisplay *display;
	/*window tracking */
	GHashTable *windows;
};

G_DEFINE_TYPE(WnckTaskModel, wnck_task_model, vala_panel_task_model_get_type())

static void wnck_task_model_init(WnckTaskModel *self)
{
	self->screen  = NULL;
	self->display = NULL;
	self->windows = g_hash_table_new(g_direct_hash, g_direct_equal);
}

static void wnck_task_model_finalize(GObject *parent)
{
	g_return_if_fail(WNCK_IS_TASK_MODEL(parent));
	WnckTaskModel *self = WNCK_TASK_MODEL(parent);
	g_hash_table_destroy(self->windows);
	G_OBJECT_CLASS(wnck_task_model_parent_class)->finalize(parent);
}

static void wnck_task_model_class_init(WnckTaskModelClass *klass)
{
	ValaPanelTaskModelClass *vclass = VALA_PANEL_TASK_MODEL_CLASS(klass);
	GObjectClass *oclass            = G_OBJECT_CLASS(klass);
	vclass->start_manager           = wnck_task_model_start_manager;
	vclass->stop_manager            = wnck_task_model_stop_manager;
	oclass->finalize                = wnck_task_model_finalize;
}

static void wnck_task_model_active_workspace_changed(WnckScreen *screen,
                                                     WnckWorkspace *previous_workspace,
                                                     WnckTaskModel *tasklist)
{
	g_return_if_fail(WNCK_IS_SCREEN(screen));
	g_return_if_fail(WNCK_IS_TASK_MODEL(tasklist));

	WnckWorkspace *wrk = wnck_screen_get_active_workspace(screen);
	vala_panel_task_model_change_current_output(VALA_PANEL_TASK_MODEL(tasklist),
	                                            wnck_workspace_get_number(wrk));
}

static void wnck_task_model_active_window_changed(WnckScreen *screen, WnckWindow *prev_window,
                                                  WnckTaskModel *tasklist)
{
	g_return_if_fail(WNCK_IS_SCREEN(screen));
	g_return_if_fail(WNCK_IS_TASK_MODEL(tasklist));

	WnckWindow *win = wnck_screen_get_active_window(screen);
	ValaPanelTask *prev_task =
	    vala_panel_task_model_get_by_uuid(VALA_PANEL_TASK_MODEL(tasklist),
	                                      g_hash_table_lookup(tasklist->windows, prev_window));
	ValaPanelTask *next_task =
	    vala_panel_task_model_get_by_uuid(VALA_PANEL_TASK_MODEL(tasklist),
	                                      g_hash_table_lookup(tasklist->windows, win));
	if (prev_task)
		g_object_notify(G_OBJECT(prev_task), VT_KEY_STATE);
	if (next_task)
		g_object_notify(G_OBJECT(next_task), VT_KEY_STATE);
}

static void wnck_task_model_viewports_changed(WnckScreen *screen, WnckTaskModel *tasklist)
{
	WnckWorkspace *active_ws;

	g_return_if_fail(WNCK_IS_SCREEN(screen));
	g_return_if_fail(WNCK_IS_TASK_MODEL(tasklist));
	g_return_if_fail(tasklist->screen == screen);

	/* pretend we changed workspace, this will update the
	 * visibility of all the buttons */
	active_ws = wnck_screen_get_active_workspace(screen);
	wnck_task_model_active_workspace_changed(screen, active_ws, tasklist);
}

static void wnck_task_model_window_removed(WnckScreen *screen, WnckWindow *window,
                                           WnckTaskModel *tasklist)
{
	vala_panel_task_model_remove_task(VALA_PANEL_TASK_MODEL(tasklist),
	                                  g_hash_table_lookup(tasklist->windows, window));
}

static void wnck_task_model_window_added(WnckScreen *screen, WnckWindow *window,
                                         WnckTaskModel *tasklist)
{
	ValaPanelTask *task = wnck_task_new(window);
	vala_panel_task_model_add_task(VALA_PANEL_TASK_MODEL(tasklist), VALA_PANEL_TASK(task));
	g_hash_table_insert(tasklist->windows, window, (char *)vala_panel_task_get_uuid(task));
}

static void wnck_task_model_start_manager(ValaPanelTaskModel *parent)
{
	g_return_if_fail(WNCK_IS_TASK_MODEL(parent));

	WnckTaskModel *tasklist = WNCK_TASK_MODEL(parent);

	g_return_if_fail(tasklist->screen == NULL);
	g_return_if_fail(tasklist->display == NULL);

	int output;
	g_object_get(tasklist, VTM_KEY_CURRENT_OUTPUT, &output, NULL);

	/* set the new screen */
	tasklist->display = gdk_display_get_default();
	/* We need the screen number for Wnck. We could use wnck_screen_get_default
	   but that might not be correct everywhere. */
	tasklist->screen = wnck_screen_get(output);

	/* add all existing windows on this screen */
	GList *windows = wnck_screen_get_windows(tasklist->screen);
	for (GList *li = windows; li != NULL; li = li->next)
	{
		ValaPanelTask *task = wnck_task_new(WNCK_WINDOW(li->data));
		vala_panel_task_model_add_task(VALA_PANEL_TASK_MODEL(tasklist), task);
	}

	/* monitor screen changes */
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "active-window-changed",
	                 G_CALLBACK(wnck_task_model_active_window_changed),
	                 tasklist);
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "active-workspace-changed",
	                 G_CALLBACK(wnck_task_model_active_workspace_changed),
	                 tasklist);
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "window-opened",
	                 G_CALLBACK(wnck_task_model_window_added),
	                 tasklist);
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "window-closed",
	                 G_CALLBACK(wnck_task_model_window_removed),
	                 tasklist);
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "viewports-changed",
	                 G_CALLBACK(wnck_task_model_viewports_changed),
	                 tasklist);

	/* update the viewport if not all monitors are shown */
	if (output != ALL_OUTPUTS)
	{
		/* update the monitor geometry */
		// 		xfce_tasklist_update_monitor_geometry(tasklist);
	}
}

static void wnck_task_model_stop_manager(ValaPanelTaskModel *parent)
{
	g_return_if_fail(WNCK_IS_TASK_MODEL(parent));

	WnckTaskModel *tasklist = WNCK_TASK_MODEL(parent);

	g_return_if_fail(WNCK_IS_SCREEN(tasklist->screen));
	g_return_if_fail(GDK_IS_DISPLAY(tasklist->display));

	/* disconnect monitor signals */
	g_signal_handlers_disconnect_by_data(tasklist->screen, tasklist);
	g_signal_handlers_disconnect_by_data(tasklist->display, tasklist);
}
