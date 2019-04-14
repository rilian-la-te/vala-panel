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
#define IS_BAR "is-menu-bar"
#define IS_SYSTEM "is-system-menu"
#define IS_INTERNAL "is-internal-menu"
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
	PROP_FIRST,
	PROP_SYSTEM,
	PROP_INTERN,
	PROP_BAR,
	PROP_ICON,
	PROP_CAPTION,
	PROP_FILENAME,
	NUM_PROPERTIES
};

extern GMenuModel *menu_maker_create_applications_menu(bool do_settings);
extern GMenuModel *menu_maker_create_main_menu(bool submenus, const char *icon);

static GParamSpec *menu_applet_props[NUM_PROPERTIES];
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
	MenuApplet *self = VALA_PANEL_MENU_APPLET(
	    vala_panel_applet_construct(menu_applet_get_type(), toplevel, settings, uuid));
	return self;
}

static void menu_applet_constructed(GObject *obj)
{
	G_OBJECT_CLASS(menu_applet_parent_class)->constructed(obj);
	MenuApplet *self = VALA_PANEL_MENU_APPLET(obj);
	GActionMap *map  = vala_panel_applet_get_action_group(VALA_PANEL_APPLET(self));
	g_simple_action_set_enabled(
	    G_SIMPLE_ACTION(g_action_map_lookup_action(map, VALA_PANEL_APPLET_ACTION_CONFIGURE)),
	    true);
	g_simple_action_set_enabled(
	    G_SIMPLE_ACTION(g_action_map_lookup_action(map, VALA_PANEL_APPLET_ACTION_REMOTE)),
	    true);
	self->button        = NULL;
	GSettings *settings = vala_panel_applet_get_settings(VALA_PANEL_APPLET(self));
	g_settings_bind(settings, IS_SYSTEM, self, IS_SYSTEM, G_SETTINGS_BIND_GET);
	g_settings_bind(settings, IS_BAR, self, IS_BAR, G_SETTINGS_BIND_GET);
	g_settings_bind(settings, IS_INTERNAL, self, IS_INTERNAL, G_SETTINGS_BIND_GET);
	g_settings_bind(settings, MODEL_FILE, self, MODEL_FILE, G_SETTINGS_BIND_GET);
	g_settings_bind(settings, ICON, self, ICON, G_SETTINGS_BIND_GET);
	g_settings_bind(settings, CAPTION, self, CAPTION, G_SETTINGS_BIND_GET);
	GtkSettings *gtksettings = gtk_widget_get_settings(self);
	g_object_set(gtksettings, "gtk-shell-shows-menubar", false, NULL);
	gtk_widget_show(self);
}

static void panel_gravity_changed(ValaPanelToplevel *panel, GParamSpec *param, GtkMenuBar *menu)
{
	GtkOrientation orient;
	GtkPackDirection pack;
	g_object_get(panel, VP_KEY_ORIENTATION, &orient, NULL);
	pack =
	    orient == GTK_ORIENTATION_HORIZONTAL ? GTK_PACK_DIRECTION_LTR : GTK_PACK_DIRECTION_TTB;
	gtk_menu_bar_set_pack_direction(menu, pack);
}

static GtkContainer *create_menubar(MenuApplet *self)
{
	ValaPanelToplevel *top   = vala_panel_applet_get_toplevel(self);
	GtkMenuBar *menubar      = gtk_menu_bar_new_from_model(G_MENU_MODEL(self->menu));
	g_autoptr(GList) ch_list = gtk_container_get_children(GTK_CONTAINER(menubar));
	apply_menu_properties(ch_list, self->menu);
	vala_panel_applet_set_background_widget(self, menubar);
	vala_panel_applet_init_background(self);
	gtk_widget_show(menubar);
	panel_gravity_changed(top, NULL, menubar);
	g_signal_connect(top,
	                 "notify::" VP_KEY_GRAVITY,
	                 G_CALLBACK(panel_gravity_changed),
	                 menubar);
	return GTK_CONTAINER(menubar);
}

static void on_menubutton_toggled(GtkToggleButton *b, void *data)
{
	MenuApplet *self = VALA_PANEL_MENU_APPLET(data);
	if (gtk_toggle_button_get_active(b) && !gtk_widget_get_visible(self->int_menu))
		gtk_menu_popup_at_widget(self->int_menu,
		                         self,
		                         GDK_GRAVITY_NORTH,
		                         GDK_GRAVITY_NORTH,
		                         NULL);
	else
		gtk_menu_popdown(self->int_menu);
}

static void on_menu_hide(GtkMenu *menu, void *data)
{
	GtkToggleButton *btn = GTK_TOGGLE_BUTTON(data);
	gtk_toggle_button_set_active(btn, false);
}

static void menubutton_create_image(MenuApplet *self, GtkToggleButton *menubutton)
{
	GtkImage *img = NULL;
	if (self->icon)
	{
		img                    = gtk_image_new();
		g_autoptr(GError) err  = NULL;
		g_autoptr(GIcon) gicon = g_icon_new_for_string(self->icon, &err);
		if (!err)
			vala_panel_setup_icon(img, gicon, vala_panel_applet_get_toplevel(self), -1);
		gtk_widget_show(img);
	}
	vala_panel_setup_button(menubutton, img, self->caption);
}

static GtkContainer *create_menubutton(MenuApplet *self)
{
	GtkImage *img               = NULL;
	GtkToggleButton *menubutton = gtk_toggle_button_new();
	if (!self->menu)
		return menubutton;
	self->int_menu           = gtk_menu_new_from_model(self->menu);
	g_autoptr(GList) ch_list = gtk_container_get_children(GTK_CONTAINER(self->int_menu));
	apply_menu_properties(ch_list, self->menu);
	g_clear_pointer(&ch_list, g_list_free);
	gtk_menu_attach_to_widget(self->int_menu, menubutton, NULL);
	g_signal_connect(menubutton, "toggled", G_CALLBACK(on_menubutton_toggled), self);
	g_signal_connect(self->int_menu, "hide", G_CALLBACK(on_menu_hide), menubutton);
	menubutton_create_image(self, menubutton);
	gtk_widget_show(menubutton);
	return menubutton;
}

static GtkContainer *menumodel_widget_create(MenuApplet *self)
{
	self->menu        = create_menumodel(self);
	GtkContainer *ret = NULL;
	if (!self->menu)
	{
		return ret;
	}
	if (self->bar)
		ret = create_menubar(self);
	else
		ret = create_menubutton(self);
	return ret;
}

static void menumodel_widget_destroy(MenuApplet *self)
{
	vala_panel_applet_set_background_widget(self, self);
	if (self->monitor_update_idle)
		g_source_remove(self->monitor_update_idle);
	if (self->show_system_menu_idle)
		g_source_remove(self->show_system_menu_idle);
	if (self->button)
	{
		g_signal_handlers_disconnect_by_data(vala_panel_applet_get_toplevel(self),
		                                     self->button);
		g_signal_handlers_disconnect_by_data(self->button, self);
	}
	if (GTK_IS_WIDGET(self->int_menu))
	{
		if (self->button)
		{
			g_signal_handlers_disconnect_by_data(self->int_menu, self->button);
			gtk_menu_detach(self->int_menu);
		}
		if (GTK_IS_WIDGET(self->int_menu))
			gtk_widget_destroy0(self->int_menu);
	}
	gtk_widget_destroy0(self->button);
	if (G_IS_OBJECT(self->menu))
		g_clear_object(&self->menu);
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
	if (GTK_IS_WIDGET(self->button))
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
	g_object_ref_sink(menu);
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

	G_OBJECT_CLASS(menu_applet_parent_class)->dispose(user_data);
}

static void menu_applet_finalize(GObject *user_data)
{
	MenuApplet *self = VALA_PANEL_MENU_APPLET(user_data);
	g_free0(self->icon);
	g_free0(self->filename);
	g_free0(self->caption);
	G_OBJECT_CLASS(menu_applet_parent_class)->finalize(user_data);
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
		if (self->bar)
			menumodel_widget_rebuild(self);
		else
		{
			GtkImage *image = gtk_button_get_image(self->button);
			if (!image)
			{
				menubutton_create_image(self, self->button);
				image = gtk_button_get_image(self->button);
			}
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
	                             IS_INTERNAL,
	                             CONF_BOOL,
	                             _("Is system menu (can be keybound)"),
	                             IS_SYSTEM,
	                             CONF_BOOL,
	                             _("Is Menubar"),
	                             IS_BAR,
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
	G_OBJECT_CLASS(klass)->constructed              = menu_applet_constructed;
	G_OBJECT_CLASS(klass)->get_property             = menu_applet_get_property;
	G_OBJECT_CLASS(klass)->set_property             = menu_applet_set_property;
	G_OBJECT_CLASS(klass)->dispose                  = menu_applet_dispose;
	G_OBJECT_CLASS(klass)->finalize                 = menu_applet_finalize;
	VALA_PANEL_APPLET_CLASS(klass)->get_settings_ui = menu_applet_get_settings_ui;
	VALA_PANEL_APPLET_CLASS(klass)->remote_command  = menu_applet_remote_command;
	menu_applet_props[PROP_BAR] =
	    g_param_spec_boolean(IS_BAR,
	                         IS_BAR,
	                         IS_BAR,
	                         false,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	menu_applet_props[PROP_ICON] =
	    g_param_spec_string(ICON, ICON, ICON, NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	menu_applet_props[PROP_INTERN] =
	    g_param_spec_boolean(IS_INTERNAL,
	                         IS_INTERNAL,
	                         IS_INTERNAL,
	                         false,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	menu_applet_props[PROP_CAPTION] =
	    g_param_spec_string(CAPTION,
	                        CAPTION,
	                        CAPTION,
	                        NULL,
	                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	menu_applet_props[PROP_SYSTEM] =
	    g_param_spec_boolean(IS_SYSTEM,
	                         IS_SYSTEM,
	                         IS_SYSTEM,
	                         false,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	menu_applet_props[PROP_FILENAME] =
	    g_param_spec_string(MODEL_FILE,
	                        MODEL_FILE,
	                        MODEL_FILE,
	                        NULL,
	                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties(G_OBJECT_CLASS(klass), NUM_PROPERTIES, menu_applet_props);
}

static void menu_applet_class_finalize(MenuAppletClass *klass)
{
}

/*
 * IO Module functions
 */

void g_io_menumodel_load(GTypeModule *module)
{
	g_return_if_fail(module != NULL);

	menu_applet_register_type(module);

	g_type_module_use(module);
	g_io_extension_point_implement(VALA_PANEL_APPLET_EXTENSION_POINT,
	                               menu_applet_get_type(),
	                               "org.valapanel.menumodel",
	                               10);
}

void g_io_menumodel_unload(GIOModule *module)
{
	g_return_if_fail(module != NULL);
}
