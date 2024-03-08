/*
 * vala-panel
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
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

#include "config.h"
#include "version.h"

#include "application.h"
#include "panel-layout.h"
#include "server.h"
#include "vala-panel-platform-standalone-layer-shell.h"
#include "vala-panel-platform-standalone-x11.h"

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

#include <glib/gi18n.h>
#include <locale.h>
#include <stdbool.h>

#define PROFILE_TR N_("Use specified profile")
#define COMMAND_DES_TR N_("Run command on already opened panel")
#define REMOTE_DES_TR N_("Run remote command on panel applet")
#define DEFAULT_PROFILE "default"

#define VALA_PANEL_KEY_LOCK "lock-command"
#define VALA_PANEL_KEY_RUN "run-command"
#define VALA_PANEL_KEY_LOGOUT "logout-command"
#define VALA_PANEL_KEY_SHUTDOWN "shutdown-command"
#define VALA_PANEL_KEY_TERMINAL "terminal-command"
#define VALA_PANEL_KEY_DARK "is-dark"
#define VALA_PANEL_KEY_CUSTOM "is-custom"
#define VALA_PANEL_KEY_CSS "css"

struct _ValaPanelApplication
{
	GtkApplication parent;
	bool restart;
	bool dark;
	bool custom;
	GSettings *config;
	char *css;
	ValaPanelPlatform *platform;
	GtkCssProvider *provider;
	char *profile;
	char *lock_command;
	char *run_command;
	char *terminal_command;
	char *logout_command;
	char *shutdown_command;
};

G_DEFINE_TYPE(ValaPanelApplication, vala_panel_application, GTK_TYPE_APPLICATION)

static void activate_menu(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_panel_preferences(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_preferences(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_about(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_run(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_lock(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_logout(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_shutdown(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_exit(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_restart(GSimpleAction *simple, GVariant *param, gpointer data);

static const GOptionEntry entries[] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, NULL, N_("Print version and exit"), NULL },
	{ "profile",
	  'p',
	  0,
	  G_OPTION_ARG_STRING,
	  NULL,
	  PROFILE_TR,
	  N_("Profile is a config file name. Default is \"default\"") },
	{ "command",
	  'c',
	  0,
	  G_OPTION_ARG_STRING,
	  NULL,
	  COMMAND_DES_TR,
	  N_("A command for main vala-panel executable.") },
	{ "remote-command",
	  'r',
	  0,
	  G_OPTION_ARG_STRING,
	  NULL,
	  REMOTE_DES_TR,
	  N_("A command for a plugin.") },
	{ NULL }
};

static const GActionEntry vala_panel_application_app_entries[10] = {
	{ "preferences", activate_preferences, NULL, NULL, NULL, { 0 } },
	{ "panel-preferences", activate_panel_preferences, "s", NULL, NULL, { 0 } },
	{ "about", activate_about, NULL, NULL, NULL, { 0 } },
	{ "menu", activate_menu, NULL, NULL, NULL, { 0 } },
	{ "run", activate_run, NULL, NULL, NULL, { 0 } },
	{ "lock", activate_lock, NULL, NULL, NULL, { 0 } },
	{ "logout", activate_logout, NULL, NULL, NULL, { 0 } },
	{ "shutdown", activate_shutdown, NULL, NULL, NULL, { 0 } },
	{ "quit", activate_exit, NULL, NULL, NULL, { 0 } },
	{ "restart", activate_restart, NULL, NULL, NULL, { 0 } },
};
static const GActionEntry vala_panel_application_menu_entries[3] = {
	{ "launch-id", activate_menu_launch_id, "s", NULL, NULL, { 0 } },
	{ "launch-uri", activate_menu_launch_uri, "s", NULL, NULL, { 0 } },
	{ "launch-command", activate_menu_launch_command, "s", NULL, NULL, { 0 } }
};

enum
{
	APP_DUMMY_PROPERTY,
	APP_PROFILE,
	APP_RUN_COMMAND,
	APP_TERMINAL_COMMAND,
	APP_LOCK_COMMAND,
	APP_LOGOUT_COMMAND,
	APP_SHUTDOWN_COMMAND,
	APP_IS_DARK,
	APP_IS_CUSTOM,
	APP_CSS,
	APP_ALL
};

static GParamSpec *app_specs[APP_ALL];

static inline void destroy0(GtkWidget *x, G_GNUC_UNUSED void *data)
{
	g_clear_pointer(&x, gtk_widget_destroy);
}

static void apply_styling(ValaPanelApplication *app)
{
	if (gtk_settings_get_default() != NULL)
		g_object_set(gtk_settings_get_default(),
		             "gtk-application-prefer-dark-theme",
		             app->dark,
		             NULL);
	if (app->custom)
	{
		if (app->provider)
		{
			gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(),
			                                             GTK_STYLE_PROVIDER(
			                                                 app->provider));
			g_clear_object(&app->provider);
		}
		app->provider = css_apply_from_file_to_app_with_provider_and_priority(
		    app->css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	}
	else if (app->provider)
	{
		gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(),
		                                             GTK_STYLE_PROVIDER(app->provider));
		g_clear_object(&app->provider);
	}
}

ValaPanelApplication *vala_panel_application_new(void)
{
	return (ValaPanelApplication *)g_object_new(vala_panel_application_get_type(),
	                                            "application-id",
	                                            "org.valapanel.application",
	                                            "flags",
	                                            G_APPLICATION_HANDLES_COMMAND_LINE,
	                                            "resource-base-path",
	                                            "/org/vala-panel/app",
	                                            NULL);
}

static void vala_panel_application_init(ValaPanelApplication *self)
{
	self->restart  = false;
	self->profile  = g_strdup(DEFAULT_PROFILE);
	self->provider = NULL;
	g_application_add_main_option_entries(G_APPLICATION(self), entries);
}

static void vala_panel_application_startup(GApplication *base)
{
	ValaPanelApplication *self = (ValaPanelApplication *)base;
	G_APPLICATION_CLASS(vala_panel_application_parent_class)->startup(base);
	g_application_mark_busy((GApplication *)self);
	setlocale(LC_CTYPE, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
	gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), DATADIR "/images");
	g_action_map_add_action_entries((GActionMap *)self,
	                                vala_panel_application_app_entries,
	                                G_N_ELEMENTS(vala_panel_application_app_entries),
	                                self);
	g_action_map_add_action_entries((GActionMap *)self,
	                                vala_panel_application_menu_entries,
	                                G_N_ELEMENTS(vala_panel_application_menu_entries),
	                                self);
}

static void vala_panel_application_shutdown(GApplication *base)
{
	g_autoptr(GSList) list = NULL;
	GList *lst             = gtk_application_get_windows(GTK_APPLICATION(base));
	for (GList *il = lst; il != NULL; il = il->next)
		list = g_slist_append(list, il->data);
	for (GSList *il = list; il != NULL; il = il->next)
	{
		GtkWindow *w = GTK_WINDOW(il->data);
		gtk_window_set_application(w, NULL);
	}
	G_APPLICATION_CLASS(vala_panel_application_parent_class)->shutdown(base);
	if (VALA_PANEL_APPLICATION(base)->restart)
	{
		g_autoptr(GError) err = NULL;
		char cwd[PATH_MAX];
		char *cwdp         = getcwd(cwd, PATH_MAX);
		const char *argv[] = {
			GETTEXT_PACKAGE, "-p", VALA_PANEL_APPLICATION(base)->profile, NULL
		};
		g_auto(GStrv) envp = g_get_environ();
		g_spawn_async(cwdp,
		              (GStrv)argv,
		              envp,
		              G_SPAWN_SEARCH_PATH,
		              child_spawn_func,
		              NULL,
		              NULL,
		              &err);
		if (err)
			g_critical("%s\n", err->message);
	}
}

static gint vala_panel_app_handle_local_options(G_GNUC_UNUSED GApplication *application,
                                                GVariantDict *options)
{
	if (g_variant_dict_contains(options, "version"))
	{
		g_print(_("%s - Version %s\n"), g_get_application_name(), VERSION);
		return 0;
	}
	return -1;
}

static int vala_panel_app_command_line(GApplication *application,
                                       GApplicationCommandLine *commandline)
{
	g_autofree char *profile_name = NULL;
	g_autofree char *ccommand     = NULL;
	g_autofree char *cremote      = NULL;
	GVariantDict *options         = g_application_command_line_get_options_dict(commandline);
	if (g_variant_dict_lookup(options, "profile", "s", &profile_name))
		g_object_set(G_OBJECT(application), "profile", profile_name, NULL);
	if (g_variant_dict_lookup(options, "command", "s", &ccommand))
	{
		g_autofree char *name     = NULL;
		g_autoptr(GVariant) param = NULL;
		g_autoptr(GError) err     = NULL;
		g_action_parse_detailed_name(ccommand, &name, &param, &err);
		if (err)
			g_warning("%s\n", err->message);
		else if (g_action_map_lookup_action(G_ACTION_MAP(application), name))
			g_action_group_activate_action(G_ACTION_GROUP(application), name, param);
		else
		{
			g_auto(GStrv) listv =
			    g_action_group_list_actions(G_ACTION_GROUP(application));
			g_autofree char *list = g_strjoinv(" ", listv);
			g_application_command_line_printerr(
			    commandline,
			    _("%s: invalid command - %s. Doing nothing.\nValid commands: %s\n"),
			    g_get_application_name(),
			    ccommand,
			    list);
		}
	}
	if (g_variant_dict_lookup(options, "remote-command", "s", &cremote))
	{
		g_autofree char *uuid            = NULL;
		g_autoptr(GVariant) command_name = NULL;
		g_autoptr(GError) err            = NULL;
		g_action_parse_detailed_name(cremote, &uuid, &command_name, &err);
		if (err)
			g_warning("%s\n", err->message);
		const char *command_str = g_variant_get_string(command_name, NULL);
		GtkApplication *app     = GTK_APPLICATION(application);
		GList *windows          = gtk_application_get_windows(app);
		for (GList *l = windows; l != NULL; l = l->next)
		{
			if (VALA_PANEL_IS_TOPLEVEL(l->data))
			{
				ValaPanelLayout *layout =
				    vala_panel_toplevel_get_layout(VALA_PANEL_TOPLEVEL(l->data));
				GList *applets = vala_panel_layout_get_applets_list(layout);
				for (GList *il = applets; il != NULL; il = il->next)
				{
					const char *gotten_uuid =
					    vala_panel_applet_get_uuid(VALA_PANEL_APPLET(il->data));
					if (!g_strcmp0(uuid, gotten_uuid))
						vala_panel_applet_remote_command(VALA_PANEL_APPLET(
						                                     il->data),
						                                 command_str);
				}
				g_list_free(applets);
			}
		}
	}
	g_application_activate(application);
	return 0;
}

static bool load_settings(ValaPanelApplication *app)
{
	g_autofree char *file         = g_build_filename(PROFILES, app->profile, NULL);
	g_autofree char *default_file = g_build_filename(PROFILES, DEFAULT_PROFILE, NULL);
	bool loaded                   = false;
	bool load_default             = false;
	if (g_file_test(file, G_FILE_TEST_EXISTS))
		loaded = true;
	if (!loaded && g_file_test(default_file, G_FILE_TEST_EXISTS))
	{
		loaded       = true;
		load_default = true;
	}
	g_autofree char *user_file = _user_config_file_name_new(app->profile);
	if (!g_file_test(user_file, G_FILE_TEST_EXISTS) && loaded)
	{
		g_autoptr(GFile) src  = g_file_new_for_path(load_default ? default_file : file);
		g_autoptr(GFile) dest = g_file_new_for_path(user_file);
		g_autoptr(GError) err = NULL;
		g_file_copy(src, dest, G_FILE_COPY_BACKUP, NULL, NULL, NULL, &err);
		if (err)
		{
			g_warning("Cannot init global config: %s\n", err->message);
			return false;
		}
	}
#ifdef PLATFORM_LAYER_SHELL
	if (GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default()))
		app->platform = VALA_PANEL_PLATFORM(
		    vala_panel_platform_layer_new(GTK_APPLICATION(app), app->profile));
#endif
#ifdef PLATFORM_X11
	if (GDK_IS_X11_DISPLAY(gdk_display_get_default()))
		app->platform = VALA_PANEL_PLATFORM(
		    vala_panel_platform_x11_new(GTK_APPLICATION(app), app->profile));
#endif
	ValaPanelCoreSettings *s         = vala_panel_platform_get_settings(app->platform);
	GSettingsBackend *config_backend = s->backend;
	app->config = g_settings_new_with_backend_and_path(VALA_PANEL_BASE_SCHEMA,
	                                                   config_backend,
	                                                   VALA_PANEL_OBJECT_PATH);
	vala_panel_bind_gsettings(app, app->config, VALA_PANEL_KEY_LOCK);
	vala_panel_bind_gsettings(app, app->config, VALA_PANEL_KEY_RUN);
	vala_panel_bind_gsettings(app, app->config, VALA_PANEL_KEY_LOGOUT);
	vala_panel_bind_gsettings(app, app->config, VALA_PANEL_KEY_SHUTDOWN);
	vala_panel_bind_gsettings(app, app->config, VALA_PANEL_KEY_TERMINAL);
	vala_panel_bind_gsettings(app, app->config, VALA_PANEL_KEY_CSS);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(app), app->config, VALA_PANEL_KEY_DARK);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(app), app->config, VALA_PANEL_KEY_CUSTOM);
	return true;
}

static void _ensure_user_config_dirs(void)
{
	g_autofree char *dir = _user_config_file_name_new("");
	/* make sure the private profile and panels dir exists */
	g_mkdir_with_parents(dir, 0700);
}

void vala_panel_application_activate(GApplication *app)
{
	static bool is_started     = false;
	ValaPanelApplication *self = VALA_PANEL_APPLICATION(app);
	if (!is_started)
	{
		g_application_mark_busy(app);
		/*load config*/
		_ensure_user_config_dirs();
		load_settings(self);
		gdk_window_set_events(gdk_get_default_root_window(),
		                      (GdkEventMask)(GDK_STRUCTURE_MASK | GDK_SUBSTRUCTURE_MASK |
		                                     GDK_PROPERTY_CHANGE_MASK));
		if (G_UNLIKELY(!vala_panel_platform_start_panels_from_profile(VALA_PANEL_PLATFORM(
		                                                                  self->platform),
		                                                              GTK_APPLICATION(app),
		                                                              self->profile)))
		{
			g_warning("Config files / toplevels are not found.\n");
			g_application_unmark_busy(app);
			g_application_quit(app);
		}
		else
		{
			is_started = true;
			apply_styling(VALA_PANEL_APPLICATION(app));
			g_application_unmark_busy(app);
		}
	}
}

static void vala_panel_app_finalize(GObject *object)
{
	ValaPanelApplication *app = VALA_PANEL_APPLICATION(object);
	g_clear_object(&app->config);
	g_clear_object(&app->platform);
	g_clear_pointer(&app->css, g_free);
	g_clear_pointer(&app->terminal_command, g_free);
	g_clear_pointer(&app->lock_command, g_free);
	g_clear_pointer(&app->run_command, g_free);
	g_clear_pointer(&app->logout_command, g_free);
	g_clear_pointer(&app->shutdown_command, g_free);
	g_clear_object(&app->provider);
	g_clear_pointer(&app->profile, g_free);
	G_OBJECT_CLASS(vala_panel_application_parent_class)->finalize(object);
}

static void vala_panel_app_set_property(GObject *object, uint id, const GValue *value,
                                        GParamSpec *pspec)
{
	ValaPanelApplication *app;
	g_return_if_fail(VALA_PANEL_IS_APPLICATION(object));

	app = VALA_PANEL_APPLICATION(object);

	switch (id)
	{
	case APP_IS_DARK:
		app->dark = g_value_get_boolean(value);
		apply_styling(app);
		g_object_notify_by_pspec(object, pspec);
		break;
	case APP_IS_CUSTOM:
		app->custom = g_value_get_boolean(value);
		apply_styling(app);
		g_object_notify_by_pspec(object, pspec);
		break;
	case APP_TERMINAL_COMMAND:
		g_clear_pointer(&app->terminal_command, g_free);
		app->terminal_command = g_strdup(g_value_get_string(value));
		g_object_notify_by_pspec(object, pspec);
		break;
	case APP_RUN_COMMAND:
		g_clear_pointer(&app->run_command, g_free);
		app->run_command = g_strdup(g_value_get_string(value));
		g_object_notify_by_pspec(object, pspec);
		break;
	case APP_LOCK_COMMAND:
		g_clear_pointer(&app->lock_command, g_free);
		app->lock_command = g_strdup(g_value_get_string(value));
		g_object_notify_by_pspec(object, pspec);
		break;
	case APP_LOGOUT_COMMAND:
		g_clear_pointer(&app->logout_command, g_free);
		app->logout_command = g_strdup(g_value_get_string(value));
		g_object_notify_by_pspec(object, pspec);
		break;
	case APP_SHUTDOWN_COMMAND:
		g_clear_pointer(&app->shutdown_command, g_free);
		app->shutdown_command = g_strdup(g_value_get_string(value));
		g_object_notify_by_pspec(object, pspec);
		break;
	case APP_PROFILE:
		g_clear_pointer(&app->profile, g_free);
		app->profile = g_strdup(g_value_get_string(value));
		g_object_notify_by_pspec(object, pspec);
		break;
	case APP_CSS:
		g_clear_pointer(&app->css, g_free);
		app->css = g_strdup(g_value_get_string(value));
		apply_styling(app);
		g_object_notify_by_pspec(object, pspec);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, pspec);
		break;
	}
}

static void vala_panel_app_get_property(GObject *object, uint id, GValue *value, GParamSpec *pspec)
{
	ValaPanelApplication *app;
	g_return_if_fail(VALA_PANEL_IS_APPLICATION(object));

	app = VALA_PANEL_APPLICATION(object);

	switch (id)
	{
	case APP_IS_DARK:
		g_value_set_boolean(value, app->dark);
		break;
	case APP_IS_CUSTOM:
		g_value_set_boolean(value, app->custom);
		break;
	case APP_RUN_COMMAND:
		g_value_set_string(value, app->run_command);
		break;
	case APP_TERMINAL_COMMAND:
		g_value_set_string(value, app->terminal_command);
		break;
	case APP_LOCK_COMMAND:
		g_value_set_string(value, app->lock_command);
		break;
	case APP_LOGOUT_COMMAND:
		g_value_set_string(value, app->logout_command);
		break;
	case APP_SHUTDOWN_COMMAND:
		g_value_set_string(value, app->shutdown_command);
		break;
	case APP_PROFILE:
		g_value_set_string(value, app->profile);
		break;
	case APP_CSS:
		g_value_set_string(value, app->css);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, pspec);
		break;
	}
}

static inline void file_chooser_helper(GtkFileChooser *self, ValaPanelApplication *app)
{
	g_autofree char *file = gtk_file_chooser_get_filename(self);
	g_object_set(app, VALA_PANEL_KEY_CSS, file, NULL);
}

/* TODO: Move activate_menu to hidden toplevel interface, it is not X11-specific */

static void activate_menu(G_GNUC_UNUSED GSimpleAction *simple, G_GNUC_UNUSED GVariant *param,
                          gpointer data)
{
	GtkApplication *app = GTK_APPLICATION(data);
	GList *windows      = gtk_application_get_windows(app);
	for (GList *l = windows; l != NULL; l = l->next)
	{
		if (VALA_PANEL_IS_TOPLEVEL(l->data))
		{
			ValaPanelLayout *layout =
			    vala_panel_toplevel_get_layout(VALA_PANEL_TOPLEVEL(l->data));
			GList *applets = vala_panel_layout_get_applets_list(layout);
			for (GList *il = applets; il != NULL; il = il->next)
			{
				ValaPanelApplet *applet = VALA_PANEL_APPLET(il->data);
				bool success = vala_panel_applet_remote_command(applet, "menu");
				if (success)
					break;
			}
			g_list_free(applets);
		}
	}
}

static void activate_panel_preferences(G_GNUC_UNUSED GSimpleAction *simple,
                                       G_GNUC_UNUSED GVariant *param, gpointer data)
{
	GtkApplication *app   = GTK_APPLICATION(data);
	GList *windows        = gtk_application_get_windows(app);
	g_autofree char *name = NULL;
	for (GList *l = windows; l != NULL; l = l->next)
	{
		if (VALA_PANEL_IS_TOPLEVEL(l->data))
		{
			g_object_get(l->data, VALA_PANEL_KEY_UUID, &name, NULL);
			if (!g_strcmp0(name, g_variant_get_string(param, NULL)))
			{
				vala_panel_toplevel_configure(l->data, "position");
				break;
			}
			g_warning(_("No panel with this name found.\n"));
		}
	}
}

static void activate_preferences(G_GNUC_UNUSED GSimpleAction *simple, G_GNUC_UNUSED GVariant *param,
                                 gpointer data)
{
	static GtkDialog *pref_dialog = NULL;
	ValaPanelApplication *self    = VALA_PANEL_APPLICATION(data);
	if (pref_dialog != NULL && GTK_IS_DIALOG(pref_dialog))
	{
		gtk_window_present(GTK_WINDOW(pref_dialog));
		return;
	}
	g_autoptr(GtkBuilder) builder =
	    gtk_builder_new_from_resource("/org/vala-panel/app/pref.ui");
	pref_dialog = GTK_DIALOG(gtk_builder_get_object(builder, "app-pref"));
	gtk_application_add_window(GTK_APPLICATION(self), GTK_WINDOW(pref_dialog));
	GObject *w = gtk_builder_get_object(builder, "logout");
	g_settings_bind(self->config, VALA_PANEL_KEY_LOGOUT, w, "text", G_SETTINGS_BIND_DEFAULT);
	w = gtk_builder_get_object(builder, "lock");
	g_settings_bind(self->config, VALA_PANEL_KEY_LOCK, w, "text", G_SETTINGS_BIND_DEFAULT);
	w = gtk_builder_get_object(builder, "shutdown");
	g_settings_bind(self->config, VALA_PANEL_KEY_SHUTDOWN, w, "text", G_SETTINGS_BIND_DEFAULT);
	w = gtk_builder_get_object(builder, "css-chooser");
	g_settings_bind(self->config, VALA_PANEL_KEY_CUSTOM, w, "sensitive", G_SETTINGS_BIND_DEFAULT);
	GtkFileChooserButton *f = GTK_FILE_CHOOSER_BUTTON(w);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(f), self->css);
	g_signal_connect(f, "file-set", G_CALLBACK(file_chooser_helper), self);
	gtk_window_present(GTK_WINDOW(pref_dialog));
	g_signal_connect(pref_dialog, "hide", G_CALLBACK(destroy0), &pref_dialog);
	g_signal_connect_after(pref_dialog, "response", G_CALLBACK(destroy0), &pref_dialog);
}

static void activate_about(G_GNUC_UNUSED GSimpleAction *simple, G_GNUC_UNUSED GVariant *param,
                           G_GNUC_UNUSED gpointer data)
{
	g_autoptr(GtkBuilder) builder =
	    gtk_builder_new_from_resource("/org/vala-panel/app/about.ui");
	GtkAboutDialog *d = GTK_ABOUT_DIALOG(gtk_builder_get_object(builder, "valapanel-about"));
	gtk_about_dialog_set_version(d, VERSION);
	gtk_window_set_position(GTK_WINDOW(d), GTK_WIN_POS_CENTER);
	gtk_window_present(GTK_WINDOW(d));
	g_signal_connect(d, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect(d, "response", G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect(d, "hide", G_CALLBACK(gtk_widget_destroy), NULL);
}

static void activate_run(G_GNUC_UNUSED GSimpleAction *simple, G_GNUC_UNUSED GVariant *param,
                         gpointer data)
{
	ValaPanelApplication *app = VALA_PANEL_APPLICATION(data);
	g_autoptr(GVariant) par   = g_variant_new_string(app->run_command);
	activate_menu_launch_command(NULL, par, app);
}

static void activate_lock(G_GNUC_UNUSED GSimpleAction *simple, G_GNUC_UNUSED GVariant *param,
                          gpointer data)
{
	ValaPanelApplication *app = VALA_PANEL_APPLICATION(data);
	g_autoptr(GVariant) par   = g_variant_new_string(app->lock_command);
	activate_menu_launch_command(NULL, par, app);
}

static void activate_logout(G_GNUC_UNUSED GSimpleAction *simple, G_GNUC_UNUSED GVariant *param,
                            gpointer data)
{
	ValaPanelApplication *app = VALA_PANEL_APPLICATION(data);
	g_autoptr(GVariant) par   = g_variant_new_string(app->logout_command);
	activate_menu_launch_command(NULL, par, app);
}

static void activate_shutdown(G_GNUC_UNUSED GSimpleAction *simple, G_GNUC_UNUSED GVariant *param,
                              gpointer data)
{
	ValaPanelApplication *app = VALA_PANEL_APPLICATION(data);
	g_autoptr(GVariant) par   = g_variant_new_string(app->shutdown_command);
	activate_menu_launch_command(NULL, par, app);
}
static void activate_exit(G_GNUC_UNUSED GSimpleAction *simple, G_GNUC_UNUSED GVariant *param,
                          gpointer data)
{
	ValaPanelApplication *app = VALA_PANEL_APPLICATION(data);
	app->restart              = false;
	g_application_quit(G_APPLICATION(app));
}

static void activate_restart(G_GNUC_UNUSED GSimpleAction *simple, G_GNUC_UNUSED GVariant *param,
                             gpointer data)
{
	ValaPanelApplication *app = VALA_PANEL_APPLICATION(data);
	app->restart              = true;
	g_application_quit(G_APPLICATION(app));
}

static void vala_panel_application_class_init(ValaPanelApplicationClass *klass)
{
	G_APPLICATION_CLASS(klass)->startup              = vala_panel_application_startup;
	G_APPLICATION_CLASS(klass)->shutdown             = vala_panel_application_shutdown;
	G_APPLICATION_CLASS(klass)->activate             = vala_panel_application_activate;
	G_APPLICATION_CLASS(klass)->handle_local_options = vala_panel_app_handle_local_options;
	G_APPLICATION_CLASS(klass)->command_line         = vala_panel_app_command_line;
	G_OBJECT_CLASS(klass)->get_property              = vala_panel_app_get_property;
	G_OBJECT_CLASS(klass)->set_property              = vala_panel_app_set_property;
	G_OBJECT_CLASS(klass)->finalize                  = vala_panel_app_finalize;
	app_specs[APP_PROFILE]                           = g_param_spec_string("profile",
                                                     "profile",
                                                     "profile",
                                                     "default",
                                                     G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	app_specs[APP_RUN_COMMAND] =
	    g_param_spec_string(VALA_PANEL_KEY_RUN,
	                        VALA_PANEL_KEY_RUN,
	                        VALA_PANEL_KEY_RUN,
	                        NULL,
	                        G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	app_specs[APP_TERMINAL_COMMAND] =
	    g_param_spec_string(VALA_PANEL_KEY_TERMINAL,
	                        VALA_PANEL_KEY_TERMINAL,
	                        VALA_PANEL_KEY_TERMINAL,
	                        NULL,
	                        G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	app_specs[APP_LOCK_COMMAND] =
	    g_param_spec_string(VALA_PANEL_KEY_LOCK,
	                        VALA_PANEL_KEY_LOCK,
	                        VALA_PANEL_KEY_LOCK,
	                        NULL,
	                        G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	app_specs[APP_LOGOUT_COMMAND] =
	    g_param_spec_string(VALA_PANEL_KEY_LOGOUT,
	                        VALA_PANEL_KEY_LOGOUT,
	                        VALA_PANEL_KEY_LOGOUT,
	                        NULL,
	                        G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	app_specs[APP_SHUTDOWN_COMMAND] =
	    g_param_spec_string(VALA_PANEL_KEY_SHUTDOWN,
	                        VALA_PANEL_KEY_SHUTDOWN,
	                        VALA_PANEL_KEY_SHUTDOWN,
	                        NULL,
	                        G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	app_specs[APP_IS_DARK]   = g_param_spec_boolean(VALA_PANEL_KEY_DARK,
                                                      VALA_PANEL_KEY_DARK,
                                                      VALA_PANEL_KEY_DARK,
                                                      false,
                                                      G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	app_specs[APP_IS_CUSTOM] = g_param_spec_boolean(VALA_PANEL_KEY_CUSTOM,
	                                                VALA_PANEL_KEY_CUSTOM,
	                                                VALA_PANEL_KEY_CUSTOM,
	                                                false,
	                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	app_specs[APP_CSS]       = g_param_spec_string(VALA_PANEL_KEY_CSS,
                                                 VALA_PANEL_KEY_CSS,
                                                 VALA_PANEL_KEY_CSS,
                                                 NULL,
                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
	g_object_class_install_properties(G_OBJECT_CLASS(klass), APP_ALL, app_specs);
}

int main(int argc, char *argv[])
{
	ValaPanelApplication *app = vala_panel_application_new();
	int ret                   = g_application_run(G_APPLICATION(app), argc, argv);
	g_clear_object(&app);
	return ret;
}
