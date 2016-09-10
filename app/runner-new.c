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
#include <gio/gdesktopappinfo.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <string.h>

#include "lib/c-lib/css.h"
#include "lib/c-lib/launcher.h"
#include "lib/definitions.h"
#include "runner-new.h"

struct _ValaPanelRunner
{
	GtkDialog __parent__;
	GtkEntry *main_entry;
	GtkToggleButton *terminal_button;
	GtkBox *main_box;
	GTask *task;
	GCancellable *cancellable;
	bool cached;
};

G_DEFINE_TYPE(ValaPanelRunner, vala_panel_runner, GTK_TYPE_DIALOG)

static GDesktopAppInfo *match_app_by_exec(const char *exec)
{
	GDesktopAppInfo *ret       = NULL;
	g_autofree char *exec_path = g_find_program_in_path(exec);
	const char *pexec;
	int path_len, exec_len, len;

	if (!exec_path)
		return NULL;

	path_len                  = strlen(exec_path);
	exec_len                  = strlen(exec);
	g_autoptr(GList) app_list = g_app_info_get_all();
	for (GList *l = app_list; l; l = l->next)
	{
		GAppInfo *app        = G_APP_INFO(l->data);
		const char *app_exec = g_app_info_get_executable(app);
		if (!app_exec)
			continue;
		if (g_path_is_absolute(app_exec))
		{
			pexec = exec_path;
			len   = path_len;
		}
		else
		{
			pexec = exec;
			len   = exec_len;
		}

		if (strncmp(app_exec, pexec, len) == 0)
		{
			/* exact match has the highest priority */
			if (app_exec[len] == '\0')
			{
				ret = G_DESKTOP_APP_INFO(app);
				break;
			}
		}
	}

	/* if this is a symlink */
	if (!ret && g_file_test(exec_path, G_FILE_TEST_IS_SYMLINK))
	{
		char target[512]; /* FIXME: is this enough? */
		len = readlink(exec_path, target, sizeof(target) - 1);
		if (len > 0)
		{
			target[len] = '\0';
			ret         = match_app_by_exec(target);
			if (!ret)
			{
				/* FIXME: Actually, target could be relative paths.
				 *        So, actually path resolution is needed here. */
				g_autofree char *basename = g_path_get_basename(target);
				g_autofree char *locate   = g_find_program_in_path(basename);
				if (locate && strcmp(locate, target) == 0)
					ret = match_app_by_exec(basename);
			}
		}
	}
	return ret;
}

static void setup_auto_complete_with_data(GObject *source_object, GAsyncResult *res,
                                          gpointer user_data)
{
	ValaPanelRunner *self = VALA_PANEL_RUNNER(user_data);
	GSList *l;
	g_autoptr(GSList) files  = (GSList *)g_task_propagate_pointer(self->task, NULL);
	GtkEntryCompletion *comp = gtk_entry_get_completion(self->main_entry);
	GtkListStore *store      = GTK_LIST_STORE(gtk_entry_completion_get_model(comp));

	for (l = files; l; l = l->next)
	{
		char *name = (char *)l->data;
		gtk_list_store_insert_with_values(store, NULL, -1, 0, name, -1);
	}
	/* trigger entry completion */
	gtk_entry_completion_complete(comp);
}

static void slist_destroy_notify(void *a)
{
	GSList *lst = (GSList *)a;
	g_slist_free_full(lst, g_free);
}

static void vala_panel_runner_create_file_list(GTask *task, void *source, void *task_data,
                                               GCancellable *cancellable)
{
	GSList *list       = NULL;
	const char *var    = g_getenv("PATH");
	g_auto(GStrv) dirs = g_strsplit(var, ":", 0);
	for (int i = 0; dirs[i] != NULL; i++)
	{
		if (g_cancellable_is_cancelled(cancellable))
			return;

		g_autoptr(GDir) gdir = g_dir_open(dirs[i], 0, NULL);
		const char *name     = NULL;
		while (!g_cancellable_is_cancelled(cancellable) &&
		       (name = g_dir_read_name(gdir)) != NULL)
		{
			g_autofree char *filename = g_build_filename(dirs[i], name, NULL);
			if (g_file_test(filename, G_FILE_TEST_IS_EXECUTABLE))
			{
				if (g_cancellable_is_cancelled(cancellable))
					return;
				if (g_slist_find_custom(list, name, (GCompareFunc)strcmp) == NULL)
					list = g_slist_append(list, g_strdup(name));
			}
		}
	}
	g_task_return_pointer(task, list, slist_destroy_notify);
	return;
}

static void setup_entry_completion(ValaPanelRunner *self)
{
	/* FIXME: consider saving the list of commands as on-disk cache. */
	if (self->cached)
	{
		/* load cached program list */
	}
	else
	{
		self->cancellable = g_cancellable_new();
		self->task =
		    g_task_new(self, self->cancellable, setup_auto_complete_with_data, self);
		/* load in another working thread */
		g_task_run_in_thread(self->task, vala_panel_runner_create_file_list);
	}
}

static void vala_panel_runner_response(GtkDialog *dlg, gint response)
{
	ValaPanelRunner *self = VALA_PANEL_RUNNER(dlg);
	if (G_LIKELY(response == GTK_RESPONSE_OK))
	{
		GError *err = NULL;
		g_autoptr(GAppInfo) app_info =
		    g_app_info_create_from_commandline(gtk_entry_get_text(self->main_entry),
		                                       NULL,
		                                       gtk_toggle_button_get_active(
		                                           self->terminal_button)
		                                           ? G_APP_INFO_CREATE_NEEDS_TERMINAL
		                                           : G_APP_INFO_CREATE_NONE,
		                                       &err);
		if (err)
		{
			g_error_free(err);
			g_signal_stop_emission_by_name(dlg, "response");
			return;
		}
		bool launch =
		    vala_panel_launch(G_DESKTOP_APP_INFO(app_info), NULL, GTK_WIDGET(dlg));
		if (!launch)
		{
			g_signal_stop_emission_by_name(dlg, "response");
			return;
		}
	}
	g_cancellable_cancel(self->cancellable);
	gtk_widget_destroy((GtkWidget *)dlg);
}

static void on_entry_changed(GtkEntry *entry, gpointer user_data)
{
	const char *str      = gtk_entry_get_text(entry);
	GDesktopAppInfo *app = NULL;
	if (str && *str)
		app = match_app_by_exec(str);

	if (app)
	{
		gtk_entry_set_icon_from_gicon(entry,
		                              GTK_ENTRY_ICON_PRIMARY,
		                              g_app_info_get_icon(G_APP_INFO(app)));
	}
	else
	{
		gtk_entry_set_icon_from_icon_name(entry,
		                                  GTK_ENTRY_ICON_PRIMARY,
		                                  "application-x-executable");
	}
}

static void on_entry_activated(GtkEntry *entry, GtkDialog *dlg)
{
	gtk_dialog_response(dlg, GTK_RESPONSE_OK);
}

static void on_icon_activated(GtkEntry *entry, GtkEntryIconPosition pos, GdkEventButton *event,
                              GtkDialog *dlg)
{
	if (pos == GTK_ENTRY_ICON_SECONDARY)
	{
		if (event->button == 1)
			gtk_dialog_response(dlg, GTK_RESPONSE_OK);
	}
}

static void vala_panel_runner_init(ValaPanelRunner *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
	css_apply_from_resource(GTK_WIDGET(self),
	                        "/org/vala-panel/app/style.css",
	                        "-panel-run-dialog");
	g_autoptr(GtkStyleContext) ctx = gtk_widget_get_style_context(GTK_WIDGET(self->main_box));
	gtk_style_context_add_class(ctx, "-panel-run-header");
	// FIXME: Implement cache
	self->cached = false;
	gtk_widget_set_visual(GTK_WIDGET(self),
	                      gdk_screen_get_rgba_visual(gtk_widget_get_screen(GTK_WIDGET(self))));
	gtk_dialog_set_default_response(GTK_DIALOG(self), GTK_RESPONSE_OK);
	gtk_window_set_keep_above(GTK_WINDOW(self), true);
	self->task        = NULL;
	self->cancellable = NULL;
}

static void vala_panel_runner_finalize(GObject *obj)
{
	ValaPanelRunner *self =
	    G_TYPE_CHECK_INSTANCE_CAST(obj, vala_panel_runner_get_type(), ValaPanelRunner);
	g_cancellable_cancel(self->cancellable);
	gtk_window_set_application((GtkWindow *)self, NULL);
	g_object_unref0(self->main_entry);
	g_object_unref0(self->terminal_button);
	g_object_unref0(self->main_box);
	g_object_unref0(self->task);
	g_object_unref0(self->cancellable);
	G_OBJECT_CLASS(vala_panel_runner_parent_class)->finalize(obj);
}

static void vala_panel_runner_class_init(ValaPanelRunnerClass *klass)
{
	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass),
	                                            "/org/vala-panel/app/app-runner.ui");
	vala_panel_runner_parent_class  = g_type_class_peek_parent(klass);
	G_OBJECT_CLASS(klass)->finalize = vala_panel_runner_finalize;
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "main-entry",
	                                          false,
	                                          G_STRUCT_OFFSET(ValaPanelRunner, main_entry));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "terminal-button",
	                                          false,
	                                          G_STRUCT_OFFSET(ValaPanelRunner,
	                                                          terminal_button));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "main-box",
	                                          true,
	                                          G_STRUCT_OFFSET(ValaPanelRunner, main_box));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_entry_changed",
	                                             G_CALLBACK(on_entry_changed));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_entry_activated",
	                                             G_CALLBACK(on_entry_activated));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_icon_activated",
	                                             G_CALLBACK(on_icon_activated));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "vala_panel_runner_response",
	                                             G_CALLBACK(vala_panel_runner_response));
}

void gtk_run(ValaPanelRunner *self)
{
	setup_entry_completion(self);
	gtk_widget_show_all(GTK_WIDGET(self));
	gtk_widget_grab_focus(GTK_WIDGET(self->main_entry));
	gtk_window_present_with_time(GTK_WINDOW(self), gtk_get_current_event_time());
}

ValaPanelRunner *vala_panel_runner_new(GtkApplication *app)
{
	return VALA_PANEL_RUNNER(
	    g_object_new(vala_panel_runner_get_type(), "application", app, NULL));
}
