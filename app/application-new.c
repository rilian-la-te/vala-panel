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

#include "application-new.h"
#include "launcher.h"
#include "lib/css.h"
#include "lib/definitions.h"

#include <glib/gi18n.h>
#include <locale.h>
#include <stdbool.h>

struct _ValaPanelApplication
{
	bool started;
	bool restart;
	GtkDialog *pref_dialog;
	GSettings *config;
	bool dark;
	bool custom;
	char *css;
	GtkCssProvider *provider;
	char *profile;
	char *run_command;
	char *terminal_command;
	char *logout_command;
	char *shutdown_command;
};

G_DEFINE_TYPE(ValaPanelApplication, vala_panel_application, GTK_TYPE_APPLICATION)

static void activate_command(GSimpleAction *simple, GVariant *param, gpointer data);
static void activate_exit(GSimpleAction *simple, GVariant *param, gpointer data);

static const GOptionEntry entries[] =
    { { "version", 'v', 0, G_OPTION_ARG_NONE, NULL, N_("Print version and exit"), NULL },
      { "profile", 'p', 0, G_OPTION_ARG_STRING, NULL, N_("Use specified profile"), N_("profile") },
      { "command",
	'c',
	0,
	G_OPTION_ARG_STRING,
	NULL,
	N_("Run command on already opened panel"),
	N_("cmd") },
      { NULL } };

static const GActionEntry vala_panel_application_app_entries[6] = {
	//      { "preferences",
	//        activate_preferences, NULL,
	//        NULL, NULL , { 0 } },
	//      { "panel-preferences",
	//        activate_panel_preferences,
	//        "s", NULL, NULL, { 0 } },
	//      { "about", activate_about,
	//        NULL, NULL, NULL, { 0 } },
	//      { "menu",
	//        activate_menu,
	//        NULL, NULL, NULL, { 0 } },
	{ "session-command", activate_command, "s", NULL, NULL, { 0 } },
	{ "quit", activate_exit, "b", NULL, NULL, { 0 } },
};
static const GActionEntry vala_panel_application_menu_entries[3] =
    { { "launch-id", activate_menu_launch_id, "s", NULL, NULL, { 0 } },
      { "launch-uri", activate_menu_launch_uri, "s", NULL, NULL, { 0 } },
      { "launch-command", activate_menu_launch_command, "s", NULL, NULL, { 0 } } };

enum
{
	VALA_PANEL_APP_DUMMY_PROPERTY,
	VALA_PANEL_APP_PROFILE,
	VALA_PANEL_APP_RUN_COMMAND,
	VALA_PANEL_APP_TERMINAL_COMMAND,
	VALA_PANEL_APP_LOGOUT_COMMAND,
	VALA_PANEL_APP_SHUTDOWN_COMMAND,
	VALA_PANEL_APP_IS_DARK,
	VALA_PANEL_APP_IS_CUSTOM,
	VALA_PANEL_APP_CSS
};
#define system_config_file_name(profile, dir, filename)                                            \
	g_build_filename(dir, GETTEXT_PACKAGE, profile, filename, NULL)

ValaPanelApplication *vala_panel_application_new()
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
	self->started = false;
	self->restart = false;
	self->profile = g_strdup("default");
	g_application_add_main_option_entries(G_APPLICATION(self), entries);
}

static void vala_panel_application_startup(GApplication *base)
{
	ValaPanelApplication *self = (ValaPanelApplication *)base;
	G_APPLICATION_CLASS(vala_panel_application_parent_class)
	    ->startup((GApplication *)G_TYPE_CHECK_INSTANCE_CAST(self,
	                                                         gtk_application_get_type(),
	                                                         GtkApplication));
	g_application_mark_busy((GApplication *)self);
	setlocale(LC_CTYPE, "");
	bindtextdomain(CONFIG_GETTEXT_PACKAGE, CONFIG_LOCALE_DIR);
	bind_textdomain_codeset(CONFIG_GETTEXT_PACKAGE, "UTF-8");
	textdomain(CONFIG_GETTEXT_PACKAGE);
	gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), CONFIG_DATADIR "/images");
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
		gtk_widget_destroy(GTK_WIDGET(w));
	}
	G_APPLICATION_CLASS(vala_panel_application_parent_class)
	    ->shutdown((GApplication *)G_TYPE_CHECK_INSTANCE_CAST(base,
	                                                          gtk_application_get_type(),
	                                                          GtkApplication));
	if (VALA_PANEL_APPLICATION(base)->restart)
	{
		char cwd[1024];
		getcwd(cwd, 1024);
		const char *argv[] = { CONFIG_GETTEXT_PACKAGE,
			               "-p",
			               VALA_PANEL_APPLICATION(base)->profile };
		g_auto(GStrv) envp = g_get_environ();
		g_spawn_async(cwd,
		              (GStrv)argv,
		              envp,
		              G_SPAWN_SEARCH_PATH,
		              child_spawn_func,
		              NULL,
		              NULL,
		              NULL);
	}
}

static gint vala_panel_app_handle_local_options(GApplication *application, GVariantDict *options)
{
	if (g_variant_dict_contains(options, "version"))
	{
		g_print(_("%s - Version %s\n"), g_get_application_name(), CONFIG_VERSION);
		return 0;
	}
	return -1;
}

static int vala_panel_app_command_line(GApplication *application,
                                       GApplicationCommandLine *commandline)
{
	g_autofree gchar *profile_name = NULL;
	g_autoptr(GVariantDict) options;
	g_autofree gchar *ccommand = NULL;
	options                    = g_application_command_line_get_options_dict(commandline);
	if (g_variant_dict_lookup(options, "profile", "&s", &profile_name))
		g_object_set(G_OBJECT(application), "profile", profile_name, NULL);
	if (g_variant_dict_lookup(options, "command", "&s", &ccommand))
	{
		g_autofree gchar *name    = NULL;
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
			g_autofree gchar *list = g_strjoinv(" ", listv);
			g_application_command_line_printerr(
			    commandline,
			    _("%s: invalid command - %s. Doing nothing.\nValid commands: %s\n"),
			    g_get_application_name(),
			    ccommand,
			    list);
		}
	}
	g_application_activate(application);
	return 0;
}

static void vala_panel_app_finalize(GObject *object)
{
	ValaPanelApplication *app = VALA_PANEL_APPLICATION(object);
	if (app->css)
		g_free(app->css);
	if (app->terminal_command)
		g_free(app->terminal_command);
	if (app->run_command)
		g_free(app->run_command);
	if (app->logout_command)
		g_free(app->logout_command);
	if (app->shutdown_command)
		g_free(app->shutdown_command);
	if (app->provider)
		g_object_unref(app->provider);
	g_free(app->profile);
}

void apply_styling(ValaPanelApplication *app)
{
	if (gtk_settings_get_default() != NULL)
		g_object_set(gtk_settings_get_default(),
		             "gtk-application-prefer-dark-theme",
		             app->dark,
		             NULL);
	if (app->custom)
	{
		if (app->provider)
			gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(),
			                                             app->provider);
		app->provider = css_apply_from_file_to_app_with_provider(app->css);
	}
	else if (app->provider)
	{
		gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(),
		                                             app->provider);
		app->provider = NULL;
	}
}

static void vala_panel_app_set_property(GObject *object, guint prop_id, const GValue *value,
                                        GParamSpec *pspec)
{
	ValaPanelApplication *app;
	g_return_if_fail(VALA_PANEL_IS_APPLICATION(object));

	app = VALA_PANEL_APPLICATION(object);

	switch (prop_id)
	{
	case VALA_PANEL_APP_IS_DARK:
		app->dark = g_value_get_boolean(value);
		apply_styling(app);
		break;
	case VALA_PANEL_APP_IS_CUSTOM:
		app->custom = g_value_get_boolean(value);
		apply_styling(app);
		break;
	case VALA_PANEL_APP_TERMINAL_COMMAND:
		if (app->terminal_command)
			g_free(app->terminal_command);
		app->terminal_command = g_strdup(g_value_get_string(value));
		break;
	case VALA_PANEL_APP_RUN_COMMAND:
		if (app->run_command)
			g_free(app->run_command);
		app->run_command = g_strdup(g_value_get_string(value));
		break;
	case VALA_PANEL_APP_LOGOUT_COMMAND:
		if (app->logout_command)
			g_free(app->logout_command);
		app->logout_command = g_strdup(g_value_get_string(value));
		break;
	case VALA_PANEL_APP_SHUTDOWN_COMMAND:
		if (app->shutdown_command)
			g_free(app->shutdown_command);
		app->shutdown_command = g_strdup(g_value_get_string(value));
		break;
	case VALA_PANEL_APP_PROFILE:
		if (app->profile)
			g_free(app->profile);
		app->profile = g_strdup(g_value_get_string(value));
		break;
	case VALA_PANEL_APP_CSS:
		if (app->css)
			g_free(app->css);
		app->css = g_strdup(g_value_get_string(value));
		apply_styling(app);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void vala_panel_app_get_property(GObject *object, guint prop_id, GValue *value,
                                        GParamSpec *pspec)
{
	ValaPanelApplication *app;
	g_return_if_fail(VALA_PANEL_IS_APPLICATION(object));

	app = VALA_PANEL_APPLICATION(object);

	switch (prop_id)
	{
	case VALA_PANEL_APP_IS_DARK:
		g_value_set_boolean(value, app->dark);
		break;
	case VALA_PANEL_APP_IS_CUSTOM:
		g_value_set_boolean(value, app->custom);
		break;
	case VALA_PANEL_APP_RUN_COMMAND:
		g_value_set_string(value, app->run_command);
		break;
	case VALA_PANEL_APP_TERMINAL_COMMAND:
		g_value_set_string(value, app->terminal_command);
		break;
	case VALA_PANEL_APP_LOGOUT_COMMAND:
		g_value_set_string(value, app->logout_command);
		break;
	case VALA_PANEL_APP_SHUTDOWN_COMMAND:
		g_value_set_string(value, app->shutdown_command);
		break;
	case VALA_PANEL_APP_PROFILE:
		g_value_set_string(value, app->profile);
		break;
	case VALA_PANEL_APP_CSS:
		g_value_set_string(value, app->css);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void activate_command(GSimpleAction *simple, GVariant *param, gpointer data)
{
	g_autofree gchar *command;
	const char *cmd     = g_variant_get_string(param, NULL);
	GtkApplication *app = GTK_APPLICATION(data);
	g_object_get(app, cmd, &command, NULL);
	g_autoptr(GVariant) par = g_variant_new_string(command);
	activate_menu_launch_command(NULL, par, NULL);
}

static void activate_exit(GSimpleAction *simple, GVariant *param, gpointer data)
{
	bool restart              = g_variant_get_boolean(param);
	ValaPanelApplication *app = (ValaPanelApplication *)data;
	app->restart              = restart;
	g_application_quit(G_APPLICATION(app));
}

static void vala_panel_application_class_init(ValaPanelApplicationClass *klass)
{
	vala_panel_application_parent_class    = g_type_class_peek_parent(klass);
	((GApplicationClass *)klass)->startup  = vala_panel_application_startup;
	((GApplicationClass *)klass)->shutdown = vala_panel_application_shutdown;
	//    ((GApplicationClass *) klass)->activate = vala_panel_app_real_activate;
	((GApplicationClass *)klass)->handle_local_options = vala_panel_app_handle_local_options;
	((GApplicationClass *)klass)->command_line         = vala_panel_app_command_line;
	G_OBJECT_CLASS(klass)->get_property                = vala_panel_app_get_property;
	G_OBJECT_CLASS(klass)->set_property                = vala_panel_app_set_property;
	G_OBJECT_CLASS(klass)->finalize                    = vala_panel_app_finalize;
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APP_PROFILE,
	                                g_param_spec_string("profile",
	                                                    "profile",
	                                                    "profile",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APP_RUN_COMMAND,
	                                g_param_spec_string("run-command",
	                                                    "run-command",
	                                                    "run-command",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APP_TERMINAL_COMMAND,
	                                g_param_spec_string("terminal-command",
	                                                    "terminal-command",
	                                                    "terminal-command",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APP_LOGOUT_COMMAND,
	                                g_param_spec_string("logout-command",
	                                                    "logout-command",
	                                                    "logout-command",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APP_SHUTDOWN_COMMAND,
	                                g_param_spec_string("shutdown-command",
	                                                    "shutdown-command",
	                                                    "shutdown-command",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APP_IS_DARK,
	                                g_param_spec_boolean("is-dark",
	                                                     "is-dark",
	                                                     "is-dark",
	                                                     false,
	                                                     G_PARAM_STATIC_NAME |
	                                                         G_PARAM_STATIC_NICK |
	                                                         G_PARAM_STATIC_BLURB |
	                                                         G_PARAM_READABLE |
	                                                         G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APP_IS_CUSTOM,
	                                g_param_spec_boolean("is-custom",
	                                                     "is-custom",
	                                                     "is-custom",
	                                                     false,
	                                                     G_PARAM_STATIC_NAME |
	                                                         G_PARAM_STATIC_NICK |
	                                                         G_PARAM_STATIC_BLURB |
	                                                         G_PARAM_READABLE |
	                                                         G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APP_CSS,
	                                g_param_spec_string("css",
	                                                    "css",
	                                                    "css",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE));
}

int main(int argc, char *argv[])
{
	g_autoptr(ValaPanelApplication) app = vala_panel_application_new();
	return g_application_run(G_APPLICATION(app), argc, argv);
}
