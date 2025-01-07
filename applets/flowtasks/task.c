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

#include "task.h"
#include "flowtasks-enums.h"

#include <gtk/gtk.h>

typedef struct
{
	char *uuid;
	GSimpleActionGroup *grp;
} ValaPanelTaskPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(ValaPanelTask, vala_panel_task, G_TYPE_OBJECT)

enum
{
	TASK_DUMMY,
	TASK_UUID,
	TASK_TITLE,
	TASK_APP_ID,
	TASK_TOOLTIP,
	TASK_ICON,
	TASK_STATE,
	TASK_OUTPUT,
	TASK_LAST
};
static GParamSpec *task_specs[TASK_LAST];
static uint destroy_signal;

static void activate_close(GSimpleAction *act, GVariant *param, gpointer obj);
static void activate_minimize(GSimpleAction *act, GVariant *param, gpointer obj);
static void activate_maximize(GSimpleAction *act, GVariant *param, gpointer obj);
static void activate_fullscreen(GSimpleAction *act, GVariant *param, gpointer obj);
static void activate_activate(GSimpleAction *act, GVariant *param, gpointer obj);

static const GActionEntry entries[] = {
	{ VT_ACTION_CLOSE, activate_close, NULL, NULL, NULL, { 0 } },
	{ VT_ACTION_FULLSCREEN, activate_minimize, NULL, NULL, NULL, { 0 } },
	{ VT_ACTION_MINIMIZE, activate_maximize, NULL, NULL, NULL, { 0 } },
	{ VT_ACTION_MAXIMIZE, activate_fullscreen, NULL, NULL, NULL, { 0 } },
	{ VT_ACTION_ACTIVATE, activate_activate, NULL, NULL, NULL, { 0 } },
};

static void activate_close(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                           gpointer obj)
{
	ValaPanelTask *self = VALA_PANEL_TASK(obj);
	vala_panel_task_close(self);
}
static void activate_minimize(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                              gpointer obj)
{
	ValaPanelTask *self = VALA_PANEL_TASK(obj);
	vala_panel_task_toggle_minimize(self);
}
static void activate_maximize(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                              gpointer obj)
{
	ValaPanelTask *self = VALA_PANEL_TASK(obj);
	vala_panel_task_toggle_maximize(self);
}
static void activate_fullscreen(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                                gpointer obj)
{
	ValaPanelTask *self = VALA_PANEL_TASK(obj);
	vala_panel_task_toggle_fullscreen(self);
}

static void activate_activate(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                              gpointer obj)
{
	ValaPanelTask *self = VALA_PANEL_TASK(obj);
	vala_panel_task_activate(self);
}

const char *vala_panel_task_get_uuid(ValaPanelTask *self)
{
	ValaPanelTaskPrivate *p = vala_panel_task_get_instance_private(self);
	return p->uuid;
}

ValaPanelTaskInfo *vala_panel_task_get_info(ValaPanelTask *self)
{
	g_return_val_if_fail(VALA_PANEL_IS_TASK(self), NULL);
	return VALA_PANEL_TASK_GET_CLASS(self)->get_info(self);
}
ValaPanelTaskState vala_panel_task_get_state(ValaPanelTask *self)
{
	g_return_val_if_fail(VALA_PANEL_IS_TASK(self), STATE_CLOSED);
	return VALA_PANEL_TASK_GET_CLASS(self)->get_state(self);
}
int vala_panel_task_get_output(ValaPanelTask *self)
{
	g_return_val_if_fail(VALA_PANEL_IS_TASK(self), 0);
	return VALA_PANEL_TASK_GET_CLASS(self)->get_output(self);
}
void vala_panel_task_set_state(ValaPanelTask *self, ValaPanelTaskState state)
{
	g_return_if_fail(VALA_PANEL_IS_TASK(self));
	VALA_PANEL_TASK_GET_CLASS(self)->set_state(self, state);
}

GMenuModel *vala_panel_task_get_menu_model(ValaPanelTask *self)
{
	g_autoptr(GtkBuilder) builder =
	    gtk_builder_new_from_resource("/org/vala-panel/flowtasks/task-menus.ui");
	GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "flowtasks-context-menu"));
	return g_object_ref(menu);
}

void vala_panel_task_toggle_minimize(ValaPanelTask *self)
{
	g_return_if_fail(VALA_PANEL_IS_TASK(self));
	ValaPanelTaskState state = VALA_PANEL_TASK_GET_CLASS(self)->get_state(self);
	state ^= STATE_MINIMIZED;
	VALA_PANEL_TASK_GET_CLASS(self)->set_state(self, state);
}
void vala_panel_task_toggle_maximize(ValaPanelTask *self)
{
	g_return_if_fail(VALA_PANEL_IS_TASK(self));
	ValaPanelTaskState state = VALA_PANEL_TASK_GET_CLASS(self)->get_state(self);
	state ^= STATE_MAXIMIZED;
	VALA_PANEL_TASK_GET_CLASS(self)->set_state(self, state);
}
void vala_panel_task_toggle_fullscreen(ValaPanelTask *self)
{
	g_return_if_fail(VALA_PANEL_IS_TASK(self));
	ValaPanelTaskState state = VALA_PANEL_TASK_GET_CLASS(self)->get_state(self);
	state ^= STATE_FULLSCREEN;
	VALA_PANEL_TASK_GET_CLASS(self)->set_state(self, state);
}
void vala_panel_task_activate(ValaPanelTask *self)
{
	g_return_if_fail(VALA_PANEL_IS_TASK(self));
	ValaPanelTaskState state = VALA_PANEL_TASK_GET_CLASS(self)->get_state(self);
	state |= STATE_ACTIVATED;
	VALA_PANEL_TASK_GET_CLASS(self)->set_state(self, state);
}
void vala_panel_task_close(ValaPanelTask *self)
{
	g_return_if_fail(VALA_PANEL_IS_TASK(self));
	ValaPanelTaskState state = VALA_PANEL_TASK_GET_CLASS(self)->get_state(self);
	state |= STATE_CLOSED;
	VALA_PANEL_TASK_GET_CLASS(self)->set_state(self, state);
}

bool vala_panel_task_is_minimized(ValaPanelTask *self)
{
	g_return_val_if_fail(VALA_PANEL_IS_TASK(self), false);
	ValaPanelTaskState state = VALA_PANEL_TASK_GET_CLASS(self)->get_state(self);
	return state & STATE_MINIMIZED;
}

GActionMap *vala_panel_task_get_action_map(ValaPanelTask *self)
{
	g_return_val_if_fail(VALA_PANEL_IS_TASK(self), false);
	ValaPanelTaskPrivate *p = vala_panel_task_get_instance_private(self);
	return G_ACTION_MAP(p->grp);
}

void vala_panel_task_notify(ValaPanelTask *self, ValaPanelTaskNotify obj)
{
	g_return_if_fail(VALA_PANEL_IS_TASK(self));
	switch (obj)
	{
	case NOTIFY_REQUEST_REMOVE:
		g_signal_emit(self, destroy_signal, 0);
	}
}

static void vala_panel_task_init(ValaPanelTask *self)
{
	ValaPanelTaskPrivate *p = vala_panel_task_get_instance_private(self);
	p->uuid                 = g_uuid_string_random();
	p->grp                  = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(p->grp), entries, G_N_ELEMENTS(entries), self);
}

static void vala_panel_task_get_property(GObject *object, guint property_id, GValue *value,
                                         GParamSpec *pspec)
{
	ValaPanelTask *self      = VALA_PANEL_TASK(object);
	ValaPanelTaskPrivate *p  = vala_panel_task_get_instance_private(self);
	ValaPanelTaskInfo *info  = VALA_PANEL_TASK_GET_CLASS(self)->get_info(self);
	ValaPanelTaskState state = VALA_PANEL_TASK_GET_CLASS(self)->get_state(self);
	int output               = VALA_PANEL_TASK_GET_CLASS(self)->get_output(self);
	switch (property_id)
	{
	case TASK_UUID:
		g_value_set_string(value, p->uuid);
		break;
	case TASK_TITLE:
		g_value_set_string(value, info->title);
		break;
	case TASK_APP_ID:
		g_value_set_string(value, info->app_id);
		break;
	case TASK_TOOLTIP:
		g_value_set_string(value, info->tooltip);
		break;
	case TASK_ICON:
		g_value_set_object(value, info->icon);
		break;
	case TASK_STATE:
		g_value_set_flags(value, state);
		break;
	case TASK_OUTPUT:
		g_value_set_int(value, output);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_task_set_property(GObject *object, guint property_id, const GValue *value,
                                         GParamSpec *pspec)
{
	ValaPanelTask *self = VALA_PANEL_TASK(object);
	ValaPanelTaskState st;
	switch (property_id)
	{
	case TASK_STATE:
		st = g_value_get_flags(value);
		VALA_PANEL_TASK_GET_CLASS(self)->set_state(self, st);
		g_object_notify_by_pspec(object, pspec);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_task_finalize(GObject *base)
{
	ValaPanelTask *self     = VALA_PANEL_TASK(base);
	ValaPanelTaskPrivate *p = vala_panel_task_get_instance_private(self);
	g_clear_pointer(&p->uuid, g_free);
	g_clear_object(&p->grp);
	G_OBJECT_CLASS(vala_panel_task_parent_class)->finalize(base);
}

void vala_panel_task_class_init(ValaPanelTaskClass *klass)
{
	GObjectClass *oclass  = G_OBJECT_CLASS(klass);
	klass->get_menu_model = vala_panel_task_get_menu_model;
	oclass->set_property  = vala_panel_task_set_property;
	oclass->get_property  = vala_panel_task_get_property;
	oclass->finalize      = vala_panel_task_finalize;
	task_specs[TASK_UUID] =
	    g_param_spec_string(VT_KEY_UUID,
	                        VT_KEY_UUID,
	                        VT_KEY_UUID,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));
	task_specs[TASK_TITLE] =
	    g_param_spec_string(VT_KEY_TITLE,
	                        VT_KEY_TITLE,
	                        VT_KEY_TITLE,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));
	task_specs[TASK_APP_ID] =
	    g_param_spec_string(VT_KEY_APP_ID,
	                        VT_KEY_APP_ID,
	                        VT_KEY_APP_ID,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));
	task_specs[TASK_TOOLTIP] =
	    g_param_spec_string(VT_KEY_TOOLTIP,
	                        VT_KEY_TOOLTIP,
	                        VT_KEY_TOOLTIP,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));

	task_specs[TASK_ICON] =
	    g_param_spec_object(VT_KEY_ICON,
	                        VT_KEY_ICON,
	                        VT_KEY_ICON,
	                        G_TYPE_ICON,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));

	task_specs[TASK_STATE] =
	    g_param_spec_flags(VT_KEY_STATE,
	                       VT_KEY_STATE,
	                       VT_KEY_STATE,
	                       vala_panel_task_state_get_type(),
	                       STATE_NORMAL,
	                       (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                                     G_PARAM_CONSTRUCT));

	task_specs[TASK_OUTPUT] =
	    g_param_spec_int(VT_KEY_OUTPUT,
	                     VT_KEY_OUTPUT,
	                     VT_KEY_OUTPUT,
	                     -1,
	                     G_MAXINT,
	                     0,
	                     (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));

	g_object_class_install_properties(oclass, TASK_LAST, task_specs);
	destroy_signal = g_signal_new(VT_KEY_REQUEST_REMOVE,
	                              vala_panel_task_get_type(),
	                              G_SIGNAL_RUN_LAST,
	                              0,
	                              NULL,
	                              NULL,
	                              g_cclosure_marshal_VOID__VOID,
	                              G_TYPE_NONE,
	                              0);
}
