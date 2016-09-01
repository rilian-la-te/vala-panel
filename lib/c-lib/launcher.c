#include <string.h>
#include <unistd.h>

#include "launcher.h"

typedef struct
{
	pid_t pid;
} SpawnData;

void child_spawn_func(void *data)
{
	setpgid(0, getpgid(getppid()));
}

bool vala_panel_launch(GDesktopAppInfo *app_info, GList *uris)
{
	g_autoptr(GError) err = NULL;
	g_autoptr(GAppLaunchContext) cxt =
	    G_APP_LAUNCH_CONTEXT(gdk_display_get_app_launch_context(gdk_display_get_default()));
	bool ret = g_desktop_app_info_launch_uris_as_manager(G_DESKTOP_APP_INFO(app_info),
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

static GAppInfo *vala_panel_get_default_for_uri(const char *uri)
{
	/* g_file_query_default_handler() calls
	* g_app_info_get_default_for_uri_scheme() too, but we have to do it
	* here anyway in case GFile can't parse @uri correctly.
	*/
	GAppInfo *app_info          = NULL;
	g_autofree char *uri_scheme = g_uri_parse_scheme(uri);
	if (uri_scheme != NULL && strlen(uri_scheme) <= 0)
		app_info = g_app_info_get_default_for_uri_scheme(uri_scheme);
	if (app_info == NULL)
	{
		g_autoptr(GFile) file = g_file_new_for_uri(uri);
		app_info              = g_file_query_default_handler(file, NULL, NULL);
	}
	return app_info;
}

void activate_menu_launch_id(GSimpleAction *action, GVariant *param, gpointer user_data)
{
	const gchar *id                 = g_variant_get_string(param, NULL);
	g_autoptr(GDesktopAppInfo) info = g_desktop_app_info_new(id);
	vala_panel_launch(info, NULL);
}

void activate_menu_launch_uri(GSimpleAction *action, GVariant *param, gpointer user_data)
{
	const char *uri                 = g_variant_get_string(param, NULL);
	g_autoptr(GList) uris           = g_list_append(NULL, (gpointer)uri);
	g_autoptr(GDesktopAppInfo) info = G_DESKTOP_APP_INFO(vala_panel_get_default_for_uri(uri));
	vala_panel_launch(info, uris);
}

void activate_menu_launch_command(GSimpleAction *action, GVariant *param, gpointer user_data)
{
	g_autoptr(GError) err           = NULL;
	const char *commandline         = g_variant_get_string(param, NULL);
	g_autoptr(GDesktopAppInfo) info = G_DESKTOP_APP_INFO(
	    g_app_info_create_from_commandline(commandline,
	                                       NULL,
	                                       G_APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION,
	                                       &err));
	if (err)
		g_warning("%s\n", err->message);
	vala_panel_launch(info, NULL);
}
