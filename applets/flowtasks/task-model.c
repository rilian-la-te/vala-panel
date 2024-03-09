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

#include "task-model.h"
#include "matcher.h"
#include <string.h>

#include <gtk/gtk.h>

/*
 * ValaPanelGroupTask GObject
 */

G_DECLARE_FINAL_TYPE(ValaPanelGroupTask, vala_panel_group_task, VALA_PANEL, GROUP_TASK,
                     ValaPanelTask)

struct _ValaPanelGroupTask
{
	ValaPanelTask parent;
	bool has_launcher : 1;
	GDesktopAppInfo *launcher_info;
	GHashTable *references;
	ValaPanelTaskInfo info;
};

G_DEFINE_TYPE(ValaPanelGroupTask, vala_panel_group_task, vala_panel_task_get_type())

static void child_spawn_func(G_GNUC_UNUSED void *data)
{
	setpgid(0, getpgid(getppid()));
}

static bool vala_panel_launch_with_context(GDesktopAppInfo *app_info, GAppLaunchContext *cxt,
                                           GList *uris)
{
	g_return_val_if_fail(G_IS_APP_INFO(app_info), false);
	g_autoptr(GError) err = NULL;
	bool ret              = g_desktop_app_info_launch_uris_as_manager(app_info,
                                                             uris,
                                                             cxt,
                                                             G_SPAWN_SEARCH_PATH,
                                                             child_spawn_func,
                                                             NULL,
                                                             NULL,
                                                             NULL,
                                                             &err);
	if (err)
		g_warning("%s\n", err->message);
	return ret;
}

bool vala_panel_group_task_new_instance(ValaPanelGroupTask *self, GAppLaunchContext *c);

static void activate_new_instance(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                                  gpointer obj)
{
	ValaPanelGroupTask *self = VALA_PANEL_GROUP_TASK(obj);
	GAppLaunchContext *gdk_context =
	    G_APP_LAUNCH_CONTEXT(gdk_display_get_app_launch_context(gdk_display_get_default()));
	vala_panel_group_task_new_instance(self, gdk_context);
}
static void activate_desktop_action(G_GNUC_UNUSED GSimpleAction *act, GVariant *param, gpointer obj)
{
	ValaPanelGroupTask *self = VALA_PANEL_GROUP_TASK(obj);
	const char *action       = g_variant_get_string(param, NULL);
	GAppLaunchContext *gdk_context =
	    G_APP_LAUNCH_CONTEXT(gdk_display_get_app_launch_context(gdk_display_get_default()));
	g_desktop_app_info_launch_action(self->launcher_info, action, gdk_context);
}

static const GActionEntry entries[] = {
	{ VTM_ACTION_NEW_INSTANCE, activate_new_instance, NULL, NULL, NULL, { 0 } },
	{ VTM_ACTION_DESKTOP, activate_desktop_action, "s", NULL, NULL, { 0 } }
};

static ValaPanelTaskInfo *vala_panel_group_task_get_info(ValaPanelTask *parent)
{
	ValaPanelGroupTask *self = VALA_PANEL_GROUP_TASK(parent);
	return &self->info;
}

static ValaPanelTaskState vala_panel_group_task_get_state(ValaPanelTask *parent)
{
	ValaPanelTaskState res_state = STATE_CLOSED;
	ValaPanelGroupTask *self     = VALA_PANEL_GROUP_TASK(parent);
	GHashTableIter iter;
	void *key, *value;
	g_hash_table_iter_init(&iter, self->references);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		if (VALA_PANEL_IS_TASK(value))
		{
			ValaPanelTask *task = VALA_PANEL_TASK(value);
			res_state &= vala_panel_task_get_state(task);
		}
	}
	return res_state;
}

static int vala_panel_group_task_get_output(ValaPanelTask *parent)
{
	ValaPanelGroupTask *self = VALA_PANEL_GROUP_TASK(parent);
	int res_out              = -2;
	GHashTableIter iter;
	void *key, *value;
	g_hash_table_iter_init(&iter, self->references);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		if (VALA_PANEL_IS_TASK(value))
		{
			ValaPanelTask *task = VALA_PANEL_TASK(value);
			int out             = vala_panel_task_get_output(task);
			res_out = res_out == out ? res_out : res_out == -2 ? out : ALL_OUTPUTS;
		}
	}
	return res_out;
}

static void vala_panel_group_task_set_state(ValaPanelTask *parent, ValaPanelTaskState state)
{
	ValaPanelGroupTask *self = VALA_PANEL_GROUP_TASK(parent);
	GHashTableIter iter;
	void *key, *value;
	g_hash_table_iter_init(&iter, self->references);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		if (VALA_PANEL_IS_TASK(value))
		{
			ValaPanelTask *task = VALA_PANEL_TASK(value);
			vala_panel_task_set_state(task, state & !STATE_FULLSCREEN);
		}
	}
}

static void vala_panel_group_task_info_from_desktop(ValaPanelGroupTask *self)
{
	g_return_if_fail(G_IS_APP_INFO(self->launcher_info));
	g_clear_object(&self->info.icon);
	g_clear_pointer(&self->info.app_id, g_free);
	g_clear_pointer(&self->info.title, g_free);
	g_clear_pointer(&self->info.tooltip, g_free);
	g_object_freeze_notify(G_OBJECT(self));
	const char *desktop_id = g_app_info_get_id(G_APP_INFO(self->launcher_info));
	if (g_str_has_suffix(desktop_id, ".desktop"))
	{
		uint32_t bn_len   = g_utf8_strlen(desktop_id, -1) - g_utf8_strlen(".desktop", -1);
		self->info.app_id = g_utf8_substring(desktop_id, 0, bn_len);
	}
	else
	{
		self->info.app_id = g_strdup(desktop_id);
	}
	g_object_notify(G_OBJECT(self), VT_KEY_APP_ID);
	self->info.title = g_strdup(g_app_info_get_name(G_APP_INFO(self->launcher_info)));
	g_object_notify(G_OBJECT(self), VT_KEY_TITLE);
	self->info.icon = g_object_ref(g_app_info_get_icon(G_APP_INFO(self->launcher_info)));
	g_object_notify(G_OBJECT(self), VT_KEY_ICON);
	self->info.tooltip = g_strdup(g_app_info_get_description(G_APP_INFO(self->launcher_info)));
	g_object_notify(G_OBJECT(self), VT_KEY_TOOLTIP);
	g_action_map_add_action_entries(vala_panel_task_get_action_map(VALA_PANEL_TASK(self)),
	                                entries,
	                                G_N_ELEMENTS(entries),
	                                self);
	g_object_thaw_notify(G_OBJECT(self));
}

static void vala_panel_group_task_info_not_found(ValaPanelGroupTask *self, const char *app_name,
                                                 const char *app_id, GIcon *icon)
{
	g_clear_object(&self->info.icon);
	g_clear_pointer(&self->info.app_id, g_free);
	g_clear_pointer(&self->info.title, g_free);
	g_clear_pointer(&self->info.tooltip, g_free);
	g_object_freeze_notify(G_OBJECT(self));
	self->info.title = g_strdup(app_name);
	g_object_notify(G_OBJECT(self), VT_KEY_TITLE);
	self->info.app_id = g_strdup(app_id);
	g_object_notify(G_OBJECT(self), VT_KEY_APP_ID);
	self->info.icon = g_object_ref(icon);
	g_object_notify(G_OBJECT(self), VT_KEY_ICON);
	self->info.tooltip = g_strdup(app_name);
	g_object_notify(G_OBJECT(self), VT_KEY_TOOLTIP);
	g_object_thaw_notify(G_OBJECT(self));
}

static void vala_panel_group_task_unlink(ValaPanelGroupTask *self, ValaPanelTask *task)
{
	g_hash_table_remove(self->references, (void *)vala_panel_task_get_uuid(task));
	if (!self->has_launcher && !g_hash_table_size(self->references))
		vala_panel_task_notify(VALA_PANEL_TASK(self), NOTIFY_REQUEST_REMOVE);
	else
		g_object_notify(G_OBJECT(self), VT_KEY_STATE);
}

static void vala_panel_group_task_link(ValaPanelGroupTask *self, ValaPanelTask *task)
{
	g_hash_table_insert(self->references, (void *)vala_panel_task_get_uuid(task), task);
	if (!self->has_launcher && self->info.app_id == NULL)
	{
		ValaPanelTaskInfo *tinfo  = vala_panel_task_get_info(task);
		ValaPanelMatcher *matcher = vala_panel_matcher_get();
		/* It should not be more than 4-5 tokens */
		g_autofree GStrv app_id_split = g_strsplit(tinfo->app_id, ".", 10);
		/* We need last app_id element for checking*/
		g_autofree char *app_name = g_strdup(app_id_split[g_strv_length(app_id_split) - 1]);
		g_print("%s, %s, %s, %ld\n", tinfo->title, app_name, tinfo->app_id, tinfo->pid);
		GDesktopAppInfo *launcher_info = NULL;
		launcher_info                  = vala_panel_matcher_match_arbitrary(matcher,
                                                                   tinfo->title,
                                                                   app_name,
                                                                   tinfo->app_id,
                                                                   tinfo->pid);

		// TODO: fix group task naming if no launcher info available
		if (launcher_info)
		{
			self->launcher_info = g_object_ref(launcher_info);
			vala_panel_group_task_info_from_desktop(self);
		}
		else
		{
			vala_panel_group_task_info_not_found(self,
			                                     app_name,
			                                     tinfo->app_id,
			                                     tinfo->icon);
		}
	}
	g_object_notify(G_OBJECT(self), VT_KEY_STATE);
}

bool vala_panel_group_task_new_instance(ValaPanelGroupTask *self, GAppLaunchContext *c)
{
	g_return_val_if_fail(VALA_PANEL_IS_GROUP_TASK(self), false);
	return vala_panel_launch_with_context(self->launcher_info, c, NULL);
}

void vala_panel_group_task_init_launcher(ValaPanelGroupTask *self, GDesktopAppInfo *info)
{
	g_return_if_fail(VALA_PANEL_IS_GROUP_TASK(self));
	if (self->has_launcher)
		return;
	self->launcher_info = g_object_ref(info);
	self->has_launcher  = true;
	vala_panel_group_task_info_from_desktop(self);
}

void vala_panel_group_task_remove_launcher(ValaPanelGroupTask *self)
{
	self->has_launcher = false;
	if (!g_hash_table_size(self->references))
		vala_panel_task_notify(VALA_PANEL_TASK(self), NOTIFY_REQUEST_REMOVE);
}

static bool vala_panel_group_task_has_launcher(ValaPanelGroupTask *self)
{
	g_return_val_if_fail(VALA_PANEL_IS_GROUP_TASK(self), false);
	return self->has_launcher;
}

static bool vala_panel_group_task_count_as_launcher(ValaPanelGroupTask *self, bool only_minimized,
                                                    int current_output)
{
	g_return_val_if_fail(VALA_PANEL_IS_GROUP_TASK(self), false);
	ValaPanelTaskState state = vala_panel_group_task_get_state(VALA_PANEL_TASK(self));
	int output               = vala_panel_group_task_get_output(VALA_PANEL_TASK(self));
	if (state & STATE_CLOSED)
		return true;
	if (only_minimized && !(state & STATE_MINIMIZED))
		return true;
	if (current_output != output && current_output != ALL_OUTPUTS)
		return true;
	return false;
}

static GMenuModel *vala_panel_group_task_get_menu_model(ValaPanelTask *parent)
{
	ValaPanelGroupTask *self = VALA_PANEL_GROUP_TASK(parent);
	g_autoptr(GtkBuilder) builder =
	    gtk_builder_new_from_resource("/org/vala-panel/flowtasks/task-menus.ui");
	GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "flowtasks-context-menu"));
	if (!G_IS_DESKTOP_APP_INFO(self->launcher_info))
		return g_object_ref(menu);
	GMenu *actions_menu = G_MENU(gtk_builder_get_object(builder, "desktop-actions"));
	GStrv actions       = (GStrv)g_desktop_app_info_list_actions(self->launcher_info);
	for (int i = 0; actions[i] != NULL; i++)
	{
		g_autofree char *name =
		    g_desktop_app_info_get_action_name(self->launcher_info, actions[i]);
		g_autofree char *action = g_strdup_printf(VTM_ACTION_DESKTOP "::%s", actions[i]);
		g_menu_append(actions_menu, name, action);
	}
	g_menu_freeze(actions_menu);
	return g_object_ref(menu);
}

static void vala_panel_group_task_init(ValaPanelGroupTask *self)
{
	self->has_launcher  = false;
	self->launcher_info = NULL;
	self->references    = g_hash_table_new(g_str_hash, g_str_equal);
	self->info.app_id   = NULL;
	self->info.title    = NULL;
	self->info.tooltip  = NULL;
	self->info.icon     = NULL;
}

static void vala_panel_group_task_finalize(GObject *base)
{
	ValaPanelGroupTask *self = VALA_PANEL_GROUP_TASK(base);
	g_clear_object(&self->launcher_info);
	g_clear_object(&self->info.icon);
	g_clear_pointer(&self->info.app_id, g_free);
	g_clear_pointer(&self->info.title, g_free);
	g_clear_pointer(&self->info.tooltip, g_free);
	G_OBJECT_CLASS(vala_panel_group_task_parent_class)->finalize(G_OBJECT(self));
}

void vala_panel_group_task_class_init(ValaPanelGroupTaskClass *klass)
{
	GObjectClass *oclass       = G_OBJECT_CLASS(klass);
	ValaPanelTaskClass *tclass = VALA_PANEL_TASK_CLASS(klass);
	tclass->get_menu_model     = vala_panel_group_task_get_menu_model;
	tclass->set_state          = vala_panel_group_task_set_state;
	tclass->get_info           = vala_panel_group_task_get_info;
	tclass->get_state          = vala_panel_group_task_get_state;
	tclass->get_output         = vala_panel_group_task_get_output;
	oclass->finalize           = vala_panel_group_task_finalize;
}

/*
 * ValaPanelTaskModel GObject
 */

typedef struct
{
	GSequence *window_items;
	GSequenceIter *last_visible_iter;
	int current_output_num;
	bool only_launchers : 1;
	bool show_launchers : 1;
	bool only_minimized : 1;
	bool current_output : 1;
	bool dock_mode : 1;
} ValaPanelTaskModelPrivate;

static void vala_panel_task_model_iface_init(GListModelInterface *iface);

G_DEFINE_TYPE_WITH_CODE(ValaPanelTaskModel, vala_panel_task_model, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, vala_panel_task_model_iface_init)
                            G_ADD_PRIVATE(ValaPanelTaskModel))

static void vala_panel_task_model_update_last_visible_iter(ValaPanelTaskModel *self);

enum
{
	MODEL_DUMMY,
	MODEL_SHOW_LAUNCHERS,
	MODEL_ONLY_MINIMIZED,
	MODEL_CURRENT_OUTPUT,
	MODEL_DOCK_MODE,
	MODEL_ONLY_LAUNCHERS,
	MODEL_LAST
};
static GParamSpec *model_specs[MODEL_LAST];

static bool vala_panel_task_model_is_task_visible(ValaPanelTaskModel *self, ValaPanelTask *task)
{
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	ValaPanelTaskState state;
	int out;
	g_object_get(task, VT_KEY_STATE, &state, VT_KEY_OUTPUT, &out, NULL);
	/* Launcher stuff */
	if (VALA_PANEL_IS_GROUP_TASK(task))
	{
		if (p->show_launchers)
			return vala_panel_group_task_count_as_launcher(VALA_PANEL_GROUP_TASK(task),
			                                               p->only_minimized,
			                                               p->current_output);
		else if (vala_panel_group_task_count_as_launcher(VALA_PANEL_GROUP_TASK(task),
		                                                 p->only_minimized,
		                                                 p->current_output))
			return false;
		if (p->only_launchers)
			return vala_panel_group_task_has_launcher(VALA_PANEL_GROUP_TASK(task));
	}
	if (p->current_output)
		return out == p->current_output_num || out == ALL_OUTPUTS;
	if (p->dock_mode)
		return VALA_PANEL_IS_GROUP_TASK(task);
	else
		return !VALA_PANEL_IS_GROUP_TASK(task) ||
		       vala_panel_group_task_has_launcher(VALA_PANEL_GROUP_TASK(task));
}

static int vala_panel_task_model_sort_func(gpointer a, gpointer b, void *user_data)
{
	ValaPanelTaskModel *self     = VALA_PANEL_TASK_MODEL(user_data);
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	if (!G_IS_OBJECT(a))
		return -1;
	else if (!G_IS_OBJECT(b))
		return 1;
	ValaPanelTask *atask = VALA_PANEL_TASK(a);
	ValaPanelTask *btask = VALA_PANEL_TASK(b);
	ValaPanelTaskState astate, bstate;
	int aout, bout;
	g_object_get(atask, VT_KEY_STATE, &astate, VT_KEY_OUTPUT, &aout, NULL);
	g_object_get(btask, VT_KEY_STATE, &bstate, VT_KEY_OUTPUT, &bout, NULL);
	/* Dock mode stuff */
	if (VALA_PANEL_IS_GROUP_TASK(atask) != VALA_PANEL_IS_GROUP_TASK(btask) && p->dock_mode)
	{
		if (VALA_PANEL_IS_GROUP_TASK(atask))
			return -1;
		if (VALA_PANEL_IS_GROUP_TASK(btask))
			return 1;
	}
	/* Launcher stuff */
	if (VALA_PANEL_IS_GROUP_TASK(atask) &&
	    (VALA_PANEL_IS_GROUP_TASK(atask) == VALA_PANEL_IS_GROUP_TASK(btask)))
	{
		ValaPanelGroupTask *agtask = VALA_PANEL_GROUP_TASK(atask);
		ValaPanelGroupTask *bgtask = VALA_PANEL_GROUP_TASK(btask);
		if (vala_panel_group_task_count_as_launcher(agtask,
		                                            p->only_minimized,
		                                            p->current_output) !=
		    vala_panel_group_task_count_as_launcher(bgtask,
		                                            p->only_minimized,
		                                            p->current_output))
		{
			return vala_panel_group_task_count_as_launcher(agtask,
			                                               p->only_minimized,
			                                               p->current_output)
			           ? (-(p->show_launchers * 2) + 1)
			           : (p->show_launchers * 2 - 1);
		}
		/* Items with launcher should go first*/
		if (vala_panel_group_task_has_launcher(agtask) !=
		    vala_panel_group_task_has_launcher(bgtask))
			return vala_panel_group_task_has_launcher(agtask) ? -1 : 1;
	}
	/* Current output stuff */
	if (aout != bout && p->current_output)
	{
		if (aout == p->current_output_num || aout == ALL_OUTPUTS)
			return -1;
		if (bout == p->current_output_num || bout == ALL_OUTPUTS)
			return 1;
	}
	/* Show minimized stuff */
	if ((vala_panel_task_is_minimized(atask) != vala_panel_task_is_minimized(btask)) &&
	    p->only_minimized)
	{
		return vala_panel_task_is_minimized(atask) ? (-(p->only_minimized * 2) + 1)
		                                           : (p->only_minimized * 2 - 1);
	}
	char *atitle, *btitle;
	g_object_get(atask, VT_KEY_TITLE, &atitle, NULL);
	g_object_get(btask, VT_KEY_TITLE, &btitle, NULL);
	int ret = g_strcmp0(atitle, btitle);
	g_clear_pointer(&atitle, g_free);
	g_clear_pointer(&btitle, g_free);
	return ret;
}

static int vala_panel_task_lookup_by_uuid(gpointer a, gpointer b, void *user_data)
{
	const char *uuid     = (const char *)user_data;
	ValaPanelTask *atask = NULL, *btask = NULL;

	if (VALA_PANEL_IS_TASK(a))
		atask = VALA_PANEL_TASK(a);
	if (VALA_PANEL_IS_TASK(b))
		btask = VALA_PANEL_TASK(b);
	char *auuid = NULL, *buuid = NULL;
	if (atask && user_data)
	{
		g_object_get(atask, VT_KEY_UUID, &auuid, NULL);
		int ret = g_strcmp0(auuid, uuid);
		g_clear_pointer(&auuid, g_free);
		return ret;
	}
	if (btask && user_data)
	{
		g_object_get(btask, VT_KEY_UUID, &buuid, NULL);
		int ret = g_strcmp0(buuid, uuid);
		g_clear_pointer(&buuid, g_free);
		return ret;
	}
	else
	{
		if (!G_IS_OBJECT(a))
			return 1;
		else if (!G_IS_OBJECT(b))
			return -1;
	}

	g_object_get(atask, VT_KEY_UUID, &auuid, NULL);
	g_object_get(btask, VT_KEY_UUID, &buuid, NULL);
	int ret = g_strcmp0(auuid, buuid);
	g_clear_pointer(&auuid, g_free);
	g_clear_pointer(&buuid, g_free);
	return ret;
}

static int vala_panel_group_task_lookup_by_app_id(gpointer a, gpointer b, void *user_data)
{
	if (!G_IS_OBJECT(a))
		return 1;
	else if (!G_IS_OBJECT(b))
		return -1;
	ValaPanelTask *atask = VALA_PANEL_TASK(a);
	ValaPanelTask *btask = VALA_PANEL_TASK(b);
	char *aid, *bid;
	g_object_get(atask, VT_KEY_APP_ID, &aid, NULL);
	g_object_get(btask, VT_KEY_APP_ID, &bid, NULL);
	int ret = -1;
	if (VALA_PANEL_IS_GROUP_TASK(atask))
		ret = g_strcmp0(aid, bid);
	g_clear_pointer(&aid, g_free);
	g_clear_pointer(&bid, g_free);
	return ret;
}

static GType vala_panel_task_model_get_item_type(G_GNUC_UNUSED GListModel *lst)
{
	return vala_panel_task_get_type();
}

static uint vala_panel_task_model_get_n_items(GListModel *lst)
{
	ValaPanelTaskModel *self     = VALA_PANEL_TASK_MODEL(lst);
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	if (!p->last_visible_iter)
		return 0;
	return (uint)g_sequence_iter_get_position(p->last_visible_iter);
}

static gpointer vala_panel_task_model_get_item(GListModel *lst, uint pos)
{
	ValaPanelTaskModel *self     = VALA_PANEL_TASK_MODEL(lst);
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	GSequenceIter *iter          = g_sequence_get_iter_at_pos(p->window_items, (int)pos);
	if (g_sequence_iter_is_end(iter))
		return NULL;
	return VALA_PANEL_TASK(g_sequence_get(iter));
}

static void vala_panel_task_model_iface_init(GListModelInterface *iface)
{
	iface->get_item_type = vala_panel_task_model_get_item_type;
	iface->get_item      = vala_panel_task_model_get_item;
	iface->get_n_items   = vala_panel_task_model_get_n_items;
}

static void vala_panel_task_model_init(ValaPanelTaskModel *self)
{
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	p->window_items              = g_sequence_new((GDestroyNotify)g_object_unref);
}

static void vala_panel_task_model_constructed(GObject *obj)
{
	g_return_if_fail(VALA_PANEL_IS_TASK_MODEL(obj));
	ValaPanelTaskModel *self     = VALA_PANEL_TASK_MODEL(obj);
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	p->last_visible_iter         = g_sequence_get_begin_iter(p->window_items);
	VALA_PANEL_TASK_MODEL_GET_CLASS(self)->start_manager(self);
	vala_panel_task_model_update_last_visible_iter(self);
	G_OBJECT_CLASS(vala_panel_task_model_parent_class)->constructed(obj);
}

static void vala_panel_task_model_destroy(GObject *obj)
{
	g_return_if_fail(VALA_PANEL_IS_TASK_MODEL(obj));
	ValaPanelTaskModel *self = VALA_PANEL_TASK_MODEL(obj);
	VALA_PANEL_TASK_MODEL_GET_CLASS(self)->stop_manager(self);
	G_OBJECT_CLASS(vala_panel_task_model_parent_class)->dispose(obj);
}

static void vala_panel_task_model_finalize(GObject *obj)
{
	ValaPanelTaskModel *self     = VALA_PANEL_TASK_MODEL(obj);
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	g_sequence_free(p->window_items);
	G_OBJECT_CLASS(vala_panel_task_model_parent_class)->finalize(obj);
}

static void vala_panel_task_model_on_destroy_task(ValaPanelTask *task, void *user_data)
{
	ValaPanelTaskModel *self     = VALA_PANEL_TASK_MODEL(user_data);
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	GSequenceIter *iter          = g_sequence_lookup(p->window_items,
                                                task,
                                                (GCompareDataFunc)vala_panel_task_lookup_by_uuid,
                                                NULL);
	GSequenceIter *group_iter =
	    g_sequence_lookup(p->window_items,
	                      task,
	                      (GCompareDataFunc)vala_panel_group_task_lookup_by_app_id,
	                      NULL);
	if (group_iter &&
	    g_strcmp0(vala_panel_task_get_uuid(task),
	              vala_panel_task_get_uuid(VALA_PANEL_TASK(g_sequence_get(group_iter)))))
		vala_panel_group_task_unlink(VALA_PANEL_GROUP_TASK(g_sequence_get(group_iter)),
		                             task);
	if (iter)
	{
		bool visible = vala_panel_task_model_is_task_visible(self, task);
		int position = g_sequence_iter_get_position(iter);
		g_sequence_remove(iter);
		if (visible)
		{
			p->last_visible_iter = g_sequence_iter_prev(p->last_visible_iter);
			g_list_model_items_changed(G_LIST_MODEL(self), position, 1, 0);
		}
	}
}

static void vala_panel_task_model_item_pos_changed(GObject *otask, GParamSpec *pspec,
                                                   void *user_data)
{
	ValaPanelTask *task          = VALA_PANEL_TASK(otask);
	ValaPanelTaskModel *self     = VALA_PANEL_TASK_MODEL(user_data);
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	GSequenceIter *iter          = g_sequence_lookup(p->window_items,
                                                task,
                                                (GCompareDataFunc)vala_panel_task_lookup_by_uuid,
                                                NULL);
	int visible_updated          = 0;
	int position                 = (iter) ? g_sequence_iter_get_position(iter) : 0;
	g_print("%s:%d of %d\n", __func__, position, g_sequence_get_length(p->window_items));
	if (iter)
	{
		if (g_sequence_iter_compare(iter, p->last_visible_iter) <= 0 &&
		    !vala_panel_task_model_is_task_visible(self, task))
			visible_updated = 1;
		else if (g_sequence_iter_compare(iter, p->last_visible_iter) > 0 &&
		         vala_panel_task_model_is_task_visible(self, task))
			visible_updated = -1;
		if (visible_updated)
			g_sequence_sort_changed(iter,
			                        (GCompareDataFunc)vala_panel_task_model_sort_func,
			                        self);
	}
	p->last_visible_iter = g_sequence_iter_move(p->last_visible_iter, visible_updated);
	if (visible_updated != 0)
		g_list_model_items_changed(G_LIST_MODEL(self),
		                           position,
		                           visible_updated < 0 ? ABS(visible_updated) : 0,
		                           visible_updated < 0 ? 0 : ABS(visible_updated));
}

static void vala_panel_task_model_update_last_visible_iter(ValaPanelTaskModel *self)
{
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	int old_len                  = g_list_model_get_n_items(G_LIST_MODEL(self));
	g_sequence_sort(p->window_items, (GCompareDataFunc)vala_panel_task_model_sort_func, self);
	for (GSequenceIter *iter = g_sequence_get_begin_iter(p->window_items);
	     !g_sequence_iter_is_end(iter);
	     iter = g_sequence_iter_next(iter))
	{
		ValaPanelTask *item = VALA_PANEL_TASK(g_sequence_get(iter));
		if (!vala_panel_task_model_is_task_visible(self, item))
		{
			p->last_visible_iter = g_sequence_iter_prev(iter);
			break;
		}
	}
	int new_len = g_list_model_get_n_items(G_LIST_MODEL(self));
	g_list_model_items_changed(G_LIST_MODEL(self), 0, old_len, new_len);
}

int vala_panel_task_model_get_current_output_num(ValaPanelTaskModel *self)
{
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	return p->current_output_num;
}

void vala_panel_task_model_change_current_output(ValaPanelTaskModel *self, int output)
{
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	p->current_output_num        = output;
	vala_panel_task_model_update_last_visible_iter(self);
}

ValaPanelTask *vala_panel_task_model_get_by_uuid(ValaPanelTaskModel *self, const char *uuid)
{
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	GSequenceIter *iter          = g_sequence_lookup(p->window_items,
                                                NULL,
                                                (GCompareDataFunc)vala_panel_task_lookup_by_uuid,
                                                (void *)uuid);
	if (!iter)
		return NULL;
	return VALA_PANEL_TASK(g_sequence_get(iter));
}

bool vala_panel_task_model_remove_task(ValaPanelTaskModel *self, const char *uuid)
{
	ValaPanelTask *task = vala_panel_task_model_get_by_uuid(self, uuid);
	if (!task || !uuid)
		return false;
	vala_panel_task_notify(task, NOTIFY_REQUEST_REMOVE);
	return true;
}

void vala_panel_task_model_add_task(ValaPanelTaskModel *self, ValaPanelTask *task)
{
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	GSequenceIter *iter =
	    g_sequence_lookup(p->window_items,
	                      task,
	                      (GCompareDataFunc)vala_panel_group_task_lookup_by_app_id,
	                      NULL);

	g_sequence_insert_sorted(p->window_items,
	                         g_object_ref_sink(task),
	                         (GCompareDataFunc)vala_panel_task_model_sort_func,
	                         self);
	if (iter && VALA_PANEL_IS_GROUP_TASK(g_sequence_get(iter)))
		vala_panel_group_task_link(VALA_PANEL_GROUP_TASK(g_sequence_get(iter)), task);
	else if (!VALA_PANEL_IS_GROUP_TASK(task))
	{
		ValaPanelGroupTask *gtask =
		    VALA_PANEL_GROUP_TASK(g_object_new(vala_panel_group_task_get_type(), NULL));
		vala_panel_group_task_link(gtask, task);
		vala_panel_task_model_add_task(self, VALA_PANEL_TASK(gtask));
	}
	g_signal_connect(task,
	                 VT_KEY_REQUEST_REMOVE,
	                 G_CALLBACK(vala_panel_task_model_on_destroy_task),
	                 (gpointer)self);
	g_signal_connect(task,
	                 "notify",
	                 G_CALLBACK(vala_panel_task_model_item_pos_changed),
	                 (gpointer)self);
	vala_panel_task_model_item_pos_changed(G_OBJECT(task), NULL, self);
}

static void vala_panel_task_model_get_property(GObject *object, guint property_id, GValue *value,
                                               GParamSpec *pspec)
{
	ValaPanelTaskModel *self     = VALA_PANEL_TASK_MODEL(object);
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	switch (property_id)
	{
	case MODEL_SHOW_LAUNCHERS:
		g_value_set_boolean(value, p->show_launchers);
		break;
	case MODEL_ONLY_MINIMIZED:
		g_value_set_boolean(value, p->only_minimized);
		break;
	case MODEL_CURRENT_OUTPUT:
		g_value_set_boolean(value, p->current_output);
		break;
	case MODEL_DOCK_MODE:
		g_value_set_boolean(value, p->dock_mode);
		break;
	case MODEL_ONLY_LAUNCHERS:
		g_value_set_boolean(value, p->only_launchers);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_task_model_set_property(GObject *object, guint property_id,
                                               const GValue *value, GParamSpec *pspec)
{
	ValaPanelTaskModel *self     = VALA_PANEL_TASK_MODEL(object);
	ValaPanelTaskModelPrivate *p = vala_panel_task_model_get_instance_private(self);
	switch (property_id)
	{
	case MODEL_SHOW_LAUNCHERS:
		p->show_launchers = g_value_get_boolean(value);
		break;
	case MODEL_ONLY_MINIMIZED:
		p->show_launchers = g_value_get_boolean(value);
		break;
	case MODEL_CURRENT_OUTPUT:
		p->show_launchers = g_value_get_boolean(value);
		break;
	case MODEL_DOCK_MODE:
		p->show_launchers = g_value_get_boolean(value);
		break;
	case MODEL_ONLY_LAUNCHERS:
		p->show_launchers = g_value_get_boolean(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
	vala_panel_task_model_update_last_visible_iter(self);
}

static void vala_panel_task_model_class_init(ValaPanelTaskModelClass *klass)
{
	GObjectClass *oclass = G_OBJECT_CLASS(klass);
	oclass->constructed  = vala_panel_task_model_constructed;
	oclass->set_property = vala_panel_task_model_set_property;
	oclass->get_property = vala_panel_task_model_get_property;
	oclass->dispose      = vala_panel_task_model_destroy;
	oclass->finalize     = vala_panel_task_model_finalize;
	model_specs[MODEL_SHOW_LAUNCHERS] =
	    g_param_spec_boolean(VTM_KEY_SHOW_LAUNCHERS,
	                         VTM_KEY_SHOW_LAUNCHERS,
	                         VTM_KEY_SHOW_LAUNCHERS,
	                         false,
	                         G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	model_specs[MODEL_ONLY_LAUNCHERS] =
	    g_param_spec_boolean(VTM_KEY_ONLY_LAUNCHERS,
	                         VTM_KEY_ONLY_LAUNCHERS,
	                         VTM_KEY_ONLY_LAUNCHERS,
	                         false,
	                         G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	model_specs[MODEL_CURRENT_OUTPUT] =
	    g_param_spec_boolean(VTM_KEY_CURRENT_OUTPUT,
	                         VTM_KEY_CURRENT_OUTPUT,
	                         VTM_KEY_CURRENT_OUTPUT,
	                         false,
	                         G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	model_specs[MODEL_DOCK_MODE] =
	    g_param_spec_boolean(VTM_KEY_DOCK_MODE,
	                         VTM_KEY_DOCK_MODE,
	                         VTM_KEY_DOCK_MODE,
	                         false,
	                         G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	model_specs[MODEL_ONLY_MINIMIZED] =
	    g_param_spec_boolean(VTM_KEY_ONLY_MINIMIZED,
	                         VTM_KEY_ONLY_MINIMIZED,
	                         VTM_KEY_ONLY_MINIMIZED,
	                         false,
	                         G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	g_object_class_install_properties(oclass, MODEL_LAST, model_specs);
}
