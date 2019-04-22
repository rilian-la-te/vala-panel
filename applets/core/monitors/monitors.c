/*
 * vala-panel
 * Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
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

#include "monitors.h"
#include "cpu.h"
#include "mem.h"
#include "monitor.h"
#include "swap.h"

#define DEFAULT_WIDTH 40 /* Pixels               */
#define UPDATE_PERIOD 1  /* Seconds */

#define ACTION "click-action"

enum
{
	CPU_POS = 0,
	RAM_POS,
	SWAP_POS,
	N_POS
};

struct _MonitorsApplet
{
	ValaPanelApplet _parent_;
	Monitor *monitors[N_POS];
	bool displayed_mons[N_POS];
	uint timer;
};

G_DEFINE_DYNAMIC_TYPE(MonitorsApplet, monitors_applet, vala_panel_applet_get_type())

/*
 * Generic monitor functions and events
 */

static bool button_release_event(G_GNUC_UNUSED GtkWidget *widget, GdkEventButton *evt,
                                 MonitorsApplet *applet)
{
	g_autoptr(GVariant) var =
	    g_settings_get_value(vala_panel_applet_get_settings(VALA_PANEL_APPLET(applet)), ACTION);
	if (evt->button == 1 &&
	    g_variant_type_is_subtype_of(g_variant_get_type(var), G_VARIANT_TYPE_STRING))
	{
		activate_menu_launch_command(NULL,
		                             var,
		                             gtk_window_get_application(
		                                 GTK_WINDOW(vala_panel_applet_get_toplevel(
		                                     VALA_PANEL_APPLET(applet)))));
		return true;
	}
	return false;
}

static void monitor_setup_size(Monitor *mon, MonitorsApplet *pl, int width)
{
	int height;
	g_object_get(vala_panel_applet_get_toplevel(VALA_PANEL_APPLET(pl)),
	             VP_KEY_HEIGHT,
	             &height,
	             NULL);
	gtk_widget_set_size_request(GTK_WIDGET(mon->da), width, height);
	monitor_resize(GTK_WIDGET(mon->da), mon);
}

static void monitor_init(Monitor *mon, MonitorsApplet *pl, const char *color, int width)
{
	monitor_init_no_height(mon, color);
	monitor_setup_size(mon, pl, width);
	g_signal_connect(mon->da, "button-release-event", G_CALLBACK(button_release_event), pl);
}

static Monitor *monitor_create(GtkBox *monitor_box, MonitorsApplet *pl, update_func update,
                               tooltip_update_func tooltip_update, const char *color, int width)
{
	Monitor *m = g_new0(Monitor, 1);
	monitor_init(m, pl, color, width);
	m->update         = update;
	m->tooltip_update = tooltip_update;
	gtk_box_pack_start(GTK_BOX(monitor_box), GTK_WIDGET(m->da), false, false, 0);
	gtk_widget_show(GTK_WIDGET(m->da));
	return m;
}

/*
 * Applet functions
 */

static Monitor *create_monitor_with_pos(MonitorsApplet *self, int pos)
{
	GSettings *settings = vala_panel_applet_get_settings(VALA_PANEL_APPLET(self));
	if (pos == CPU_POS)
	{
		g_autofree char *color = g_settings_get_string(settings, CPU_CL);
		int width              = g_settings_get_int(settings, CPU_WIDTH);
		return monitor_create(GTK_BOX(gtk_bin_get_child(GTK_BIN(self))),
		                      self,
		                      cpu_update,
		                      tooltip_update_cpu,
		                      color,
		                      width);
	}
	if (pos == RAM_POS)
	{
		g_autofree char *color = g_settings_get_string(settings, RAM_CL);
		int width              = g_settings_get_int(settings, RAM_WIDTH);
		return monitor_create(GTK_BOX(gtk_bin_get_child(GTK_BIN(self))),
		                      self,
		                      update_mem,
		                      tooltip_update_mem,
		                      color,
		                      width);
	}
	if (pos == SWAP_POS)
	{
		g_autofree char *color = g_settings_get_string(settings, SWAP_CL);
		int width              = g_settings_get_int(settings, SWAP_WIDTH);
		return monitor_create(GTK_BOX(gtk_bin_get_child(GTK_BIN(self))),
		                      self,
		                      update_swap,
		                      tooltip_update_swap,
		                      color,
		                      width);
	}
	return NULL;
}

static int monitors_update(void *data)
{
	MonitorsApplet *self = VALA_PANEL_MONITORS_APPLET(data);
	if (g_source_is_destroyed(g_main_current_source()))
		return false;
	for (int i = 0; i < N_POS; i++)
	{
		if (self->monitors[i] != NULL)
			monitor_update(self->monitors[i]);
	}
	return true;
}

static void rebuild_mon(MonitorsApplet *self, int i)
{
	if (self->displayed_mons[i] && self->monitors[i] == NULL)
	{
		self->monitors[i] = create_monitor_with_pos(self, i);
		gtk_box_reorder_child(GTK_BOX(gtk_bin_get_child(GTK_BIN(self))),
		                      GTK_WIDGET(self->monitors[i]->da),
		                      i);
	}
	else if (!self->displayed_mons[i] && self->monitors[i] != NULL)
	{
		g_clear_pointer(&self->monitors[i], monitor_dispose);
	}
}

void on_settings_changed(GSettings *settings, char *key, gpointer user_data)
{
	MonitorsApplet *self = VALA_PANEL_MONITORS_APPLET(user_data);
	if (!g_strcmp0(key, DISPLAY_CPU))
	{
		self->displayed_mons[CPU_POS] = g_settings_get_boolean(settings, DISPLAY_CPU);
		rebuild_mon(self, CPU_POS);
	}
	else if ((!g_strcmp0(key, CPU_CL)) && self->monitors[CPU_POS] != NULL)
	{
		g_autofree char *color = g_settings_get_string(settings, CPU_CL);
		gdk_rgba_parse(&self->monitors[CPU_POS]->foreground_color, color);
	}
	else if (!g_strcmp0(key, DISPLAY_RAM))
	{
		self->displayed_mons[RAM_POS] = g_settings_get_boolean(settings, DISPLAY_RAM);
		rebuild_mon(self, RAM_POS);
	}
	else if ((!g_strcmp0(key, RAM_CL)) && self->monitors[RAM_POS] != NULL)
	{
		g_autofree char *color = g_settings_get_string(settings, RAM_CL);
		gdk_rgba_parse(&self->monitors[RAM_POS]->foreground_color, color);
	}
	else if (!g_strcmp0(key, DISPLAY_SWAP))
	{
		self->displayed_mons[SWAP_POS] = g_settings_get_boolean(settings, DISPLAY_SWAP);
		rebuild_mon(self, SWAP_POS);
	}
	else if ((!g_strcmp0(key, SWAP_CL)) && self->monitors[SWAP_POS] != NULL)
	{
		g_autofree char *color = g_settings_get_string(settings, SWAP_CL);
		gdk_rgba_parse(&self->monitors[SWAP_POS]->foreground_color, color);
	}
	else if ((!g_strcmp0(key, CPU_WIDTH)) && self->monitors[CPU_POS] != NULL)
	{
		int width = g_settings_get_int(settings, CPU_WIDTH);
		monitor_setup_size(self->monitors[CPU_POS], self, width);
	}
	else if ((!g_strcmp0(key, RAM_WIDTH)) && self->monitors[RAM_POS] != NULL)
	{
		int width = g_settings_get_int(settings, RAM_WIDTH);
		monitor_setup_size(self->monitors[RAM_POS], self, width);
	}
	else if ((!g_strcmp0(key, SWAP_WIDTH)) && self->monitors[SWAP_POS] != NULL)
	{
		int width = g_settings_get_int(settings, SWAP_WIDTH);
		monitor_setup_size(self->monitors[SWAP_POS], self, width);
	}
}

/* Applet widget constructor. */
MonitorsApplet *monitors_applet_new(ValaPanelToplevel *toplevel, GSettings *settings,
                                    const char *uuid)
{
	/* Allocate applet context*/
	MonitorsApplet *self = VALA_PANEL_MONITORS_APPLET(
	    vala_panel_applet_construct(monitors_applet_get_type(), toplevel, settings, uuid));

	return self;
}
static void monitors_applet_constructed(GObject *obj)
{
	G_OBJECT_CLASS(monitors_applet_parent_class)->constructed(obj);
	MonitorsApplet *self = VALA_PANEL_MONITORS_APPLET(obj);
	GSettings *settings  = vala_panel_applet_get_settings(VALA_PANEL_APPLET(self));
	GActionMap *map = G_ACTION_MAP(vala_panel_applet_get_action_group(VALA_PANEL_APPLET(self)));
	g_simple_action_set_enabled(
	    G_SIMPLE_ACTION(g_action_map_lookup_action(map, VALA_PANEL_APPLET_ACTION_CONFIGURE)),
	    true);
	GtkBox *box                    = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
	self->displayed_mons[CPU_POS]  = g_settings_get_boolean(settings, DISPLAY_CPU);
	self->displayed_mons[RAM_POS]  = g_settings_get_boolean(settings, DISPLAY_RAM);
	self->displayed_mons[SWAP_POS] = g_settings_get_boolean(settings, DISPLAY_SWAP);
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(box));
	gtk_widget_show(GTK_WIDGET(box));
	for (int i = 0; i < N_POS; i++)
		rebuild_mon(self, i);
	self->timer = g_timeout_add_seconds(UPDATE_PERIOD, (GSourceFunc)monitors_update, self);
	g_signal_connect(settings, "changed", G_CALLBACK(on_settings_changed), self);
	gtk_widget_show(GTK_WIDGET(self));
}

static GtkWidget *monitors_get_settings_ui(ValaPanelApplet *base)
{
	return generic_config_widget(vala_panel_applet_get_settings(base),
	                             _("Display CPU usage"),
	                             DISPLAY_CPU,
	                             CONF_BOOL,
	                             _("CPU color"),
	                             CPU_CL,
	                             CONF_STR,
	                             _("CPU width"),
	                             CPU_WIDTH,
	                             CONF_INT,
	                             _("Display RAM usage"),
	                             DISPLAY_RAM,
	                             CONF_BOOL,
	                             _("RAM color"),
	                             RAM_CL,
	                             CONF_STR,
	                             _("RAM width"),
	                             RAM_WIDTH,
	                             CONF_INT,
	                             _("Display swap usage"),
	                             DISPLAY_SWAP,
	                             CONF_BOOL,
	                             _("Swap color"),
	                             SWAP_CL,
	                             CONF_STR,
	                             _("Swap width"),
	                             SWAP_WIDTH,
	                             CONF_INT,
	                             _("Action when clicked"),
	                             ACTION,
	                             CONF_STR,
	                             NULL);
}

/* Plugin destructor. */
static void monitors_applet_dispose(GObject *user_data)
{
	MonitorsApplet *c = VALA_PANEL_MONITORS_APPLET(user_data);
	/* Disconnect the timer. */
	if (c->timer)
	{
		g_source_remove(c->timer);
		c->timer = 0;
	}
	/* Freeing all monitors */
	for (int i = 0; i < N_POS; i++)
	{
		if (c->monitors[i])
			g_clear_pointer(&c->monitors[i], monitor_dispose);
	}

	G_OBJECT_CLASS(monitors_applet_parent_class)->dispose(user_data);
}

static void monitors_applet_init(G_GNUC_UNUSED MonitorsApplet *self)
{
}

static void monitors_applet_class_init(MonitorsAppletClass *klass)
{
	G_OBJECT_CLASS(klass)->constructed              = monitors_applet_constructed;
	G_OBJECT_CLASS(klass)->dispose                  = monitors_applet_dispose;
	VALA_PANEL_APPLET_CLASS(klass)->get_settings_ui = monitors_get_settings_ui;
}

static void monitors_applet_class_finalize(G_GNUC_UNUSED MonitorsAppletClass *klass)
{
}

/*
 * IO Module functions
 */

void g_io_monitors_load(GTypeModule *module)
{
	g_return_if_fail(module != NULL);

	monitors_applet_register_type(module);

	g_type_module_use(module);
	g_io_extension_point_implement(VALA_PANEL_APPLET_EXTENSION_POINT,
	                               monitors_applet_get_type(),
	                               "org.valapanel.monitors",
	                               10);
}

void g_io_monitors_unload(GIOModule *module)
{
	g_return_if_fail(module != NULL);
}
