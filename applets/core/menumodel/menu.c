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

enum
{
	PROP_SYSTEM,
	PROP_INTERN,
	PROP_BAR,
	PROP_ICON,
	PROP_CAPTION,
	PROP_FILENAME
};

extern GMenuModel *menu_maker_create_applications_menu(bool do_settings);
extern GMenuModel *menu_maker_create_main_menu(bool submenus, const char *icon);

static GMenuModel *create_menumodel(MenuApplet *self);
static void menumodel_widget_rebuild(MenuApplet *self);
static GtkContainer *menumodel_widget_create(MenuApplet *self);

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
	gtk_widget_show(c->button);
	gtk_widget_show(c);
	return c;
}

static GObject *menu_applet_constructor(GType type, guint n_construct_properties,
                                        GObjectConstructParam *construct_properties)
{
	GObjectClass *parent_class = G_OBJECT_CLASS(menu_applet_parent_class);
	GObject *obj =
	    parent_class->constructor(type, n_construct_properties, construct_properties);
	MenuApplet *self = VALA_PANEL_MENU_APPLET(obj);
	GActionMap *map = G_ACTION_MAP(vala_panel_applet_get_action_group(VALA_PANEL_APPLET(self)));
	g_simple_action_set_enabled(
	    G_SIMPLE_ACTION(g_action_map_lookup_action(map, VALA_PANEL_APPLET_ACTION_CONFIGURE)),
	    true);
	g_simple_action_set_enabled(
	    G_SIMPLE_ACTION(g_action_map_lookup_action(map, VALA_PANEL_APPLET_ACTION_REMOTE)),
	    true);
	self->button        = NULL;
	GSettings *settings = vala_panel_applet_get_settings(VALA_PANEL_APPLET(self));
	g_settings_bind(settings, IS_SYSTEM_MENU, self, "system", G_SETTINGS_BIND_GET);
	g_settings_bind(settings, IS_MENU_BAR, self, "bar", G_SETTINGS_BIND_GET);
	g_settings_bind(settings, IS_INTERNAL_MENU, self, "intern", G_SETTINGS_BIND_GET);
	g_settings_bind(settings, MODEL_FILE, self, "filename", G_SETTINGS_BIND_GET);
	g_settings_bind(settings, ICON, self, "icon", G_SETTINGS_BIND_GET);
	g_settings_bind(settings, CAPTION, self, "caption", G_SETTINGS_BIND_GET);
	GtkContainer *w = menumodel_widget_create(self);
	self->button    = w;
	gtk_container_add(self, self->button);
	GtkSettings *gtksettings = gtk_widget_get_settings(self);
	g_object_set(gtksettings, "gtk-shell-shows-menubar", false, NULL);
	return obj;
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
		g_source_remove(self->monitor_update_idle);
	if (self->show_system_menu_idle)
		g_source_remove(self->show_system_menu_idle);
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

static void menu_applet_get_property(GObject *object, uint prop_id, GValue *value,
                                     GParamSpec *pspec)
{
	MenuApplet *self = VALA_PANEL_MENU_APPLET(object);
	switch (prop_id)
	{
	case PROP_BAR:
		g_value_set_boolean(value, self->bar);
		break;
	case PROP_INTERN:
		g_value_set_boolean(value, self->intern);
		break;
	case PROP_SYSTEM:
		g_value_set_boolean(value, self->system);
		break;
	case PROP_ICON:
		g_value_set_string(value, self->icon);
		break;
	case PROP_CAPTION:
		g_value_set_string(value, self->caption);
		break;
	case PROP_FILENAME:
		g_value_set_string(value, self->filename);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void menu_applet_set_property(GObject *object, uint prop_id, const GValue *value,
                                     GParamSpec *pspec)
{
	MenuApplet *self = VALA_PANEL_MENU_APPLET(object);
	switch (prop_id)
	{
	case PROP_BAR:
		self->bar = g_value_get_boolean(value);
		menumodel_widget_rebuild(self);
		break;
	case PROP_INTERN:
		self->intern = g_value_get_boolean(value);
		menumodel_widget_rebuild(self);
		break;
	case PROP_SYSTEM:
		self->system = g_value_get_boolean(value);
		break;
	case PROP_ICON:
		g_free0(self->icon);
		self->icon = g_value_dup_string(value);
		if (!self->bar)
			menumodel_widget_rebuild(self);
		else
		{
			GtkImage *image       = gtk_button_get_image(self->button);
			g_autoptr(GError) err = NULL;
			g_autoptr(GIcon) icon = g_icon_new_for_string(self->icon, &err);
			if (err)
				break;
			g_object_set(image, "gicon", icon, NULL);
		}
		break;
	case PROP_CAPTION:
		g_free0(self->caption);
		self->caption = g_value_dup_string(value);
		if (!self->bar)
			gtk_button_set_label(self->button, self->caption);
		break;
	case PROP_FILENAME:
		g_free0(self->icon);
		self->caption = g_value_dup_string(value);
		if (!self->intern)
			menumodel_widget_rebuild(self);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static GtkWidget *menu_applet_get_settings_ui(ValaPanelApplet *self)
{
	GSettings *settings = vala_panel_applet_get_settings(self);
	return generic_config_widget(settings,
	                             _("If internal menu is enabled, menu file will not be used,\n "
	                               "predefeined menu will be used instead."),
	                             NULL,
	                             CONF_TRIM,
	                             _("Is internal menu"),
	                             IS_INTERNAL_MENU,
	                             CONF_BOOL,
	                             _("Is system menu (can be keybound)"),
	                             IS_SYSTEM_MENU,
	                             CONF_BOOL,
	                             _("Is Menubar"),
	                             IS_MENU_BAR,
	                             CONF_BOOL,
	                             _("Icon"),
	                             ICON,
	                             CONF_FILE_ENTRY,
	                             _("Caption (for button only)"),
	                             CAPTION,
	                             CONF_STR,
	                             _("Menu file name"),
	                             MODEL_FILE,
	                             CONF_FILE_ENTRY,
	                             NULL);
}

static int show_menu_int(void *data)
{
	MenuApplet *self = VALA_PANEL_MENU_APPLET(data);
	if (g_source_is_destroyed(g_main_current_source()))
		return false;
	if (GTK_IS_MENU(self->int_menu))
		gtk_menu_popup_at_widget(self->int_menu,
		                         self,
		                         GDK_GRAVITY_NORTH,
		                         GDK_GRAVITY_NORTH,
		                         NULL);
	else
		gtk_menu_shell_select_first(GTK_MENU_SHELL(self->button), false);
	self->show_system_menu_idle = 0;
	return false;
}

static bool menu_applet_remote_command(ValaPanelApplet *obj, const char *command_name)
{
	MenuApplet *self = VALA_PANEL_MENU_APPLET(obj);
	if (!g_strcmp0(command_name, "menu") && self->system && self->show_system_menu_idle == 0)
	{
		g_timeout_add(200, show_menu_int, self);
		return true;
	}
	return false;
}

static void menu_applet_init(MenuApplet *self)
{
}

static void menu_applet_class_init(MenuAppletClass *klass)
{
	G_OBJECT_CLASS(klass)->constructor              = menu_applet_constructor;
	G_OBJECT_CLASS(klass)->get_property             = menu_applet_get_property;
	G_OBJECT_CLASS(klass)->set_property             = menu_applet_set_property;
	G_OBJECT_CLASS(klass)->dispose                  = menu_applet_dispose;
	VALA_PANEL_APPLET_CLASS(klass)->get_settings_ui = menu_applet_get_settings_ui;
	VALA_PANEL_APPLET_CLASS(klass)->remote_command  = menu_applet_remote_command;
}

static void menu_applet_class_finalize(MenuAppletClass *klass)
{
}
