#include "applet-info.h"
#include "config.h"

#include <glib/gi18n.h>
#include <stdbool.h>

#define VALA_PANEL_APPLET_GROUP "Applet"

#define VALA_PANEL_APPLET_INFO_NAME "Name"
#define VALA_PANEL_APPLET_INFO_DESCRIPTION "Description"
#define VALA_PANEL_APPLET_INFO_ICON "Icon"
#define VALA_PANEL_APPLET_INFO_AUTHORS "Authors"
#define VALA_PANEL_APPLET_INFO_WEBSITE "Website"
#define VALA_PANEL_APPLET_INFO_HELP_URI "HelpURI"
#define VALA_PANEL_APPLET_INFO_LICENSE "License"
#define VALA_PANEL_APPLET_INFO_VERSION "Version"
#define VALA_PANEL_APPLET_INFO_ONE_PER_SYSTEM "OnePerSystem"
#define VALA_PANEL_APPLET_INFO_EXPANDABLE "Expandable"

struct _ValaPanelAppletInfo
{
	char *name;
	char *description;
	char *icon_name;
	char **authors;
	char *website;
	char *help_uri;
	char *license;
	char *version;
	bool one_per_system;
	bool expandable;
};

ValaPanelAppletInfo *vala_panel_applet_info_load(const char *extension_name)
{
	g_autoptr(GKeyFile) file      = g_key_file_new();
	g_autofree char *desktop_name = g_strdup_printf("%s.desktop", extension_name);
	g_autoptr(GError) err         = NULL;
	bool loaded                   = g_key_file_load_from_data_dirs(file,
	                                             desktop_name,
	                                             NULL,
	                                             G_KEY_FILE_KEEP_TRANSLATIONS,
	                                             &err);
	if (err)
		g_error("%s\n", err->message);
	if (!loaded || err)
		return NULL;
	struct _ValaPanelAppletInfo *ret = g_slice_alloc0(sizeof(struct _ValaPanelAppletInfo));
	char *tmp =
	    g_key_file_get_string(file, VALA_PANEL_APPLET_GROUP, VALA_PANEL_APPLET_INFO_NAME, NULL);
	ret->name = tmp != NULL ? tmp : g_strdup(_("Applet"));
	tmp       = g_key_file_get_string(file,
	                            VALA_PANEL_APPLET_GROUP,
	                            VALA_PANEL_APPLET_INFO_DESCRIPTION,
	                            NULL);
	ret->description = tmp != NULL ? tmp : g_strdup(_("Vala Panel Applet"));
	tmp =
	    g_key_file_get_string(file, VALA_PANEL_APPLET_GROUP, VALA_PANEL_APPLET_INFO_ICON, NULL);
	ret->icon_name = tmp != NULL ? tmp : g_strdup("software-update-available");
	GStrv tmp_list = g_key_file_get_string_list(file,
	                                            VALA_PANEL_APPLET_GROUP,
	                                            VALA_PANEL_APPLET_INFO_AUTHORS,
	                                            NULL,
	                                            NULL);
	ret->authors = tmp_list;
	tmp          = g_key_file_get_string(file,
	                            VALA_PANEL_APPLET_GROUP,
	                            VALA_PANEL_APPLET_INFO_WEBSITE,
	                            NULL);
	ret->website =
	    tmp != NULL ? tmp : g_strdup("https://gitlab.com/vala-panel-project/vala-panel");
	tmp = g_key_file_get_string(file,
	                            VALA_PANEL_APPLET_GROUP,
	                            VALA_PANEL_APPLET_INFO_HELP_URI,
	                            NULL);
	ret->help_uri =
	    tmp != NULL ? tmp
	                : g_strdup("https://gitlab.com/vala-panel-project/vala-panel/wikis/home");
	tmp = g_key_file_get_string(file,
	                            VALA_PANEL_APPLET_GROUP,
	                            VALA_PANEL_APPLET_INFO_WEBSITE,
	                            NULL);
	ret->license = tmp != NULL ? tmp : g_strdup("GNU LGPLv3");
	tmp          = g_key_file_get_string(file,
	                            VALA_PANEL_APPLET_GROUP,
	                            VALA_PANEL_APPLET_INFO_VERSION,
	                            NULL);
	ret->version        = tmp != NULL ? tmp : g_strdup(VERSION);
	ret->one_per_system = g_key_file_get_boolean(file,
	                                             VALA_PANEL_APPLET_GROUP,
	                                             VALA_PANEL_APPLET_INFO_ONE_PER_SYSTEM,
	                                             NULL);
	ret->expandable = g_key_file_get_boolean(file,
	                                         VALA_PANEL_APPLET_GROUP,
	                                         VALA_PANEL_APPLET_INFO_EXPANDABLE,
	                                         NULL);
	return ret;
}

ValaPanelAppletInfo *vala_panel_applet_info_duplicate(void *info)
{
	struct _ValaPanelAppletInfo *ainfo = (struct _ValaPanelAppletInfo *)info;
	struct _ValaPanelAppletInfo *ret   = g_slice_alloc0(sizeof(struct _ValaPanelAppletInfo));
	ret->name                          = g_strdup(ainfo->name);
	ret->description                   = g_strdup(ainfo->description);
	ret->icon_name                     = g_strdup(ainfo->icon_name);
	if (ainfo->authors)
	{
		u_int32_t len = g_strv_length(ainfo->authors);
		ret->authors  = g_new0(char *, len + 1);
		for (uint i             = 0; i < len; i++)
			ret->authors[i] = g_strdup(ainfo->authors[i]);
	}
	else
		ret->authors = NULL;
	ret->website         = g_strdup(ainfo->website);
	ret->help_uri        = g_strdup(ainfo->help_uri);
	ret->license         = g_strdup(ainfo->license);
	ret->version         = g_strdup(ainfo->version);
	ret->one_per_system  = ainfo->one_per_system;
	ret->expandable      = ainfo->expandable;
	return ret;
}

void vala_panel_applet_info_free(void *info)
{
	struct _ValaPanelAppletInfo *ainfo = (struct _ValaPanelAppletInfo *)info;
	g_free(ainfo->name);
	g_free(ainfo->description);
	g_free(ainfo->icon_name);
	if (ainfo->authors)
		g_strfreev(ainfo->authors);
	g_free(ainfo->website);
	g_free(ainfo->help_uri);
	g_free(ainfo->license);
	g_free(ainfo->version);
	g_slice_free(struct _ValaPanelAppletInfo, info);
}

G_DEFINE_BOXED_TYPE(ValaPanelAppletInfo, vala_panel_applet_info,
                    (GBoxedCopyFunc)vala_panel_applet_info_duplicate, vala_panel_applet_info_free)
