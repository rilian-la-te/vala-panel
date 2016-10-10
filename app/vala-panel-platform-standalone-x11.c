#include "vala-panel-platform-standalone-x11.h"
#include "gio/gsettingsbackend.h"
#include "lib/applets-new/applet-api.h"
#include "lib/c-lib/toplevel.h"
#include "lib/definitions.h"

struct _ValaPanelPlatformX11
{
	GObject __parent__;
	char *profile;
};

#define g_key_file_load_from_config(f, p)                                                          \
	g_key_file_load_from_file(f,                                                               \
	                          _user_config_file_name(GETTEXT_PACKAGE, p, NULL),                \
	                          G_KEY_FILE_KEEP_COMMENTS,                                        \
	                          NULL)

static void vala_panel_platform_x11_default_init(ValaPanelPlatformInterface *iface);
G_DEFINE_TYPE_WITH_CODE(ValaPanelPlatformX11, vala_panel_platform_x11, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(vala_panel_platform_get_type(),
                                              vala_panel_platform_x11_default_init))

ValaPanelPlatformX11 *vala_panel_platform_x11_new(const char *profile)
{
	ValaPanelPlatformX11 *pl =
	    VALA_PANEL_PLATFORM_X11(g_object_new(vala_panel_platform_x11_get_type(), NULL));
	pl->profile = g_strdup(profile);
	return pl;
}

static GSettings *vala_panel_platform_x11_get_settings_for_scheme(ValaPanelPlatform *obj,
                                                                  const char *scheme,
                                                                  const char *path)
{
	ValaPanelPlatformX11 *self = VALA_PANEL_PLATFORM_X11(obj);
	g_autoptr(GSettingsBackend) backend =
	    g_keyfile_settings_backend_new(_user_config_file_name(GETTEXT_PACKAGE,
	                                                          self->profile,
	                                                          NULL),
	                                   DEFAULT_PLUGIN_PATH,
	                                   "main-settings");
	return g_settings_new_with_backend_and_path(scheme, backend, path);
}

static void vala_panel_platform_x11_remove_settings_path(ValaPanelPlatform *obj, const char *path,
                                                         const char *child_name)
{
	ValaPanelPlatformX11 *self = VALA_PANEL_PLATFORM_X11(obj);
	g_autoptr(GKeyFile) f      = g_key_file_new();
	g_key_file_load_from_config(f, self->profile);
	if (g_key_file_has_group(f, child_name))
	{
		g_key_file_remove_group(f, child_name, NULL);
		g_key_file_save_to_file(f,
		                        _user_config_file_name(GETTEXT_PACKAGE,
		                                               self->profile,
		                                               NULL),
		                        NULL);
	}
}

static bool vala_panel_platform_x11_start_panels_from_profile(ValaPanelPlatform *obj,
                                                              GtkApplication *app,
                                                              const char *profile)
{
	ValaPanelPlatformX11 *self = VALA_PANEL_PLATFORM_X11(obj);
	g_autoptr(GKeyFile) f      = g_key_file_new();
	g_key_file_load_from_config(f, self->profile);
	g_autoptr(GSettingsBackend) backend =
	    g_keyfile_settings_backend_new(_user_config_file_name(GETTEXT_PACKAGE,
	                                                          self->profile,
	                                                          NULL),
	                                   DEFAULT_PLUGIN_PATH,
	                                   "main-settings");
	g_autoptr(GSettings) s =
	    g_settings_new_with_backend_and_path(VALA_PANEL_APPLICATION_SETTINGS,
	                                         backend,
	                                         DEFAULT_PLUGIN_PATH);
	g_autoptr(GSettings) settings = g_settings_get_child(s, profile);
	g_auto(GStrv) panels = g_settings_get_strv(settings, VALA_PANEL_APPLICATION_PANELS);
	for (int i = 0; panels[i] != NULL; i++)
	{
		ValaPanelToplevelUnit *unit = vala_panel_toplevel_unit_new_from_uid(app, panels[i]);
		gtk_application_add_window(app, GTK_WINDOW(unit));
	}
	return true;
}

static void vala_panel_platform_x11_default_init(ValaPanelPlatformInterface *iface)
{
	iface->get_settings_for_scheme   = vala_panel_platform_x11_get_settings_for_scheme;
	iface->remove_settings_path      = vala_panel_platform_x11_remove_settings_path;
	iface->start_panels_from_profile = vala_panel_platform_x11_start_panels_from_profile;
}

static void vala_panel_platform_x11_finalize(GObject *obj)
{
	ValaPanelPlatformX11 *self = VALA_PANEL_PLATFORM_X11(obj);
	g_free(self->profile);
	(*G_OBJECT_CLASS(vala_panel_platform_x11_parent_class)->finalize)(obj);
}

static void vala_panel_platform_x11_init(ValaPanelPlatformX11 *self)
{
}

static void vala_panel_platform_x11_class_init(ValaPanelPlatformX11Class *klass)
{
	G_OBJECT_CLASS(klass)->finalize = vala_panel_platform_x11_finalize;
}
