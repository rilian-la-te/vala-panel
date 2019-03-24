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

static GMenuModel *create_menumodel(MenuApplet *self);
static void menumodel_widget_rebuild(MenuApplet *self);

/* Private context for Menu applet. */
struct _MenuApplet
{
	ValaPanelApplet parent;
	GMenu *menu;
	GtkContainer *button;
	GtkMenu *int_menu;
	GAppInfoMonitor *app_monitor;
	GFileMonitor *file_monitor;
	uint show_system_menu_idle;
	uint monitor_update_idle;
	bool system;
	bool intern;
	bool bar;
	char *icon;
	char *caption;
	char *filename;
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

static GtkContainer *menumodel_widget_create(MenuApplet *self)
{
	self->menu        = create_menumodel(self);
	GtkContainer *ret = NULL;
	//    if (bar)
	//        ret = create_menubar() as Container;
	//    else
	//        ret = create_menubutton() as Container;
	return ret;
}

static void menumodel_widget_destroy(MenuApplet *self)
{
	vala_panel_applet_set_background_widget(self, self);
	if (self->monitor_update_idle)
	{
		g_source_remove(self->monitor_update_idle);
	}
	if (GTK_IS_WIDGET(self->int_menu))
	{
		if (self->button)
			gtk_menu_detach(self->int_menu);
		gtk_widget_destroy0(self->int_menu);
	}
	if (self->app_monitor)
	{
		g_signal_handlers_disconnect_by_data(self->app_monitor, self);
		g_clear_object(&self->app_monitor);
	}
	if (self->file_monitor)
	{
		g_signal_handlers_disconnect_by_data(self->file_monitor, self);
		g_clear_object(&self->file_monitor);
	}
}

static int monitor_update_idle(gpointer user_data)
{
	MenuApplet *m = VALA_PANEL_MENU_APPLET(user_data);
	if (g_source_is_destroyed(g_main_current_source()))
		return false;
	menumodel_widget_rebuild(m);
	m->monitor_update_idle = 0;
	return false;
}

static void monitor_update(MenuApplet *m)
{
	if (m && m->monitor_update_idle == 0)
		m->monitor_update_idle = g_timeout_add(200, monitor_update_idle, m);
}

static void menumodel_widget_rebuild(MenuApplet *self)
{
	menumodel_widget_destroy(self);
	self->button = menumodel_widget_create(self);
	gtk_container_add(self, self->button);
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

static GMenuModel *create_menumodel(MenuApplet *m)
{
	GMenuModel *ret;
	if (m->intern)
	{
		ret            = menu_maker_create_main_menu(m->bar, m->icon);
		m->app_monitor = g_app_info_monitor_get();
		g_signal_connect_swapped(m->app_monitor, "changed", G_CALLBACK(monitor_update), m);
		m->file_monitor = NULL;
	}
	else
	{
		if (!m->filename)
			return NULL;
		g_autoptr(GFile) f = g_file_new_for_path(m->filename);
		ret                = read_menumodel(m);
		m->app_monitor     = NULL;
		m->file_monitor =
		    g_file_monitor_file(f,
		                        G_FILE_MONITOR_SEND_MOVED | G_FILE_MONITOR_WATCH_MOVES,
		                        NULL,
		                        NULL);
		g_signal_connect_swapped(m->file_monitor, "changed", G_CALLBACK(monitor_update), m);
	}
	return ret;
}

static void menu_applet_dispose(GObject *user_data)
{
	MenuApplet *self = VALA_PANEL_MENU_APPLET(user_data);
	menumodel_widget_destroy(self);
	g_free0(self->icon);
	g_free0(self->filename);
	g_free0(self->caption);
	G_OBJECT_CLASS(menu_applet_parent_class)->dispose(user_data);
}

static void menu_applet_init(MenuApplet *self)
{
}

static void menu_applet_class_init(MenuAppletClass *klass)
{
	G_OBJECT_CLASS(klass)->dispose = menu_applet_dispose;
}

static void menu_applet_class_finalize(MenuAppletClass *klass)
{
}
