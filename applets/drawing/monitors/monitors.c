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

#define DEFAULT_WIDTH 40 /* Pixels               */
#define UPDATE_PERIOD 1  /* Seconds */
#define DISPLAY_CPU "display-cpu-monitor"
#define DISPLAY_RAM "display-ram-monitor"
#define ACTION "click-action"
#define CPU_CL "cpu-color"
#define RAM_CL "ram-color"

enum
{
	CPU_POS = 0,
	RAM_POS,
	N_POS
};

struct _MonitorsApplet
{
	ValaPanelApplet _parent_;
	Monitor *monitors[2];
	bool displayed_mons[2];
	uint timer;
};

#define monitors_applet_from_da(da) VALA_PANEL_CPU_APPLET(gtk_widget_get_parent(GTK_WIDGET(da)))
#define monitors_applet_get_da(p) GTK_DRAWING_AREA(gtk_bin_get_child(GTK_WIDGET(p)))

G_DEFINE_DYNAMIC_TYPE(MonitorsApplet, monitors_applet, vala_panel_applet_get_type())

/*
 * Generic monitor functions and events
 */

static bool button_release_event(GtkWidget *widget, GdkEventButton *evt, MonitorsApplet *applet)
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

static void monitor_init(Monitor *mon, MonitorsApplet *pl, const char *color)
{
	monitor_init_no_height(mon, color);
	int height;
	g_object_get(vala_panel_applet_get_toplevel(VALA_PANEL_APPLET(pl)),
	             VP_KEY_HEIGHT,
	             &height,
	             NULL);
	gtk_widget_set_size_request(GTK_WIDGET(mon->da), DEFAULT_WIDTH, height);
	g_signal_connect(mon->da, "button-release-event", G_CALLBACK(button_release_event), pl);
}

static Monitor *monitor_create(GtkBox *monitor_box, MonitorsApplet *pl, update_func update,
                               tooltip_update_func tooltip_update, const char *color)
{
	Monitor *m = g_new0(Monitor, 1);
	monitor_init(m, pl, color);
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
	if (pos == CPU_POS)
	{
		g_autofree char *color =
		    g_settings_get_string(vala_panel_applet_get_settings(VALA_PANEL_APPLET(self)),
		                          CPU_CL);
		return monitor_create(GTK_BOX(gtk_bin_get_child(GTK_BIN(self))),
		                      self,
		                      cpu_update,
		                      tooltip_update_cpu,
		                      color);
	}
	if (pos == RAM_POS)
	{
		g_autofree char *color =
		    g_settings_get_string(vala_panel_applet_get_settings(VALA_PANEL_APPLET(self)),
		                          RAM_CL);
		return monitor_create(GTK_BOX(gtk_bin_get_child(GTK_BIN(self))),
		                      self,
		                      update_mem,
		                      tooltip_update_mem,
		                      color);
	}
	return NULL;
}

static bool monitors_update(void *data)
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

static void rebuild_mons(MonitorsApplet *self)
{
	for (int i = 0; i < N_POS; i++)
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
			monitor_dispose(self->monitors[i]);
		}
	}
}

void on_settings_changed(GSettings *settings, char *key, gpointer user_data)
{
	MonitorsApplet *self = VALA_PANEL_MONITORS_APPLET(user_data);
	if (!g_strcmp0(key, DISPLAY_CPU))
	{
		self->displayed_mons[CPU_POS] = g_settings_get_boolean(settings, DISPLAY_CPU);
		rebuild_mons(self);
	}
	else if (!g_strcmp0(key, DISPLAY_RAM))
	{
		self->displayed_mons[RAM_POS] = g_settings_get_boolean(settings, DISPLAY_RAM);
		rebuild_mons(self);
	}
	else if ((!g_strcmp0(key, CPU_CL)) && self->monitors[CPU_POS] != NULL)
	{
		g_autofree char *color = g_settings_get_string(settings, CPU_CL);
		gdk_rgba_parse(&self->monitors[CPU_POS]->foreground_color, color);
	}
	else if ((!g_strcmp0(key, RAM_CL)) && self->monitors[RAM_POS] != NULL)
	{
		g_autofree char *color = g_settings_get_string(settings, RAM_CL);
		gdk_rgba_parse(&self->monitors[RAM_POS]->foreground_color, color);
	}
}

/* Applet widget constructor. */
MonitorsApplet *monitors_applet_new(ValaPanelToplevel *toplevel, GSettings *settings,
                                    const char *uuid)
{
	/* Allocate applet context*/
	MonitorsApplet *self = VALA_PANEL_MONITORS_APPLET(
	    vala_panel_applet_construct(monitors_applet_get_type(), toplevel, settings, uuid));
	GActionMap *map = G_ACTION_MAP(vala_panel_applet_get_action_group(VALA_PANEL_APPLET(self)));
	g_simple_action_set_enabled(
	    G_SIMPLE_ACTION(g_action_map_lookup_action(map, VALA_PANEL_APPLET_ACTION_CONFIGURE)),
	    true);
	GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
	gtk_box_set_homogeneous(box, true);
	self->displayed_mons[CPU_POS] = g_settings_get_boolean(settings, DISPLAY_CPU);
	self->displayed_mons[RAM_POS] = g_settings_get_boolean(settings, DISPLAY_RAM);
	if (self->displayed_mons[CPU_POS])
	{
		g_autofree char *color = g_settings_get_string(settings, CPU_CL);
		self->monitors[CPU_POS] =
		    monitor_create(box, self, cpu_update, tooltip_update_cpu, color);
	}
	if (self->displayed_mons[RAM_POS])
	{
		g_autofree char *color = g_settings_get_string(settings, RAM_CL);
		self->monitors[RAM_POS] =
		    monitor_create(box, self, update_mem, tooltip_update_mem, color);
	}
	self->timer = g_timeout_add_seconds(UPDATE_PERIOD, (GSourceFunc)monitors_update, self);
	g_signal_connect(settings, "changed", G_CALLBACK(on_settings_changed), self);
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(box));
	gtk_widget_show(GTK_WIDGET(box));
	gtk_widget_show(GTK_WIDGET(self));
	return self;
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
	                             _("Display RAM usage"),
	                             DISPLAY_RAM,
	                             CONF_BOOL,
	                             _("RAM color"),
	                             RAM_CL,
	                             CONF_STR,
	                             _("Action when clicked"),
	                             ACTION,
	                             CONF_STR);
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

static void monitors_applet_init(MonitorsApplet *self)
{
}

static void monitors_applet_class_init(MonitorsAppletClass *klass)
{
	G_OBJECT_CLASS(klass)->dispose                  = monitors_applet_dispose;
	VALA_PANEL_APPLET_CLASS(klass)->get_settings_ui = monitors_get_settings_ui;
}

static void monitors_applet_class_finalize(MonitorsAppletClass *klass)
{
}

/*
 * Plugin functions
 */

struct _MonitorsPlugin
{
	ValaPanelAppletPlugin parent;
};

G_DEFINE_DYNAMIC_TYPE(MonitorsPlugin, monitors_plugin, vala_panel_applet_plugin_get_type())

static ValaPanelApplet *monitors_plugin_get_applet_widget(ValaPanelAppletPlugin *base,
                                                          ValaPanelToplevel *toplevel,
                                                          GSettings *settings, const char *uuid)
{
	g_return_val_if_fail(toplevel != NULL, NULL);
	g_return_val_if_fail(uuid != NULL, NULL);

	return VALA_PANEL_APPLET(monitors_applet_new(toplevel, settings, uuid));
}

MonitorsApplet *monitors_plugin_new(GType object_type)
{
	return VALA_PANEL_MONITORS_APPLET(
	    vala_panel_applet_plugin_construct(monitors_applet_get_type()));
}

static void monitors_plugin_class_init(MonitorsPluginClass *klass)
{
	((ValaPanelAppletPluginClass *)klass)->get_applet_widget =
	    monitors_plugin_get_applet_widget;
}

static void monitors_plugin_init(MonitorsPlugin *self)
{
}

static void monitors_plugin_class_finalize(MonitorsPluginClass *klass)
{
}

/*
 * IO Module functions
 */

void g_io_monitors_load(GTypeModule *module)
{
	g_return_if_fail(module != NULL);

	monitors_applet_register_type(module);
	monitors_plugin_register_type(module);

	g_type_module_use(module);
	g_io_extension_point_implement(VALA_PANEL_APPLET_EXTENSION_POINT,
	                               monitors_plugin_get_type(),
	                               "org.valapanel.monitors",
	                               10);
}

void g_io_monitors_unload(GIOModule *module)
{
	g_return_if_fail(module != NULL);
}
