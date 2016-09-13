#include "runner.h"
#include "lib/c-lib/css.h"
#include "lib/c-lib/launcher.h"
#include "lib/definitions.h"
#include <gio/gdesktopappinfo.h>
#include <stdbool.h>
#include <string.h>

struct _ValaPanelRunner
{
	GtkDialog __parent__;
	GtkSettings *settings;
	GtkCssProvider *css_provider;
	char *current_theme_uri;
	GtkRevealer *bottom_revealer;
	GtkListBox *app_box;
	GtkSearchEntry *main_entry;
	GtkWidget *bootstrap_row;
	GtkLabel *bootstrap_label;
	GtkToggleButton *terminal_button;
	GTask *task;
	GCancellable *cancellable;
	bool cached;
};

G_DEFINE_TYPE(ValaPanelRunner, vala_panel_runner, GTK_TYPE_DIALOG);
#define BUTTON_QUARK g_quark_from_static_string("button-id")
#define g_app_launcher_button_get_info_data(btn)                                                   \
	(InfoData *)g_object_get_qdata(G_OBJECT(btn), BUTTON_QUARK)
#define g_app_launcher_button_set_info_data(btn, info)                                             \
	g_object_set_qdata_full(G_OBJECT(btn),                                                     \
	                        BUTTON_QUARK,                                                      \
	                        (gpointer)info,                                                    \
	                        (GDestroyNotify)info_data_free)
void create_bootstrap(ValaPanelRunner *self);
/*
 * InfoData struct
 */

static char *generate_markup(const char *name, const char *sdesc)
{
	g_autofree char *nom  = g_markup_escape_text(name, strlen(name));
	g_autofree char *desc = g_markup_escape_text(sdesc, strlen(sdesc));
	return g_strdup_printf("<big>%s</big>\n<small>%s</small>", nom, desc);
}

typedef struct info_data
{
	GIcon *icon;
	char *name_markup;
	char *disp_name;
	char *command;
	bool free_icon;
} InfoData;

static InfoData *info_data_new_from_info(GAppInfo *info)
{
	InfoData *data  = (InfoData *)g_malloc0(sizeof(InfoData));
	data->icon      = g_app_info_get_icon(info);
	data->free_icon = false;
	data->disp_name = g_strdup(g_app_info_get_display_name(info));
	const char *name =
	    g_app_info_get_name(info) ? g_app_info_get_name(info) : g_app_info_get_executable(info);
	const char *sdesc =
	    g_app_info_get_description(info) ? g_app_info_get_description(info) : "";
	data->name_markup = generate_markup(name, sdesc);
	data->command     = g_strdup(g_app_info_get_executable(info));
	return data;
}

static InfoData *info_data_new_from_command(const char *command)
{
	InfoData *data        = (InfoData *)g_malloc0(sizeof(InfoData));
	data->icon            = g_themed_icon_new_with_default_fallbacks("system-run-symbolic");
	data->free_icon       = true;
	data->disp_name       = g_strdup_printf(_("Run %s"), command);
	g_autofree char *name = g_strdup_printf(_("Run %s"), command);
	const char *sdesc     = _("Run system command");
	data->name_markup     = generate_markup(name, sdesc);
	data->command         = g_strdup(command);
	return data;
}

static void info_data_free(InfoData *data)
{
	if (data->free_icon)
		g_object_unref(data->icon);
	g_free(data->disp_name);
	g_free(data->name_markup);
	g_free(data->command);
	g_free(data);
}

void add_application(InfoData *data, ValaPanelRunner *self);

GtkWidget *create_widget_func(const InfoData *data)
{
	GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
	g_app_launcher_button_set_info_data(box, data);
	gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(box)),
	                            "launcher-button");
	GtkImage *image = GTK_IMAGE(gtk_image_new_from_gicon(data->icon, GTK_ICON_SIZE_DIALOG));
	gtk_image_set_pixel_size(image, 48);
	gtk_widget_set_margin_start(GTK_WIDGET(image), 8);
	gtk_box_pack_start(box, GTK_WIDGET(image), false, false, 0);
	GtkLabel *label = GTK_LABEL(gtk_label_new(data->name_markup));
	gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(label)), "dim-label");
	gtk_label_set_line_wrap(label, true);
	g_object_set(label, "xalign", 0.0, NULL);
	gtk_label_set_use_markup(label, true);
	gtk_widget_set_margin_start(GTK_WIDGET(label), 12);
	gtk_label_set_max_width_chars(label, 60);
	gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
	gtk_box_pack_start(box, GTK_WIDGET(label), false, false, 0);
	gtk_widget_set_hexpand(GTK_WIDGET(box), false);
	gtk_widget_set_vexpand(GTK_WIDGET(box), false);
	gtk_widget_set_halign(GTK_WIDGET(box), GTK_ALIGN_START);
	gtk_widget_set_valign(GTK_WIDGET(box), GTK_ALIGN_START);
	gtk_widget_set_tooltip_text(GTK_WIDGET(box), data->disp_name);
	gtk_widget_set_margin_top(GTK_WIDGET(box), 3);
	gtk_widget_set_margin_bottom(GTK_WIDGET(box), 3);
	return GTK_WIDGET(box);
}

/*
 * Main functions
 */

static void vala_panel_runner_response(GtkDialog *dlg, gint response)
{
	ValaPanelRunner *self = VALA_PANEL_RUNNER(dlg);
	if (G_LIKELY(response == GTK_RESPONSE_ACCEPT))
	{
		GtkWidget *active_row =
		    gtk_bin_get_child(GTK_BIN(gtk_list_box_get_selected_row(self->app_box)));
		g_autoptr(GAppInfo) app_info = NULL;
		if (active_row == self->bootstrap_row)
		{
			GError *err = NULL;
			app_info    = g_app_info_create_from_commandline(
			    gtk_entry_get_text(GTK_ENTRY(self->main_entry)),
			    NULL,
			    gtk_toggle_button_get_active(self->terminal_button)
			        ? G_APP_INFO_CREATE_NEEDS_TERMINAL
			        : G_APP_INFO_CREATE_NONE,
			    &err);
			if (err)
			{
				g_error_free(err);
				g_signal_stop_emission_by_name(dlg, "response");
				return;
			}
		}
		else
		{
			InfoData *data = g_app_launcher_button_get_info_data(active_row);
			app_info       = g_app_info_create_from_commandline(
			    data->command,
			    NULL,
			    gtk_toggle_button_get_active(self->terminal_button)
			        ? G_APP_INFO_CREATE_NEEDS_TERMINAL
			        : G_APP_INFO_CREATE_NONE,
			    NULL);
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

/**
 * Filter the list
 */
bool on_filter(GtkListBoxRow *row, ValaPanelRunner *self)
{
	InfoData *info = g_app_launcher_button_get_info_data(gtk_bin_get_child(GTK_BIN(row)));
	//        g_autofree char* disp_name = g_utf8_strdown(g_app_info_get_display_name(info),-1);
	const char *search_text = gtk_entry_get_text(GTK_ENTRY(self->main_entry));
	const char *match       = info ? info->command : NULL;
	if (!strcmp(search_text, ""))
		return false;
	else if (!(match) && (gtk_bin_get_child(GTK_BIN(row)) == self->bootstrap_row))
		return true;
	else if (g_strstr_len(match, -1, search_text))
		return true;
	return false;
}

void on_entry_changed(GtkSearchEntry *ent, ValaPanelRunner *self)
{
	gtk_list_box_invalidate_filter(self->app_box);
	GtkWidget *active_row = NULL;
	GList *chl            = gtk_container_get_children(GTK_CONTAINER(self->app_box));
	for (GList *l = chl; l != NULL; l = g_list_next(l))
	{
		GtkWidget *row = GTK_WIDGET(l->data);
		if (gtk_widget_get_visible(row) && gtk_widget_get_child_visible(row))
		{
			active_row = row;
			break;
		}
	}

	if (active_row == NULL)
	{
		gtk_revealer_set_transition_type(self->bottom_revealer,
		                                 GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
		gtk_revealer_set_reveal_child(self->bottom_revealer, false);
	}
	else
	{
		gtk_revealer_set_transition_type(self->bottom_revealer,
		                                 GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
		gtk_revealer_set_reveal_child(self->bottom_revealer, true);
		gtk_list_box_select_row(self->app_box, GTK_LIST_BOX_ROW(active_row));
	}
}

void add_application(InfoData *data, ValaPanelRunner *self)
{
	GtkWidget *button = create_widget_func(data);
	gtk_container_add(GTK_CONTAINER(self->app_box), button);
	gtk_widget_show_all(button);
}

static void setup_list_box_with_data(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	ValaPanelRunner *self = VALA_PANEL_RUNNER(user_data);
	GSList *l;
	GSList *files = (GSList *)g_task_propagate_pointer(self->task, NULL);

	for (l = files; l; l = l->next)
		add_application((InfoData *)l->data, self);
	gtk_list_box_set_filter_func(self->app_box, (GtkListBoxFilterFunc)on_filter, self, NULL);
	g_slist_free(files);
}

static void slist_destroy_notify(void *a)
{
	GSList *lst = (GSList *)a;
	g_slist_free_full(lst, (GDestroyNotify)info_data_free);
}

static int slist_find_func(gconstpointer slist, gconstpointer data)
{
	const char *str = (const char *)data;
	InfoData *info  = (InfoData *)slist;
	return strcmp(info->command, str);
}

static void vala_panel_runner_create_data_list(GTask *task, void *source, void *task_data,
                                               GCancellable *cancellable)
{
	GSList *obj_list          = NULL;
	g_autoptr(GList) app_list = g_app_info_get_all();
	for (GList *l = app_list; l; l = g_list_next(l))
	{
		if (g_cancellable_is_cancelled(cancellable))
			return;
		obj_list = g_slist_append(obj_list, info_data_new_from_info(G_APP_INFO(l->data)));
	}
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
				if (g_slist_find_custom(obj_list, name, slist_find_func) == NULL)
				{
					if (g_cancellable_is_cancelled(cancellable))
						return;
					InfoData *info = info_data_new_from_command(name);
					obj_list       = g_slist_append(obj_list, info);
				}
			}
		}
	}
	g_task_return_pointer(task, obj_list, slist_destroy_notify);
	return;
}

static void build_app_box(ValaPanelRunner *self)
{
	create_bootstrap(self);
	gtk_container_add(GTK_CONTAINER(self->app_box), self->bootstrap_row);
	/* FIXME: consider saving the list of commands as on-disk cache. */
	if (self->cached)
	{
		/* load cached program list */
	}
	else
	{
		self->cancellable = g_cancellable_new();
		self->task = g_task_new(self, self->cancellable, setup_list_box_with_data, self);
		/* load in another working thread */
		g_task_run_in_thread(self->task, vala_panel_runner_create_data_list);
	}
}

/**
 * Handle click/<enter> activation on the main list
 */
void on_row_activated(GtkListBoxRow *row, ValaPanelRunner *self)
{
	gtk_dialog_response(GTK_DIALOG(self), GTK_RESPONSE_ACCEPT);
}

/**
 * Handle click/<enter> activation on the entry
 */
void on_entry_activated(GtkEntry *row, ValaPanelRunner *self)
{
	gtk_dialog_response(GTK_DIALOG(self), GTK_RESPONSE_ACCEPT);
}

static void vala_panel_runner_finalize(GObject *obj)
{
	ValaPanelRunner *self =
	    G_TYPE_CHECK_INSTANCE_CAST(obj, vala_panel_runner_get_type(), ValaPanelRunner);
	gtk_window_set_application((GtkWindow *)self, NULL);
	g_cancellable_cancel(self->cancellable);
	g_object_unref0(self->cancellable);
	g_object_unref0(self->task);
	g_object_unref0(self->bootstrap_row);
	g_object_unref0(self->bootstrap_label);
	g_object_unref0(self->main_entry);
	g_object_unref0(self->bottom_revealer);
	g_object_unref0(self->app_box);
	g_object_unref0(self->terminal_button);
	G_OBJECT_CLASS(vala_panel_runner_parent_class)->finalize(obj);
}

void vala_panel_runner_init(ValaPanelRunner *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
	css_apply_from_resource(GTK_WIDGET(self),
	                        "/org/vala-panel/runner/style.css",
	                        "-panel-run-dialog");
	build_app_box(self);
}

void vala_panel_runner_class_init(ValaPanelRunnerClass *klass)
{
	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass),
	                                            "/org/vala-panel/runner/app-runner.ui");
	vala_panel_runner_parent_class  = g_type_class_peek_parent(klass);
	G_OBJECT_CLASS(klass)->finalize = vala_panel_runner_finalize;
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "main-entry",
	                                          false,
	                                          G_STRUCT_OFFSET(ValaPanelRunner, main_entry));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "search-box",
	                                          false,
	                                          G_STRUCT_OFFSET(ValaPanelRunner, app_box));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "terminal-button",
	                                          false,
	                                          G_STRUCT_OFFSET(ValaPanelRunner,
	                                                          terminal_button));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "revealer",
	                                          true,
	                                          G_STRUCT_OFFSET(ValaPanelRunner,
	                                                          bottom_revealer));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_search_changed",
	                                             G_CALLBACK(on_entry_changed));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_search_activated",
	                                             G_CALLBACK(on_entry_activated));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "vala_panel_runner_response",
	                                             G_CALLBACK(vala_panel_runner_response));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_row_activated",
	                                             G_CALLBACK(on_row_activated));
}

ValaPanelRunner *vala_panel_runner_new(GtkApplication *app)
{
	return VALA_PANEL_RUNNER(
	    g_object_new(vala_panel_runner_get_type(), "application", app, NULL));
}
void create_bootstrap(ValaPanelRunner *self)
{
	GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
	gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(box)),
	                            "launcher-button");
	GtkImage *image =
	    GTK_IMAGE(gtk_image_new_from_icon_name("system-run-symbolic", GTK_ICON_SIZE_DIALOG));
	gtk_image_set_pixel_size(image, 48);
	gtk_widget_set_margin_start(GTK_WIDGET(image), 8);
	gtk_box_pack_start(box, GTK_WIDGET(image), false, false, 0);
	self->bootstrap_row   = GTK_WIDGET(box);
	self->bootstrap_label = GTK_LABEL(gtk_label_new(""));
	gtk_box_pack_start(box, GTK_WIDGET(self->bootstrap_label), false, false, 0);
	g_object_bind_property(self->main_entry,
	                       "text",
	                       self->bootstrap_label,
	                       "label",
	                       G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
}

void gtk_run(ValaPanelRunner *self)
{
	gtk_widget_show_all(GTK_WIDGET(self));
	gtk_widget_grab_focus(GTK_WIDGET(self->main_entry));
	gtk_window_present_with_time(GTK_WINDOW(self), gtk_get_current_event_time());
}
