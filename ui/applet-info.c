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
#include "version.h"

#include <glib/gi18n.h>

#define INFO_GROUP "Plugin"

#define INFO_NAME "Name"
#define INFO_DESCRIPTION "Description"
#define INFO_COMMENT "Comment"
#define INFO_ICON "Icon"
#define INFO_AUTHORS "Authors"
#define INFO_WEBSITE "Website"
#define INFO_HELP_URI "HelpURI"
#define INFO_LICENSE "License"
#define INFO_VERSION "Version"
#define INFO_EXCLUSIVE "Exclusive"
#define INFO_PLATFORMS "Platforms"

struct _ValaPanelAppletInfo
{
	GType type;
	char *module_name;
	char *name;
	char *description;
	char *icon_name;
	char **authors;
	char *website;
	char *help_uri;
	char *version;
	char **platforms;
	bool exclusive;
	GtkLicense license;
};

#define fb(str, fbs) (str) != NULL ? (str) : (fbs)

static GtkLicense vp_applet_info_get_lic_from_name(const char *license_name)
{
	GEnumClass *enum_class = (GEnumClass *)g_type_class_ref(gtk_license_get_type());
	GEnumValue *val        = g_enum_get_value_by_name(enum_class, license_name);
	if (!val)
		val = g_enum_get_value_by_nick(enum_class, license_name);
	g_type_class_unref(enum_class);
	return val ? (GtkLicense)val->value : GTK_LICENSE_LGPL_3_0;
}

ValaPanelAppletInfo *vp_applet_info_load(const char *extension_name, GType plugin_type)
{
	g_autoptr(GKeyFile) file      = g_key_file_new();
	g_autofree char *desktop_name = g_strdup_printf(PLUGINS_DATA "/%s.plugin", extension_name);
	g_autoptr(GError) err         = NULL;
	bool loaded =
	    g_key_file_load_from_file(file, desktop_name, G_KEY_FILE_KEEP_TRANSLATIONS, &err);
	if (err)
		g_critical("%s: %s\n", desktop_name, err->message);
	if (!loaded || err || !G_TYPE_IS_CLASSED(plugin_type))
		return NULL;
	struct _ValaPanelAppletInfo *ret =
	    (struct _ValaPanelAppletInfo *)g_slice_alloc0(sizeof(struct _ValaPanelAppletInfo));
	ret->type        = plugin_type;
	char *tmp        = g_key_file_get_locale_string(file, INFO_GROUP, INFO_NAME, NULL, NULL);
	ret->module_name = g_strdup(extension_name);
	ret->name        = fb(tmp, g_strdup(_("Applet")));
	tmp = g_key_file_get_locale_string(file, INFO_GROUP, INFO_DESCRIPTION, NULL, NULL);
	g_autofree char *cmt =
	    g_key_file_get_locale_string(file, INFO_GROUP, INFO_COMMENT, NULL, NULL);
	ret->description = fb(cmt, fb(tmp, g_strdup(_("Vala Panel Applet"))));
	tmp              = g_key_file_get_string(file, INFO_GROUP, INFO_ICON, NULL);
	ret->icon_name   = fb(tmp, g_strdup("package-x-generic"));
	g_key_file_set_list_separator(file, ';');
	GStrv tmp_list = g_key_file_get_string_list(file, INFO_GROUP, INFO_AUTHORS, NULL, NULL);
	ret->authors   = fb(tmp_list, g_strsplit("Konstantin <ria.freelander@gmail.com>;", ";", 0));
	tmp_list       = g_key_file_get_string_list(file, INFO_GROUP, INFO_PLATFORMS, NULL, NULL);
	ret->platforms = fb(tmp_list, g_strsplit("all;", ";", 0));
	tmp            = g_key_file_get_string(file, INFO_GROUP, INFO_WEBSITE, NULL);
	ret->website   = fb(tmp, g_strdup("https://gitlab.com/vala-panel-project/vala-panel"));
	tmp            = g_key_file_get_locale_string(file, INFO_GROUP, INFO_HELP_URI, NULL, NULL);
	ret->help_uri =
	    fb(tmp, g_strdup("https://gitlab.com/vala-panel-project/vala-panel/wikis/home"));
	tmp          = g_key_file_get_string(file, INFO_GROUP, INFO_LICENSE, NULL);
	ret->license = tmp != NULL ? vp_applet_info_get_lic_from_name(tmp) : GTK_LICENSE_LGPL_3_0;
	g_clear_pointer(&tmp, g_free);
	tmp            = g_key_file_get_string(file, INFO_GROUP, INFO_VERSION, NULL);
	ret->version   = fb(tmp, g_strdup(VERSION));
	ret->exclusive = g_key_file_get_boolean(file, INFO_GROUP, INFO_EXCLUSIVE, NULL);
	return (ValaPanelAppletInfo *)ret;
}

ValaPanelAppletInfo *vp_applet_info_duplicate(void *info)
{
	struct _ValaPanelAppletInfo *ainfo = (struct _ValaPanelAppletInfo *)info;
	struct _ValaPanelAppletInfo *ret   = g_slice_alloc0(sizeof(struct _ValaPanelAppletInfo));
	ret->type                          = ainfo->type;
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
	if (ainfo->platforms)
	{
		u_int32_t len  = g_strv_length(ainfo->platforms);
		ret->platforms = g_new0(char *, len + 1);
		for (uint i = 0; i < len; i++)
			ret->platforms[i] = g_strdup(ainfo->platforms[i]);
	}
	else
		ret->platforms = NULL;
	ret->website   = g_strdup(ainfo->website);
	ret->help_uri  = g_strdup(ainfo->help_uri);
	ret->license   = ainfo->license;
	ret->version   = g_strdup(ainfo->version);
	ret->exclusive = ainfo->exclusive;
	return (ValaPanelAppletInfo *)ret;
}

void vp_applet_info_free(void *info)
{
	struct _ValaPanelAppletInfo *ainfo = (struct _ValaPanelAppletInfo *)info;
	g_clear_pointer(&ainfo->module_name, g_free);
	g_clear_pointer(&ainfo->name, g_free);
	g_clear_pointer(&ainfo->description, g_free);
	g_clear_pointer(&ainfo->icon_name, g_free);
	g_clear_pointer(&ainfo->authors, g_strfreev);
	g_clear_pointer(&ainfo->platforms, g_strfreev);
	g_clear_pointer(&ainfo->website, g_free);
	g_clear_pointer(&ainfo->help_uri, g_free);
	g_clear_pointer(&ainfo->version, g_free);
	g_slice_free(struct _ValaPanelAppletInfo, info);
}

G_DEFINE_BOXED_TYPE(ValaPanelAppletInfo, vp_applet_info,
                    (GBoxedCopyFunc)vp_applet_info_duplicate, vp_applet_info_free)

GType vp_applet_info_get_stored_type(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->type;
}

const char *vp_applet_info_get_module_name(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->module_name;
}

const char *vp_applet_info_get_name(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->name;
}

const char *vp_applet_info_get_description(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->description;
}

const char *vp_applet_info_get_icon_name(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->icon_name;
}

const char *const *vp_applet_info_get_authors(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return (const char *const *)ainfo->authors;
}

const char *const *vp_applet_info_get_platforms(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return (const char *const *)ainfo->platforms;
}

const char *vp_applet_info_get_website(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->website;
}

const char *vp_applet_info_get_help_uri(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->help_uri;
}

GtkLicense vp_applet_info_get_license(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->license;
}

const char *vp_applet_info_get_version(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->version;
}

bool vp_applet_info_is_exclusive(ValaPanelAppletInfo *info)
{
	struct _ValaPanelAppletInfo *ainfo = ((struct _ValaPanelAppletInfo *)info);
	return ainfo->exclusive;
}

static GtkDialog *vp_applet_info_get_about_dialog(ValaPanelAppletInfo *info)
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

GtkWidget *vp_applet_info_get_about_widget(ValaPanelAppletInfo *info)
{
	GtkDialog *d         = vp_applet_info_get_about_dialog(info);
	GtkWidget *content   = gtk_dialog_get_content_area(d);
	GtkContainer *parent = GTK_CONTAINER(gtk_widget_get_parent(content));
	gtk_container_remove(parent, content);
	return content;
}

void vp_applet_info_show_about_dialog(ValaPanelAppletInfo *info)
{
	GtkDialog *d = vp_applet_info_get_about_dialog(info);
	gtk_window_present(GTK_WINDOW(d));
	g_signal_connect(d, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect(d, "response", G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect(d, "hide", G_CALLBACK(gtk_widget_destroy), NULL);
}
