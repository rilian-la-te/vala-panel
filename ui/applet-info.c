/*
 * vala-panel
 * Copyright (C) 2015-2018 Konstantin Pugin <ria.freelander@gmail.com>
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
#include "applet-info.h"
#include "config.h"

#include <glib/gi18n.h>

#define APPLET_INFO_GROUP "Plugin"

#define APPLET_INFO_NAME "Name"
#define APPLET_INFO_DESCRIPTION "Description"
#define APPLET_INFO_ICON "Icon"
#define APPLET_INFO_AUTHORS "Authors"
#define APPLET_INFO_WEBSITE "Website"
#define APPLET_INFO_HELP_URI "HelpURI"
#define APPLET_INFO_LICENSE "License"
#define APPLET_INFO_VERSION "Version"
#define APPLET_INFO_EXCLUSIVE "Exclusive"

struct _ValaPanelAppletInfo
{
	char *module_name;
	char *name;
	char *description;
	char *icon_name;
	char **authors;
	char *website;
	char *help_uri;
	char *version;
	bool exclusive;
	GtkLicense license;
};

static GtkLicense vala_panel_applet_info_get_license_from_name(const char *license_name)
{
	GEnumClass *enum_class = (GEnumClass *)g_type_class_ref(gtk_license_get_type());
	GEnumValue *val        = g_enum_get_value_by_name(enum_class, license_name);
	if (!val)
		val = g_enum_get_value_by_nick(enum_class, license_name);
	g_type_class_unref(enum_class);
	return val ? (GtkLicense)val->value : GTK_LICENSE_LGPL_3_0;
}

ValaPanelAppletInfo *vala_panel_applet_info_load(const char *extension_name)
{
	g_autoptr(GKeyFile) file      = g_key_file_new();
	g_autofree char *desktop_name = g_strdup_printf(PLUGINS_DATA "/%s.plugin", extension_name);
	g_autoptr(GError) err         = NULL;
	bool loaded =
	    g_key_file_load_from_file(file, desktop_name, G_KEY_FILE_KEEP_TRANSLATIONS, &err);
	if (err)
		g_critical("%s: %s\n", desktop_name, err->message);
	if (!loaded || err)
		return NULL;
	struct _ValaPanelAppletInfo *ret =
	    (struct _ValaPanelAppletInfo *)g_slice_alloc0(sizeof(struct _ValaPanelAppletInfo));
	char *tmp =
	    g_key_file_get_locale_string(file, APPLET_INFO_GROUP, APPLET_INFO_NAME, NULL, NULL);
	ret->module_name = g_strdup(extension_name);
	ret->name        = tmp != NULL ? tmp : g_strdup(_("Applet"));
	tmp              = g_key_file_get_locale_string(file,
                                           APPLET_INFO_GROUP,
                                           APPLET_INFO_DESCRIPTION,
                                           NULL,
                                           NULL);
	ret->description = tmp != NULL ? tmp : g_strdup(_("Vala Panel Applet"));
	tmp              = g_key_file_get_string(file, APPLET_INFO_GROUP, APPLET_INFO_ICON, NULL);
	ret->icon_name   = tmp != NULL ? tmp : g_strdup("package-x-generic");
	g_key_file_set_list_separator(file, ';');
	GStrv tmp_list =
	    g_key_file_get_string_list(file, APPLET_INFO_GROUP, APPLET_INFO_AUTHORS, NULL, NULL);
	ret->authors = tmp_list != NULL
	                   ? tmp_list
	                   : g_strsplit("Konstantin <ria.freelander@gmail.com>;", ";", 0);
	tmp = g_key_file_get_string(file, APPLET_INFO_GROUP, APPLET_INFO_WEBSITE, NULL);
	ret->website =
	    tmp != NULL ? tmp : g_strdup("https://gitlab.com/vala-panel-project/vala-panel");
	tmp =
	    g_key_file_get_locale_string(file, APPLET_INFO_GROUP, APPLET_INFO_HELP_URI, NULL, NULL);
	ret->help_uri =
	    tmp != NULL ? tmp
	                : g_strdup("https://gitlab.com/vala-panel-project/vala-panel/wikis/home");
	tmp = g_key_file_get_string(file, APPLET_INFO_GROUP, APPLET_INFO_LICENSE, NULL);
	ret->license =
	    tmp != NULL ? vala_panel_applet_info_get_license_from_name(tmp) : GTK_LICENSE_LGPL_3_0;
	g_clear_pointer(&tmp, g_free);
	tmp          = g_key_file_get_string(file, APPLET_INFO_GROUP, APPLET_INFO_VERSION, NULL);
	ret->version = tmp != NULL ? tmp : g_strdup(VERSION);
	ret->exclusive =
	    g_key_file_get_boolean(file, APPLET_INFO_GROUP, APPLET_INFO_EXCLUSIVE, NULL);
	return ret;
}

ValaPanelAppletInfo *vala_panel_applet_info_duplicate(void *info)
{
	struct _ValaPanelAppletInfo *ainfo = (struct _ValaPanelAppletInfo *)info;
	struct _ValaPanelAppletInfo *ret   = g_slice_alloc0(sizeof(struct _ValaPanelAppletInfo));
	ret->module_name                   = g_strdup(ainfo->module_name);
	ret->name                          = g_strdup(ainfo->name);
	ret->description                   = g_strdup(ainfo->description);
	ret->icon_name                     = g_strdup(ainfo->icon_name);
	if (ainfo->authors)
	{
		u_int32_t len = g_strv_length(ainfo->authors);
		ret->authors  = g_new0(char *, len + 1);
		for (uint i = 0; i < len; i++)
			ret->authors[i] = g_strdup(ainfo->authors[i]);
	}
	else
		ret->authors = NULL;
	ret->website   = g_strdup(ainfo->website);
	ret->help_uri  = g_strdup(ainfo->help_uri);
	ret->license   = ainfo->license;
	ret->version   = g_strdup(ainfo->version);
	ret->exclusive = ainfo->exclusive;
	return ret;
}

void vala_panel_applet_info_free(void *info)
{
	struct _ValaPanelAppletInfo *ainfo = (struct _ValaPanelAppletInfo *)info;
	g_clear_pointer(&ainfo->module_name, g_free);
	g_clear_pointer(&ainfo->name, g_free);
	g_clear_pointer(&ainfo->description, g_free);
	g_clear_pointer(&ainfo->icon_name, g_free);
	g_clear_pointer(&ainfo->authors, g_strfreev);
	g_clear_pointer(&ainfo->website, g_free);
	g_clear_pointer(&ainfo->help_uri, g_free);
	g_clear_pointer(&ainfo->version, g_free);
	g_slice_free(struct _ValaPanelAppletInfo, info);
}

G_DEFINE_BOXED_TYPE(ValaPanelAppletInfo, vala_panel_applet_info,
                    (GBoxedCopyFunc)vala_panel_applet_info_duplicate, vala_panel_applet_info_free)

const char *vala_panel_applet_info_get_module_name(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->module_name;
}

const char *vala_panel_applet_info_get_name(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->name;
}

const char *vala_panel_applet_info_get_description(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->description;
}

const char *vala_panel_applet_info_get_icon_name(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->icon_name;
}

const char *const *vala_panel_applet_info_get_authors(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->authors;
}

const char *vala_panel_applet_info_get_website(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->website;
}

const char *vala_panel_applet_info_get_help_uri(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->help_uri;
}

GtkLicense vala_panel_applet_info_get_license(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->license;
}

const char *vala_panel_applet_info_get_version(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->version;
}

bool vala_panel_applet_info_is_exclusive(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->exclusive;
}

static GtkDialog *vala_panel_applet_info_get_about_dialog(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	g_autoptr(GtkBuilder) builder =
	    gtk_builder_new_from_resource("/org/vala-panel/lib/about.ui");
	GtkAboutDialog *d =
	    GTK_ABOUT_DIALOG(gtk_builder_get_object(builder, "valapanel-plugin-about"));
	gtk_about_dialog_set_program_name(d, ainfo->name);
	gtk_about_dialog_set_comments(d, ainfo->description);
	gtk_about_dialog_set_logo_icon_name(d, ainfo->icon_name);
	gtk_about_dialog_set_license_type(d, ainfo->license);
	gtk_about_dialog_set_authors(d, (const char **)ainfo->authors);
	gtk_about_dialog_set_website(d, ainfo->website);
	gtk_about_dialog_set_version(d, ainfo->version);
	gtk_window_set_position(GTK_WINDOW(d), GTK_WIN_POS_CENTER);
	return GTK_DIALOG(d);
}

GtkWidget *vala_panel_applet_info_get_about_widget(ValaPanelAppletInfo *info)
{
	GtkDialog *d         = vala_panel_applet_info_get_about_dialog(info);
	GtkWidget *content   = gtk_dialog_get_content_area(d);
	GtkContainer *parent = GTK_CONTAINER(gtk_widget_get_parent(content));
	gtk_container_remove(parent, content);
	return content;
}

void vala_panel_applet_info_show_about_dialog(ValaPanelAppletInfo *info)
{
	GtkDialog *d = vala_panel_applet_info_get_about_dialog(info);
	gtk_window_present(GTK_WINDOW(d));
	g_signal_connect(d, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect(d, "response", G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect(d, "hide", G_CALLBACK(gtk_widget_destroy), NULL);
}
