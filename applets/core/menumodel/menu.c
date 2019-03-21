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

#include <stdbool.h>

#include "menu.h"

#define ICON "icon-name"
#define CAPTION "menu-caption"
#define IS_MENU_BAR "is-menu-bar"
#define IS_SYSTEM_MENU "is-system-menu"
#define IS_INTERNAL_MENU "is-internal-menu"
#define MODEL_FILE "model-file"

typedef enum
{
	APPLICATIONS,
	SETTINGS,
	MOUNTS,
	RECENT
} MenuInternalEnum;

extern GMenuModel *menu_maker_create_applications_menu(bool do_settings);
extern GMenuModel *menu_maker_create_main_menu(bool submenus, const char *icon);

/* Private context for Menu applet. */
struct _MenuApplet
{
	ValaPanelApplet parent;
	GMenu *menu;
	GtkContainer *button;
	GtkMenu *int_menu;
	GAppInfoMonitor *app_monitor;
	GFileMonitor *file_monitor;
	guint show_system_menu_idle;
	bool system;
	bool intern;
	bool bar;
	char *icon;
	char *caption;
	char *filename;
	guint monitor_update_idle;
};

G_DEFINE_DYNAMIC_TYPE(MenuApplet, menu_applet, vala_panel_applet_get_type())

/* Applet widget constructor. */
MenuApplet *menu_applet_new(ValaPanelToplevel *toplevel, GSettings *settings, const char *uuid)
{
	/* Allocate applet context*/
	MenuApplet *c = VALA_PANEL_MENU_APPLET(
	    vala_panel_applet_construct(menu_applet_get_type(), toplevel, settings, uuid));

	return c;
}

static void load_internal_menus(GMenu *menu, MenuInternalEnum enum_id)
{
	g_autoptr(GMenuModel) section = NULL;

	if (enum_id == APPLICATIONS)
	{
		section = menu_maker_create_applications_menu(false);
		g_menu_append_section(menu, NULL, section);
	}
	if (enum_id == SETTINGS)
	{
		section = menu_maker_create_applications_menu(true);
		g_menu_append_section(menu, NULL, section);
	}
}

static GMenuModel *read_menumodel(MenuApplet *m)
{
	GMenu *gotten;
	g_autoptr(GtkBuilder) builder = gtk_builder_new();
	g_autoptr(GError) err         = NULL;
	gtk_builder_add_from_file(builder, m->filename, &err);
	if (err)
	{
		fprintf(stderr, "%s\n", err->message);
		return NULL;
	}
	GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "vala-panel-menu"));
	gotten = G_MENU(gtk_builder_get_object(builder, "vala-panel-internal-applications"));
	if (gotten)
		load_internal_menus(gotten, APPLICATIONS);
	gotten = G_MENU(gtk_builder_get_object(builder, "vala-panel-internal-settings"));
	if (gotten)
		load_internal_menus(gotten, SETTINGS);
	gotten = G_MENU(gtk_builder_get_object(builder, "vala-panel-internal-mounts"));
	if (gotten)
		load_internal_menus(gotten, MOUNTS);
	gotten = G_MENU(gtk_builder_get_object(builder, "vala-panel-internal-recent"));
	if (gotten)
		load_internal_menus(gotten, RECENT);
	g_object_ref(menu);
	return menu;
}

static void menu_applet_init(MenuApplet *self)
{
}

static void menu_applet_class_init(MenuAppletClass *klass)
{
	//    G_OBJECT_CLASS(klass)->dispose = cpu_applet_dispose;
}

static void menu_applet_class_finalize(MenuAppletClass *klass)
{
}
