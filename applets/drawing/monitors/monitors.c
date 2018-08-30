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

#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <stdio.h>

#include "monitors.h"

#define DEFAULT_WIDTH 40 /* Pixels               */
#define BORDER_SIZE 2    /* Pixels               */
#define UPDATE_PERIOD 1  /* Seconds              */
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

struct mon;

typedef bool (*update_func)(struct mon *);
typedef void (*tooltip_update_func)(struct mon *);

typedef struct mon
{
	GdkRGBA foreground_color; /* Foreground color for drawing area      */
	GtkDrawingArea *da;       /* Drawing area                           */
	cairo_surface_t *pixmap;  /* Pixmap to be drawn on drawing area     */
	int pixmap_width;         /* Width and size of the buffer           */
	int pixmap_height;        /* Does not include border size           */
	double *stats;            /* Circular buffer of values              */
	double total;             /* Maximum possible value, as in mem_total*/
	int ring_cursor;          /* Cursor for ring/circular buffer        */
	update_func update;
	tooltip_update_func tooltip_update;
} Monitor;

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

static bool monitor_update(Monitor *mon)
{
	if (mon->tooltip_update != NULL && mon->da != NULL)
		mon->tooltip_update(mon);
	return mon->update(mon);
}

static void monitor_redraw_pixmap(Monitor *mon)
{
	cairo_t *cr = cairo_create(mon->pixmap);
	cairo_set_line_width(cr, 1.0);
	/* Erase pixmap */
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	gdk_cairo_set_source_rgba(cr, &mon->foreground_color);
	for (int i = 0; i < mon->pixmap_width; i++)
	{
		uint drawing_cursor = (mon->ring_cursor + i) % mon->pixmap_width;
		/* Draw one bar of the graph */
		cairo_move_to(cr, i + 0.5, mon->pixmap_height);
		cairo_line_to(cr, i + 0.5, (1.0 - mon->stats[drawing_cursor]) * mon->pixmap_height);
		cairo_stroke(cr);
	}
	cairo_status(cr);
	cairo_destroy(cr);
	/* Redraw pixmap */
	gtk_widget_queue_draw(GTK_WIDGET(mon->da));
}

static bool configure_event(GtkWidget *widget, GdkEventConfigure *dummy, gpointer data)
{
	Monitor *mon = (Monitor *)data;
	GtkAllocation allocation;
	int new_pixmap_width, new_pixmap_height;
	gtk_widget_get_allocation(GTK_WIDGET(mon->da), &allocation);
	new_pixmap_width  = allocation.width - BORDER_SIZE * 2;
	new_pixmap_height = allocation.height - BORDER_SIZE * 2;
	if (new_pixmap_width > 0 && new_pixmap_height > 0)
	{
		/*
		 * If the stats buffer does not exist (first time we get inside this
		 * function) or its size changed, reallocate the buffer and preserve
		 * existing data.
		 */
		if (mon->stats == NULL || (new_pixmap_width != mon->pixmap_width))
		{
			double *new_stats = (double *)g_malloc0(sizeof(double) * new_pixmap_width);
			if (new_stats == NULL)
				return G_SOURCE_REMOVE;

			if (mon->stats != NULL)
			{
				/* New allocation is larger.
				 * Add new "oldest" samples of zero following the cursor*/
				if (new_pixmap_width > mon->pixmap_width)
				{
					/* Number of values between the ring cursor and the end of
					 * the buffer */
					int nvalues = mon->pixmap_width - mon->ring_cursor;
					memcpy(new_stats,
					       mon->stats,
					       mon->ring_cursor * sizeof(double));
					memcpy(&new_stats[nvalues],
					       &mon->stats[mon->ring_cursor],
					       nvalues * sizeof(double));
				}
				/* New allocation is smaller, but still larger than the ring
				 * buffer cursor */
				else if (mon->ring_cursor <= new_pixmap_width)
				{
					/* Numver of values that can be stored between the end of
					 * the new buffer and the ring cursor */
					int nvalues = new_pixmap_width - mon->ring_cursor;
					memcpy(new_stats,
					       mon->stats,
					       mon->ring_cursor * sizeof(double));
					memcpy(&new_stats[mon->ring_cursor],
					       &mon->stats[mon->pixmap_width - nvalues],
					       nvalues * sizeof(double));
				}
				/* New allocation is smaller, and also smaller than the ring
				 * buffer cursor.  Discard all oldest samples following the ring
				 * buffer cursor and additional samples at the beginning of the
				 * buffer. */
				else
				{
					memcpy((void *)new_stats,
					       (void *)&mon
					           ->stats[mon->ring_cursor - new_pixmap_width],
					       new_pixmap_width * sizeof(double));
				}
			}
			g_clear_pointer(&mon->stats, g_free);
			mon->stats = new_stats;
		}

		mon->pixmap_width  = new_pixmap_width;
		mon->pixmap_height = new_pixmap_height;
		g_clear_pointer(&mon->pixmap, cairo_surface_destroy);
		mon->pixmap = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		                                         mon->pixmap_width,
		                                         mon->pixmap_height);
		cairo_surface_status(mon->pixmap);
		monitor_redraw_pixmap(mon);
	}
	return G_SOURCE_CONTINUE;
}

static bool draw(GtkWidget *widget, cairo_t *cr, Monitor *mon)
{
	/* Draw the requested part of the pixmap onto the drawing area.
	 * Translate it in both x and y by the border size. */
	if (mon->pixmap != NULL)
	{
		cairo_set_source_surface(cr, mon->pixmap, BORDER_SIZE, BORDER_SIZE);
		cairo_paint(cr);
		cairo_status(cr);
	}
	return false;
}

static bool button_release_event(GtkWidget *widget, GdkEventButton *evt, MonitorsApplet *applet)
{
	g_autoptr(GVariant) var =
	    g_settings_get_value(vala_panel_applet_get_settings(VALA_PANEL_APPLET(applet)), ACTION);
	if (evt->button == 1)
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
	mon->da = GTK_DRAWING_AREA(gtk_drawing_area_new());

	int height;
	g_object_get(vala_panel_applet_get_toplevel(VALA_PANEL_APPLET(pl)),
	             VP_KEY_HEIGHT,
	             &height,
	             NULL);
	gtk_widget_set_size_request(GTK_WIDGET(mon->da), DEFAULT_WIDTH, height);
	gtk_widget_add_events(GTK_WIDGET(mon->da),
	                      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
	                          GDK_BUTTON_MOTION_MASK);
	gdk_rgba_parse(&mon->foreground_color, color);
	g_signal_connect(mon->da, "configure-event", G_CALLBACK(configure_event), mon);
	g_signal_connect(mon->da, "draw", G_CALLBACK(draw), mon);
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

static void monitor_dispose(Monitor *mon)
{
	gtk_widget_destroy(GTK_WIDGET(mon->da));
	g_clear_pointer(&mon->pixmap, cairo_surface_destroy);
	g_clear_pointer(&mon->stats, g_free);
	g_clear_pointer(&mon, g_free);
}

/*
 * CPU monitor functions
 */

typedef struct
{                                          /* Value from /proc/stat */
	long long unsigned int u, n, s, i; /* User, nice, system, idle */
} cpu_stat;

static bool cpu_update(Monitor *c)
{
	static cpu_stat previous_cpu_stat = { 0, 0, 0, 0 };

	if ((c->stats != NULL) && (c->pixmap != NULL))
	{
		/* Open statistics file and scan out CPU usage. */
		cpu_stat cpu;
		FILE *stat = fopen("/proc/stat", "r");
		if (stat == NULL)
			return true;
		int fscanf_result =
		    fscanf(stat, "cpu %llu %llu %llu %llu", &cpu.u, &cpu.n, &cpu.s, &cpu.i);
		fclose(stat);

		/* Ensure that fscanf succeeded. */
		if (fscanf_result == 4)
		{
			/* Compute delta from previous statistics. */
			cpu_stat cpu_delta;
			cpu_delta.u = cpu.u - previous_cpu_stat.u;
			cpu_delta.n = cpu.n - previous_cpu_stat.n;
			cpu_delta.s = cpu.s - previous_cpu_stat.s;
			cpu_delta.i = cpu.i - previous_cpu_stat.i;

			/* Copy current to previous. */
			memcpy(&previous_cpu_stat, &cpu, sizeof(cpu_stat));

			/* Compute user+nice+system as a fraction of total.
			 * Introduce this sample to ring buffer, increment and wrap ring buffer
			 * cursor. */
			float cpu_uns            = cpu_delta.u + cpu_delta.n + cpu_delta.s;
			c->stats[c->ring_cursor] = cpu_uns / (cpu_uns + cpu_delta.i);
			c->ring_cursor += 1;
			if (c->ring_cursor >= c->pixmap_width)
				c->ring_cursor = 0;

			/* Redraw with the new sample. */
			monitor_redraw_pixmap(c);
		}
	}
	return G_SOURCE_CONTINUE;
}

static void tooltip_update_cpu(Monitor *m)
{
	if (m != NULL && m->stats != NULL)
	{
		int ring_pos = (m->ring_cursor == 0) ? m->pixmap_width - 1 : m->ring_cursor - 1;
		if (m->da != NULL)
		{
			g_autofree char *tooltip_txt =
			    g_strdup_printf(_("CPU usage: %.2f%%"), m->stats[ring_pos] * 100);
			gtk_widget_set_tooltip_text(GTK_WIDGET(m->da), tooltip_txt);
		}
	}
}

/*
 * Memory monitor functions
 */
static bool update_mem(Monitor *m)
{
	char buf[80];
	long mem_total   = 0;
	long mem_free    = 0;
	long mem_buffers = 0;
	long mem_cached  = 0;
	uint readmask    = 0x8 | 0x4 | 0x2 | 0x1;
	if (m->stats == NULL || m->pixmap == NULL)
		return true;
	FILE *meminfo = fopen("/proc/meminfo", "r");
	if (meminfo == NULL)
	{
		g_warning("monitors: Could not open /proc/meminfo: %d, %s", errno, strerror(errno));
		return false;
	}

	while (readmask != 0 && fgets(buf, 80, meminfo) != NULL)
	{
		if (sscanf(buf, "MemTotal: %ld kB\n", &mem_total) == 1)
		{
			readmask ^= 0x1;
			continue;
		}
		if (sscanf(buf, "MemFree: %ld kB\n", &mem_free) == 1)
		{
			readmask ^= 0x2;
			continue;
		}
		if (sscanf(buf, "Buffers: %ld kB\n", &mem_buffers) == 1)
		{
			readmask ^= 0x4;
			continue;
		}
		if (sscanf(buf, "Cached: %ld kB\n", &mem_cached) == 1)
		{
			readmask ^= 0x8;
			continue;
		}
	}
	fclose(meminfo);

	if (readmask != 0)
	{
		g_warning("monitors: Could not read all values from /proc/meminfo:\n readmask %x",
		          readmask);
		return false;
	}

	m->total = mem_total;
	/* Adding stats to the buffer:
	 * It is debatable if 'mem_buffers' counts as free or not. I'll go with
	 * 'free', because it can be flushed fairly quickly, and generally
	 * isn't necessary to keep in memory.
	 * It is hard to draw the line, which caches should be counted as free,
	 * and which not. Pagecaches, dentry, and inode caches are quickly
	 * filled up again for almost any use case. Hence I would not count
	 * them as 'free'.
	 * 'mem_cached' definitely counts as 'free' because it is immediately
	 * released should any application need it. */
	m->stats[m->ring_cursor] =
	    (mem_total - mem_buffers - mem_free - mem_cached) / (double)mem_total;

	m->ring_cursor += 1;
	if (m->ring_cursor >= m->pixmap_width)
		m->ring_cursor = 0;
	/* Redraw the pixmap, with the new sample */
	monitor_redraw_pixmap(m);
	return true;
}
static void tooltip_update_mem(Monitor *m)
{
	if (m != NULL && m->stats != NULL)
	{
		int ring_pos = (m->ring_cursor == 0) ? m->pixmap_width - 1 : m->ring_cursor - 1;
		if (m->da != NULL)
		{
			g_autofree char *tooltip_txt =
			    g_strdup_printf(_("RAM usage: %.1fMB (%.2f%%)"),
			                    m->stats[ring_pos] * m->total / 1024,
			                    m->stats[ring_pos] * 100);
			gtk_widget_set_tooltip_text(GTK_WIDGET(m->da), tooltip_txt);
		}
	}
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
			self->monitors[i]->update(self->monitors[i]);
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
