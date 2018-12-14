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
#include "monitor.h"
#include "net.h"

#define DEFAULT_WIDTH 40 /* Pixels               */
#define UPDATE_PERIOD 1  /* Seconds */

#define ACTION "click-action"

enum
{
	NET_TX_POS = 0,
	NET_RX_POS,
	N_POS
};

struct _NetMonApplet
{
	ValaPanelApplet _parent_;
	NetMon *monitors[N_POS];
	bool displayed_mons[N_POS];
	uint timer;
};

G_DEFINE_DYNAMIC_TYPE(NetMonApplet, netmon_applet, vala_panel_applet_get_type())

/*
 * Generic monitor functions and events
 */

static bool button_release_event(GtkWidget *widget, GdkEventButton *evt, NetMonApplet *applet)
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

static void monitor_setup_size(NetMon *mon, NetMonApplet *pl, int width)
{
	int height;
	g_object_get(vala_panel_applet_get_toplevel(VALA_PANEL_APPLET(pl)),
	             VP_KEY_HEIGHT,
	             &height,
	             NULL);
	gtk_widget_set_size_request(GTK_WIDGET(mon->da), width, height);
	netmon_resize(GTK_WIDGET(mon->da), mon);
}

static void monitor_init(NetMon *mon, NetMonApplet *pl, const char *color, int width)
{
	netmon_init_no_height(mon, color);
	monitor_setup_size(mon, pl, width);
	g_signal_connect(mon->da, "button-release-event", G_CALLBACK(button_release_event), pl);
}

static NetMon *monitor_create(GtkBox *monitor_box, NetMonApplet *pl, update_func update,
                              tooltip_update_func tooltip_update, const char *color, int width)
{
	NetMon *m = g_new0(NetMon, 1);
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

static NetMon *create_monitor_with_pos(NetMonApplet *self, int pos)
{
	GSettings *settings = vala_panel_applet_get_settings(VALA_PANEL_APPLET(self));
	if (pos == NET_TX_POS)
	{
		g_autofree char *color = g_settings_get_string(settings, NET_TX_CL);
		int width              = g_settings_get_int(settings, NET_TX_WIDTH);
		return monitor_create(GTK_BOX(gtk_bin_get_child(GTK_BIN(self))),
		                      self,
		                      update_net_tx,
		                      tooltip_update_net_tx,
		                      color,
		                      width);
	}
	if (pos == NET_RX_POS)
	{
		g_autofree char *color = g_settings_get_string(settings, NET_RX_CL);
		int width              = g_settings_get_int(settings, NET_RX_WIDTH);
		return monitor_create(GTK_BOX(gtk_bin_get_child(GTK_BIN(self))),
		                      self,
		                      update_net_rx,
		                      tooltip_update_net_rx,
		                      color,
		                      width);
	}
	return NULL;
}

static bool monitors_update(void *data)
{
	NetMonApplet *self = VALA_PANEL_NETMON_APPLET(data);
	if (g_source_is_destroyed(g_main_current_source()))
		return false;
	for (int i = 0; i < N_POS; i++)
	{
		if (self->monitors[i] != NULL)
			netmon_update(self->monitors[i]);
	}
	return true;
}

static void rebuild_mon(NetMonApplet *self, int i)
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
		g_clear_pointer(&self->monitors[i], netmon_dispose);
	}
}

void on_settings_changed(GSettings *settings, char *key, gpointer user_data)
{
	NetMonApplet *self = VALA_PANEL_NETMON_APPLET(user_data);
	if (!g_strcmp0(key, DISPLAY_NET))
	{
		self->displayed_mons[NET_TX_POS] = g_settings_get_boolean(settings, DISPLAY_NET);
		self->displayed_mons[NET_RX_POS] = g_settings_get_boolean(settings, DISPLAY_NET);
		rebuild_mon(self, NET_TX_POS);
		rebuild_mon(self, NET_RX_POS);
	}
	//	else if ((!g_strcmp0(key, NET_RX_CL)) && self->monitors[NET_RX_POS] != NULL)
	//	{
	//		g_autofree char *color = g_settings_get_string(settings, NET_RX_CL);
	//		gdk_rgba_parse(&self->monitors[NET_RX_POS]->foreground_color, color);
	//	}
	//	else if ((!g_strcmp0(key, NET_TX_CL)) && self->monitors[NET_TX_POS] != NULL)
	//	{
	//		g_autofree char *color = g_settings_get_string(settings, NET_TX_CL);
	//		gdk_rgba_parse(&self->monitors[NET_TX_POS]->foreground_color, color);
	//	}
	else if ((!g_strcmp0(key, NET_RX_WIDTH)) && self->monitors[NET_RX_POS] != NULL)
	{
		int width = g_settings_get_int(settings, NET_RX_WIDTH);
		monitor_setup_size(self->monitors[NET_RX_POS], self, width);
	}
	else if ((!g_strcmp0(key, NET_TX_WIDTH)) && self->monitors[NET_TX_POS] != NULL)
	{
		int width = g_settings_get_int(settings, NET_TX_WIDTH);
		monitor_setup_size(self->monitors[NET_TX_POS], self, width);
	}
}

/* Applet widget constructor. */
NetMonApplet *netmon_applet_new(ValaPanelToplevel *toplevel, GSettings *settings, const char *uuid)
{
	/* Allocate applet context*/
	NetMonApplet *self = VALA_PANEL_NETMON_APPLET(
	    vala_panel_applet_construct(netmon_applet_get_type(), toplevel, settings, uuid));
	GActionMap *map = G_ACTION_MAP(vala_panel_applet_get_action_group(VALA_PANEL_APPLET(self)));
	g_simple_action_set_enabled(
	    G_SIMPLE_ACTION(g_action_map_lookup_action(map, VALA_PANEL_APPLET_ACTION_CONFIGURE)),
	    true);
	GtkBox *box                      = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
	self->displayed_mons[NET_RX_POS] = g_settings_get_boolean(settings, DISPLAY_NET);
	self->displayed_mons[NET_TX_POS] = g_settings_get_boolean(settings, DISPLAY_NET);
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(box));
	gtk_widget_show(GTK_WIDGET(box));
	for (int i = 0; i < N_POS; i++)
		rebuild_mon(self, i);
	self->timer = g_timeout_add_seconds(UPDATE_PERIOD, (GSourceFunc)monitors_update, self);
	g_signal_connect(settings, "changed", G_CALLBACK(on_settings_changed), self);
	gtk_widget_show(GTK_WIDGET(self));
	return self;
}

static GtkWidget *netmon_get_settings_ui(ValaPanelApplet *base)
{
	return generic_config_widget(vala_panel_applet_get_settings(base),
	                             _("Display network usage"),
	                             DISPLAY_NET,
	                             CONF_BOOL,
	                             _("Net receive color"),
	                             NET_RX_CL,
	                             CONF_STR,
	                             _("Net receive width"),
	                             NET_RX_WIDTH,
	                             CONF_INT,
	                             _("Net transmit color"),
	                             NET_TX_CL,
	                             CONF_STR,
	                             _("Net receive width"),
	                             NET_TX_WIDTH,
	                             CONF_INT,
	                             _("Action when clicked"),
	                             ACTION,
	                             CONF_STR,
	                             NULL);
}

/* Plugin destructor. */
static void netmon_applet_dispose(GObject *user_data)
{
	NetMonApplet *c = VALA_PANEL_NETMON_APPLET(user_data);
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
			g_clear_pointer(&c->monitors[i], netmon_dispose);
	}

	G_OBJECT_CLASS(netmon_applet_parent_class)->dispose(user_data);
}

static void netmon_applet_init(NetMonApplet *self)
{
}

static void netmon_applet_class_init(NetMonAppletClass *klass)
{
	G_OBJECT_CLASS(klass)->dispose                  = netmon_applet_dispose;
	VALA_PANEL_APPLET_CLASS(klass)->get_settings_ui = netmon_get_settings_ui;
}

static void netmon_applet_class_finalize(NetMonAppletClass *klass)
{
}

/*
 * Plugin functions
 */

struct _NetMonPlugin
{
	ValaPanelAppletPlugin parent;
};

G_DEFINE_DYNAMIC_TYPE(NetMonPlugin, netmon_plugin, vala_panel_applet_plugin_get_type())

static ValaPanelApplet *netmon_plugin_get_applet_widget(ValaPanelAppletPlugin *base,
                                                        ValaPanelToplevel *toplevel,
                                                        GSettings *settings, const char *uuid)
{
	g_return_val_if_fail(toplevel != NULL, NULL);
	g_return_val_if_fail(uuid != NULL, NULL);

	return VALA_PANEL_APPLET(netmon_applet_new(toplevel, settings, uuid));
}

NetMonApplet *netmon_plugin_new(GType object_type)
{
	return VALA_PANEL_NETMON_APPLET(
	    vala_panel_applet_plugin_construct(netmon_applet_get_type()));
}

static void netmon_plugin_class_init(NetMonPluginClass *klass)
{
	((ValaPanelAppletPluginClass *)klass)->get_applet_widget = netmon_plugin_get_applet_widget;
}

static void netmon_plugin_init(NetMonPlugin *self)
{
}

static void netmon_plugin_class_finalize(NetMonPluginClass *klass)
{
}

/*
 * IO Module functions
 */

void g_io_netmon_load(GTypeModule *module)
{
	g_return_if_fail(module != NULL);

	netmon_applet_register_type(module);
	netmon_plugin_register_type(module);

	g_type_module_use(module);
	g_io_extension_point_implement(VALA_PANEL_APPLET_EXTENSION_POINT,
	                               netmon_plugin_get_type(),
	                               "org.valapanel.netmon",
	                               10);
}

void g_io_netmon_unload(GIOModule *module)
{
	g_return_if_fail(module != NULL);
}
