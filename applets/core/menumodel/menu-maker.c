/*
 * vala-panel
 * Copyright (C) 2019 Konstantin Pugin <ria.freelander@gmail.com>
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
#include <stdbool.h>

#include "client.h"
#include "config.h"

static void menu_maker_parse_app_info(GDesktopAppInfo *info, GtkBuilder *builder)
{
	if (g_app_info_should_show(info))
	{
		GMenu *menu_link          = NULL;
		bool found                = false;
		g_autoptr(GMenuItem) item = g_menu_item_new(NULL, NULL);
		g_menu_item_set_label(item, g_app_info_get_name(info));
		g_menu_item_set_action_and_target(item,
		                                  "app.launch-id",
		                                  "s",
		                                  g_app_info_get_id(info));
		GIcon *icon = g_app_info_get_icon(info);
		if (icon)
			g_menu_item_set_icon(item, icon);
		else
			g_menu_item_set_attribute(item,
			                          G_MENU_ATTRIBUTE_ICON,
			                          "s",
			                          "application-x-executable");
		g_menu_item_set_attribute(item, ATTRIBUTE_DND_SOURCE, "b", true);
		if (g_app_info_get_description(info) != NULL)
			g_menu_item_set_attribute(item,
			                          ATTRIBUTE_TOOLTIP,
			                          "s",
			                          g_app_info_get_description(info));
		const char *cats_str = g_desktop_app_info_get_categories(info)
		                           ? g_desktop_app_info_get_categories(info)
		                           : " ";
		g_auto(GStrv) cats = g_strsplit_set(cats_str, ";", 0);
		for (int i = 0; cats[i]; i++)
		{
			g_autofree char *catdown = g_ascii_strdown(cats[i], -1);
			menu_link                = G_MENU(gtk_builder_get_object(builder, catdown));
			if (menu_link)
			{
				found = true;
				break;
			}
		}
		if (!found)
			menu_link = G_MENU(gtk_builder_get_object(builder, "other"));
		g_menu_append_item(menu_link, item);
	}
}

G_GNUC_INTERNAL GMenuModel *menu_maker_applications_model(const char **cats)
{
	g_autoptr(GtkBuilder) builder =
	    gtk_builder_new_from_resource("/org/vala-panel/menumodel/system-menus.ui");
	GMenu *menu = gtk_builder_get_object(builder, "applications-menu");
	g_object_ref_sink(menu);
	GList *list = g_app_info_get_all();
	for (GList *l = list; l; l = l->next)
	{
		menu_maker_parse_app_info(l->data, builder);
		g_clear_object(&l->data);
	}
	g_list_free(list);
	for (int i = 0; i < g_menu_model_get_n_items(menu); i++)
	{
		i                    = (i < 0) ? 0 : i;
		g_autofree char *cat = NULL;
		bool in_cat =
		    g_menu_model_get_item_attribute(menu, i, "x-valapanel-cat", "s", &cat);
		g_autoptr(GMenu) submenu =
		    G_MENU(g_menu_model_get_item_link(G_MENU_MODEL(menu), i, G_MENU_LINK_SUBMENU));
		if (g_menu_model_get_n_items(G_MENU_MODEL(submenu)) <= 0 ||
		    (in_cat && g_strv_contains(cats, cat)))
		{
			g_menu_remove(menu, i);
			i--;
		}

		if (i >= g_menu_model_get_n_items(menu) || g_menu_model_get_n_items(menu) <= 0)
			break;
	}
	g_menu_freeze(menu);
	return G_MENU_MODEL(menu);
}

G_GNUC_INTERNAL GMenuModel *menu_maker_create_applications_menu(bool do_settings)
{
	const char *apps_cats[]     = { "audiovideo",  "education", "game",   "graphics",
                                    "system",      "network",   "office", "utility",
                                    "development", "other",     NULL };
	const char *settings_cats[] = { "settings", NULL };
	if (do_settings)
		return menu_maker_applications_model(apps_cats);
	else
		return menu_maker_applications_model(settings_cats);
}

static GMenuItem *add_app_info_launch_item(GDesktopAppInfo *app_info)
{
	GMenuItem *item = g_menu_item_new(NULL, NULL);
	if (g_app_info_get_description(app_info) != NULL)
		g_menu_item_set_attribute(item,
		                          ATTRIBUTE_TOOLTIP,
		                          "s",
		                          g_app_info_get_description(app_info));
	g_menu_item_set_attribute(item, ATTRIBUTE_DND_SOURCE, "b", true);
	GVariant *idv = g_variant_new_string(g_app_info_get_id(G_APP_INFO(app_info)));
	g_menu_item_set_action_and_target_value(item, "app.launch-id", idv);
	return item;
}

G_GNUC_INTERNAL GMenuModel *menu_maker_create_places_menu()
{
	g_autoptr(GtkBuilder) builder =
	    gtk_builder_new_from_resource("/org/vala-panel/menumodel/system-menus.ui");
	GMenu *menu    = G_MENU(gtk_builder_get_object(builder, "places-menu"));
	GMenu *section = G_MENU(gtk_builder_get_object(builder, "folders-section"));
	g_object_ref_sink(menu);
	GMenuItem *item = g_menu_item_new(_("Home"), NULL);
	char *path      = g_filename_to_uri(g_get_home_dir(), NULL, NULL);
	g_menu_item_set_attribute(item, "icon", "s", "user-home");
	g_menu_item_set_action_and_target_value(item, "app.launch-uri", g_variant_new_string(path));
	g_menu_item_set_attribute(item, ATTRIBUTE_DND_SOURCE, "b", true);
	g_menu_append_item(section, item);
	g_clear_object(&item);
	g_clear_pointer(&path, g_free);
	item = g_menu_item_new(_("Desktop"), NULL);
	path = g_filename_to_uri(g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP), NULL, NULL);
	g_menu_item_set_attribute(item, "icon", "s", "user-desktop");
	g_menu_item_set_action_and_target_value(item, "app.launch-uri", g_variant_new_string(path));
	g_menu_append_item(section, item);
	g_clear_object(&item);
	g_clear_pointer(&path, g_free);
	section = G_MENU(gtk_builder_get_object(builder, "recent-section"));
	g_autoptr(GDesktopAppInfo) app_info = g_desktop_app_info_new("gnome-search-tool.desktop");
	if (!app_info)
		app_info = g_desktop_app_info_new("mate-search-tool.desktop");
	if (app_info)
	{
		item = add_app_info_launch_item(app_info);
		g_menu_item_set_label(item, _("Search..."));
		g_menu_item_set_attribute(item, "icon", "s", "system-search");
		g_menu_prepend_item(section, item);
		g_clear_object(&item);
	}
	return G_MENU_MODEL(menu);
}

G_GNUC_INTERNAL GMenuModel *menu_maker_create_system_menu()
{
	g_autoptr(GMenu) section = G_MENU(menu_maker_create_applications_menu(true));
	g_autoptr(GtkBuilder) builder =
	    gtk_builder_new_from_resource("/org/vala-panel/menumodel/system-menus.ui");
	GMenu *menu = G_MENU(gtk_builder_get_object(builder, "settings-section"));
	g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
	g_autoptr(GDesktopAppInfo) app_info =
	    g_desktop_app_info_new("gnome-control-center.desktop");
	if (!app_info)
		app_info = g_desktop_app_info_new("matecc.desktop");
	if (!app_info)
		app_info = g_desktop_app_info_new("cinnamon-settings.desktop");
	if (!app_info)
		app_info = g_desktop_app_info_new("xfce4-settings-manager.desktop");
	if (!app_info)
		app_info = g_desktop_app_info_new("kdesystemsettings.desktop");
	if (app_info)
	{
		g_autoptr(GMenuItem) item = add_app_info_launch_item(app_info);
		g_menu_item_set_label(item, _("Control center"));
		g_menu_item_set_attribute(item, "icon", "s", "preferences-system");
		g_menu_append_item(menu, item);
	}
	g_menu_freeze(menu);
	menu = G_MENU(gtk_builder_get_object(builder, "system-menu"));
	g_object_ref_sink(menu);
	g_menu_freeze(menu);
	return G_MENU_MODEL(menu);
}

GMenuModel *menu_maker_create_main_menu(bool as_submenus, const char *icon_str)
{
	GMenu *menu                  = g_menu_new();
	g_autoptr(GMenuModel) apps   = menu_maker_create_applications_menu(false);
	g_autoptr(GMenuModel) places = menu_maker_create_places_menu();
	g_autoptr(GMenuModel) system = menu_maker_create_system_menu();
	g_autoptr(GMenu) section     = g_menu_new();
	if (as_submenus)
	{
		g_autoptr(GMenuItem) item = g_menu_item_new_submenu(_("Applications"), apps);
		if (icon_str)
			g_menu_item_set_attribute(item, "icon", "s", icon_str);
		g_menu_append_item(menu, item);
		g_menu_append_submenu(menu, _("Places"), places);
		g_menu_append_submenu(menu, _("System"), system);
	}
	else
	{
		g_menu_append(menu, _("Vala Panel - " VERSION), "foo.improper-action");
		g_menu_append_section(menu, NULL, apps);
		g_menu_append_submenu(section, _("Places"), places);
		g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
		append_all_sections(menu, system);
	}
	g_menu_freeze(menu);
	return G_MENU_MODEL(menu);
}
