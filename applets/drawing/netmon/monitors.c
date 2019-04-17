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
	NET_RX_POS,
	N_POS
};

struct _NetMonApplet
{
	ValaPanelApplet _parent_;
	NetMon *monitor;
	uint timer;
};

G_DEFINE_DYNAMIC_TYPE(NetMonApplet, netmon_applet, vala_panel_applet_get_type())

/*
 * Generic monitor functions and events
 */

static bool button_release_event(G_GNUC_UNUSED GtkWidget *widget, GdkEventButton *evt,
                                 NetMonApplet *applet)
{
	ValaPanelApplet *base = VALA_PANEL_APPLET(applet);
	GtkWindow *top        = GTK_WINDOW(vala_panel_applet_get_toplevel(base));
	g_autoptr(GVariant) var =
	    g_settings_get_value(vala_panel_applet_get_settings(base), ACTION);
	if (evt->button == 1 &&
	    g_variant_type_is_subtype_of(g_variant_get_type(var), G_VARIANT_TYPE_STRING))
	{
		activate_menu_launch_command(NULL, var, gtk_window_get_application(top));
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

static void monitor_init(NetMon *mon, NetMonApplet *pl, const char *rx_color, const char *tx_color,
                         int width)
{
	netmon_init_no_height(mon, rx_color, tx_color);
	monitor_setup_size(mon, pl, width);
	g_signal_connect(mon->da, "button-release-event", G_CALLBACK(button_release_event), pl);
}

static NetMon *monitor_create(GtkBox *monitor_box, NetMonApplet *pl, update_func update,
                              tooltip_update_func tooltip_update, const char *interface_name,
                              const char *rx_color, const char *tx_color, int width,
                              int average_samples, bool use_bar)
{
	NetMon *m = g_new0(NetMon, 1);
	monitor_init(m, pl, rx_color, tx_color, width);
	m->interface_name  = (char *)interface_name;
	m->average_samples = average_samples;
	m->use_bar         = use_bar;
	m->update          = update;
	m->tooltip_update  = tooltip_update;
	gtk_box_pack_start(GTK_BOX(monitor_box), GTK_WIDGET(m->da), false, false, 0);
	gtk_widget_show(GTK_WIDGET(m->da));
	return m;
}

/*
 * Applet functions
 */

static NetMon *create_monitor(NetMonApplet *self)
{
	GSettings *settings       = vala_panel_applet_get_settings(VALA_PANEL_APPLET(self));
	g_autofree char *rx_color = g_settings_get_string(settings, NET_RX_CL);
	g_autofree char *tx_color = g_settings_get_string(settings, NET_TX_CL);
	char *interface_name      = g_settings_get_string(settings, NET_IFACE);
	int width                 = g_settings_get_int(settings, NET_WIDTH);
	int average_samples       = g_settings_get_int(settings, NET_AVERAGE_SAMPLES);
	bool use_bar              = g_settings_get_boolean(settings, NET_USE_BAR);
	return monitor_create(GTK_BOX(gtk_bin_get_child(GTK_BIN(self))),
	                      self,
	                      update_net,
	                      tooltip_update_net,
	                      interface_name,
	                      rx_color,
	                      tx_color,
	                      width,
	                      average_samples,
	                      use_bar);
}

static int monitors_update(void *data)
{
	NetMonApplet *self = VALA_PANEL_NETMON_APPLET(data);
	if (g_source_is_destroyed(g_main_current_source()))
		return false;
	netmon_update(self->monitor);
	return true;
}

static void rebuild_mon(NetMonApplet *self)
{
	g_clear_pointer(&self->monitor, netmon_dispose);
	self->monitor = create_monitor(self);
}

void on_settings_changed(GSettings *settings, char *key, gpointer user_data)
{
	NetMonApplet *self = VALA_PANEL_NETMON_APPLET(user_data);
	if (!g_strcmp0(key, NET_IFACE))
	{
		rebuild_mon(self);
	}
	else if (!g_strcmp0(key, NET_RX_CL))
	{
		g_autofree char *color = g_settings_get_string(settings, NET_RX_CL);
		gdk_rgba_parse(&self->monitor->rx_color, color);
	}
	else if (!g_strcmp0(key, NET_TX_CL))
	{
		g_autofree char *color = g_settings_get_string(settings, NET_TX_CL);
		gdk_rgba_parse(&self->monitor->tx_color, color);
	}
	else if (!g_strcmp0(key, NET_WIDTH))
	{
		int width = g_settings_get_int(settings, NET_WIDTH);
		monitor_setup_size(self->monitor, self, width);
	}
	else if (!g_strcmp0(key, NET_AVERAGE_SAMPLES))
	{
		int width                      = g_settings_get_int(settings, NET_AVERAGE_SAMPLES);
		self->monitor->average_samples = width;
	}
	else if (!g_strcmp0(key, NET_USE_BAR))
	{
		bool use_bar           = g_settings_get_boolean(settings, NET_USE_BAR);
		self->monitor->use_bar = use_bar;
	}
}

/* Applet widget constructor. */
NetMonApplet *netmon_applet_new(ValaPanelToplevel *toplevel, GSettings *settings, const char *uuid)
{
	/* Allocate applet context*/
	NetMonApplet *self = VALA_PANEL_NETMON_APPLET(
	    vala_panel_applet_construct(netmon_applet_get_type(), toplevel, settings, uuid));
	return self;
}
static void netmon_applet_constructed(GObject *obj)
{
	G_OBJECT_CLASS(netmon_applet_parent_class)->constructed(obj);
	NetMonApplet *self  = VALA_PANEL_NETMON_APPLET(obj);
	GSettings *settings = vala_panel_applet_get_settings(VALA_PANEL_APPLET(self));
	GActionMap *map = G_ACTION_MAP(vala_panel_applet_get_action_group(VALA_PANEL_APPLET(self)));
	g_simple_action_set_enabled(
	    G_SIMPLE_ACTION(g_action_map_lookup_action(map, VALA_PANEL_APPLET_ACTION_CONFIGURE)),
	    true);
	GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(box));
	gtk_widget_show(GTK_WIDGET(box));
	rebuild_mon(self);
	self->timer = g_timeout_add_seconds(UPDATE_PERIOD, (GSourceFunc)monitors_update, self);
	g_signal_connect(settings, "changed", G_CALLBACK(on_settings_changed), self);
	gtk_widget_show(GTK_WIDGET(self));
}

static GtkWidget *netmon_get_settings_ui(ValaPanelApplet *base)
{
	return generic_config_widget(vala_panel_applet_get_settings(base),
	                             _("Network interface"),
	                             NET_IFACE,
	                             CONF_STR,
	                             _("Net average samples count (for more round speed)"),
	                             NET_AVERAGE_SAMPLES,
	                             CONF_INT,
	                             _("Net receive color"),
	                             NET_RX_CL,
	                             CONF_STR,
	                             _("Net transmit color"),
	                             NET_TX_CL,
	                             CONF_STR,
	                             _("Net graph as histogram"),
	                             NET_USE_BAR,
	                             CONF_BOOL,
	                             _("Net width"),
	                             NET_WIDTH,
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
	g_clear_pointer(&c->monitor, netmon_dispose);

	G_OBJECT_CLASS(netmon_applet_parent_class)->dispose(user_data);
}

static void netmon_applet_init(G_GNUC_UNUSED NetMonApplet *self)
{
}

static void netmon_applet_class_init(NetMonAppletClass *klass)
{
	G_OBJECT_CLASS(klass)->dispose                  = netmon_applet_dispose;
	G_OBJECT_CLASS(klass)->constructed              = netmon_applet_constructed;
	VALA_PANEL_APPLET_CLASS(klass)->get_settings_ui = netmon_get_settings_ui;
}

static void netmon_applet_class_finalize(G_GNUC_UNUSED NetMonAppletClass *klass)
{
}

/*
 * IO Module functions
 */

void g_io_netmon_load(GTypeModule *module)
{
	g_return_if_fail(module != NULL);

	netmon_applet_register_type(module);

	g_type_module_use(module);
	g_io_extension_point_implement(VALA_PANEL_APPLET_EXTENSION_POINT,
	                               netmon_applet_get_type(),
	                               "org.valapanel.netmon",
	                               10);
}

void g_io_netmon_unload(GIOModule *module)
{
	g_return_if_fail(module != NULL);
}
