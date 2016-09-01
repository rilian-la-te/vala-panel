#include "config.h"

#include "application-new.h"
#include "launcher.h"
#include "lib/definitions.h"
#include "runner-new.h"

#include <glib/gi18n-lib.h>
#include <locale.h>
#include <stdbool.h>

struct _ValaPanelApplication
{
	bool started;
	bool restart;
	GtkDialog *pref_dialog;
	GSettings *config;
	ValaPanelRunner *runner;
	bool dark;
	bool custom;
	char *css;
	GtkCssProvider *provider;
	char *profile;
	char *terminal_command;
	char *logout_command;
	char *shutdown_command;
};

G_DEFINE_TYPE(ValaPanelApplication, vala_panel_application, GTK_TYPE_APPLICATION)

static const GActionEntry vala_panel_application_app_entries[9] = {
	//  { "preferences",
	//    activate_preferences, NULL,
	//    NULL, NULL },
	//  { "panel-preferences",
	//    activate_panel_preferences,
	//    "s", NULL, NULL },
	//  { "about", activate_about,
	//    NULL, NULL, NULL },
	//  { "menu",
	//    activate_menu,
	//    NULL, NULL, NULL },
	//  { "run", activate_run, NULL,
	//    NULL, NULL },
	//  { "logout", activate_logout,
	//    NULL, NULL, NULL },
	//  { "shutdown",
	//    activate_shutdown, NULL,
	//    NULL, NULL },
	//  { "restart",
	//    activate_restart, NULL,
	//    NULL, NULL },
	//  { "quit", activate_exit,
	//    NULL, NULL, NULL }
};
static const GActionEntry vala_panel_application_menu_entries[3] =
    { { "launch-id", activate_menu_launch_id, "s", NULL, NULL, { 0 } },
      { "launch-uri", activate_menu_launch_uri, "s", NULL, NULL, { 0 } },
      { "launch-command", activate_menu_launch_command, "s", NULL, NULL, { 0 } } };

enum
{
	VALA_PANEL_APP_DUMMY_PROPERTY,
	VALA_PANEL_APP_PROFILE,
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
	GList *lst	     = gtk_application_get_windows(GTK_APPLICATION(base));
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

static void vala_panel_application_class_init(ValaPanelApplicationClass *klass)
{
	vala_panel_application_parent_class    = g_type_class_peek_parent(klass);
	((GApplicationClass *)klass)->startup  = vala_panel_application_startup;
	((GApplicationClass *)klass)->shutdown = vala_panel_application_shutdown;
	//    ((GApplicationClass *) klass)->activate = vala_panel_app_real_activate;
	//    ((GApplicationClass *) klass)->handle_local_options =
	//    vala_panel_app_real_handle_local_options;
	//    ((GApplicationClass *) klass)->command_line = vala_panel_app_real_command_line;
	//    G_OBJECT_CLASS (klass)->get_property = _vala_vala_panel_app_get_property;
	//    G_OBJECT_CLASS (klass)->set_property = _vala_vala_panel_app_set_property;
	//    G_OBJECT_CLASS (klass)->constructor = vala_panel_app_constructor;
	//    G_OBJECT_CLASS (klass)->finalize = vala_panel_app_finalize;
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
