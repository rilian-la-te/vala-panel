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

#include <glib/gi18n.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <time.h>

#include "cpu.h"

#define BORDER_SIZE 2
#define PANEL_HEIGHT_DEFAULT 26 /* from panel defaults */

typedef struct
{                                          /* Value from /proc/stat */
	long long unsigned int u, n, s, i; /* User, nice, system, idle */
} cpu_stat;

/* Private context for CPU applet. */
struct _CpuApplet
{
	ValaPanelApplet parent;
	GdkRGBA foreground_color; /* Foreground color for drawing area */
	cairo_surface_t *pixmap;  /* Pixmap to be drawn on drawing area */

	uint timer;       /* Timer for periodic update */
	float *stats_cpu; /* Ring buffer of CPU utilization values as saved CPU utilization value as
	                       0.0..1.0*/
	uint ring_cursor; /* Cursor for ring buffer */
	uint pixmap_width;  /* Width of drawing area pixmap; also size of ring buffer; does not
	                       include border size */
	uint pixmap_height; /* Height of drawing area pixmap; does not include border size */
	cpu_stat previous_cpu_stat; /* Previous value of cpu_stat */
};

#define cpu_applet_from_da(da) VALA_PANEL_CPU_PLUGIN(gtk_widget_get_parent(GTK_WIDGET(da)))
#define cpu_applet_get_da(p) GTK_DRAWING_AREA(gtk_bin_get_child(GTK_WIDGET(p)))

G_DEFINE_DYNAMIC_TYPE(CpuApplet, cpu_applet, vala_panel_applet_get_type())

/* Redraw after timer callback or resize. */
static void redraw_pixmap(CpuApplet *c)
{
	cairo_t *cr              = cairo_create(c->pixmap);
	GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(c));
	GtkStateFlags flags      = gtk_widget_get_state_flags(GTK_WIDGET(c));
	GdkRGBA background_color;
	gtk_style_context_get(context, flags, "background-color", &background_color, NULL);
	cairo_set_line_width(cr, 1.0);
	/* Erase pixmap. */
	cairo_rectangle(cr, 0, 0, c->pixmap_width, c->pixmap_height);
	gdk_cairo_set_source_rgba(cr, &background_color);
	cairo_fill(cr);

	/* Recompute pixmap. */
	uint i;
	uint drawing_cursor = c->ring_cursor;
	gdk_cairo_set_source_rgba(cr, &c->foreground_color);
	for (i = 0; i < c->pixmap_width; i++)
	{
		/* Draw one bar of the CPU usage graph. */
		if (c->stats_cpu[drawing_cursor] != 0.0)
		{
			cairo_move_to(cr, i + 0.5, c->pixmap_height);
			cairo_line_to(cr,
			              i + 0.5,
			              c->pixmap_height -
			                  c->stats_cpu[drawing_cursor] * c->pixmap_height);
			cairo_stroke(cr);
		}

		/* Increment and wrap drawing cursor. */
		drawing_cursor += 1;
		if (drawing_cursor >= c->pixmap_width)
			drawing_cursor = 0;
	}

	/* check_cairo_status(cr); */
	cairo_destroy(cr);

	/* Redraw pixmap. */
	gtk_widget_queue_draw(cpu_applet_get_da(c));
}

/* Periodic timer callback. */
static bool cpu_update(CpuApplet *c)
{
	if (g_source_is_destroyed(g_main_current_source()))
		return false;
	if ((c->stats_cpu != NULL) && (c->pixmap != NULL))
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
			cpu_delta.u = cpu.u - c->previous_cpu_stat.u;
			cpu_delta.n = cpu.n - c->previous_cpu_stat.n;
			cpu_delta.s = cpu.s - c->previous_cpu_stat.s;
			cpu_delta.i = cpu.i - c->previous_cpu_stat.i;

			/* Copy current to previous. */
			memcpy(&c->previous_cpu_stat, &cpu, sizeof(cpu_stat));

			/* Compute user+nice+system as a fraction of total.
			 * Introduce this sample to ring buffer, increment and wrap ring buffer
			 * cursor. */
			float cpu_uns                = cpu_delta.u + cpu_delta.n + cpu_delta.s;
			c->stats_cpu[c->ring_cursor] = cpu_uns / (cpu_uns + cpu_delta.i);
			c->ring_cursor += 1;
			if (c->ring_cursor >= c->pixmap_width)
				c->ring_cursor = 0;

			/* Redraw with the new sample. */
			redraw_pixmap(c);
		}
	}
	return true;
}

/* Handler for configure_event on drawing area. */
static bool configure_event(GtkWidget *widget, GdkEventConfigure *event, CpuApplet *c)
{
	GtkAllocation allocation;

	gtk_widget_get_allocation(widget, &allocation);
	/* Allocate pixmap and statistics buffer without border pixels. */
	uint new_pixmap_width  = MAX(allocation.width - BORDER_SIZE * 2, 0);
	uint new_pixmap_height = MAX(allocation.height - BORDER_SIZE * 2, 0);
	if ((new_pixmap_width > 0) && (new_pixmap_height > 0))
	{
		/* If statistics buffer does not exist or it changed size, reallocate and preserve
		 * existing data. */
		if ((c->stats_cpu == NULL) || (new_pixmap_width != c->pixmap_width))
		{
			float *new_stats_cpu = g_new0(typeof(*c->stats_cpu), new_pixmap_width);
			if (c->stats_cpu != NULL)
			{
				if (new_pixmap_width > c->pixmap_width)
				{
					/* New allocation is larger.
					 * Introduce new "oldest" samples of zero following the
					 * cursor. */
					memcpy(&new_stats_cpu[0],
					       &c->stats_cpu[0],
					       c->ring_cursor * sizeof(float));
					memcpy(&new_stats_cpu[new_pixmap_width - c->pixmap_width +
					                      c->ring_cursor],
					       &c->stats_cpu[c->ring_cursor],
					       (c->pixmap_width - c->ring_cursor) * sizeof(float));
				}
				else if (c->ring_cursor <= new_pixmap_width)
				{
					/* New allocation is smaller, but still larger than the ring
					 * buffer cursor.
					 * Discard the oldest samples following the cursor. */
					memcpy(&new_stats_cpu[0],
					       &c->stats_cpu[0],
					       c->ring_cursor * sizeof(float));
					memcpy(&new_stats_cpu[c->ring_cursor],
					       &c->stats_cpu[c->pixmap_width - new_pixmap_width +
					                     c->ring_cursor],
					       (new_pixmap_width - c->ring_cursor) * sizeof(float));
				}
				else
				{
					/* New allocation is smaller, and also smaller than the ring
					 * buffer cursor.
					 * Discard all oldest samples following the ring buffer
					 * cursor and additional samples at the beginning of the
					 * buffer. */
					memcpy(&new_stats_cpu[0],
					       &c->stats_cpu[c->ring_cursor - new_pixmap_width],
					       new_pixmap_width * sizeof(float));
					c->ring_cursor = 0;
				}
				g_free(c->stats_cpu);
			}
			c->stats_cpu = new_stats_cpu;
		}

		/* Allocate or reallocate pixmap. */
		c->pixmap_width  = new_pixmap_width;
		c->pixmap_height = new_pixmap_height;
		if (c->pixmap)
			cairo_surface_destroy(c->pixmap);
		c->pixmap = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
		                                       c->pixmap_width,
		                                       c->pixmap_height);
		/* check_cairo_surface_status(&c->pixmap); */

		/* Redraw pixmap at the new size. */
		redraw_pixmap(c);
	}
	return true;
}

/* Handler for draw on drawing area. */
static bool draw(GtkWidget *widget, cairo_t *cr, CpuApplet *c)
{
	/* Draw the requested part of the pixmap onto the drawing area.
	 * Translate it in both x and y by the border size. */
	if (c->pixmap != NULL)
	{
		GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(c));
		GtkStateFlags flags      = gtk_widget_get_state_flags(GTK_WIDGET(c));
		GdkRGBA background_color;
		gtk_style_context_get(context, flags, "background-color", &background_color, NULL);
		gdk_cairo_set_source_rgba(cr, &background_color);
		cairo_set_source_surface(cr, c->pixmap, BORDER_SIZE, BORDER_SIZE);
		cairo_paint(cr);
		/* check_cairo_status(cr); */
	}
	return false;
}

static void on_height_change(GObject *owner, GParamSpec *pspec, void *data)
{
	GtkWidget *da = GTK_WIDGET(data);
	uint height;
	g_object_get(owner, VP_KEY_HEIGHT, &height, NULL);
	gtk_widget_set_size_request(da, height > 40 ? height : 40, height);
}

/* Applet widget constructor. */
CpuApplet *cpu_applet_new(ValaPanelToplevel *toplevel, GSettings *settings, const char *uuid)
{
	/* Allocate applet context*/
	CpuApplet *c = VALA_PANEL_CPU_APPLET(
	    vala_panel_applet_construct(cpu_applet_get_type(), toplevel, settings, uuid));

	/* Allocate drawing area as a child of top level widget. */
	GtkDrawingArea *da = gtk_drawing_area_new();
	gtk_widget_add_events(da,
	                      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
	                          GDK_BUTTON_MOTION_MASK);
	on_height_change(toplevel, NULL, da);
	gtk_container_add(GTK_CONTAINER(c), da);
	GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(toplevel));
	GtkStateFlags flags      = gtk_widget_get_state_flags(GTK_WIDGET(toplevel));
	gtk_style_context_get_color(context, flags, &c->foreground_color);

	/* Connect signals. */
	g_signal_connect(G_OBJECT(toplevel),
	                 "notify::" VP_KEY_HEIGHT,
	                 G_CALLBACK(on_height_change),
	                 (gpointer)da);
	g_signal_connect(G_OBJECT(da), "configure-event", G_CALLBACK(configure_event), (gpointer)c);
	g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(draw), (gpointer)c);
	/* Show the widget.  Connect a timer to refresh the statistics. */
	gtk_widget_show(da);
	c->timer = g_timeout_add(1500, (GSourceFunc)cpu_update, (gpointer)c);
	gtk_widget_show(GTK_WIDGET(c));
	return c;
}

/* Plugin destructor. */
static void cpu_applet_finalize(GObject *user_data)
{
	CpuApplet *c = VALA_PANEL_CPU_APPLET(user_data);

	/* Disconnect the timer. */
	g_source_remove(c->timer);

	/* Deallocate memory. */
	cairo_surface_destroy(c->pixmap);
	g_free(c->stats_cpu);
	g_free(c);
}

static void cpu_applet_init(CpuApplet *self)
{
}

static void cpu_applet_class_init(CpuAppletClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = cpu_applet_finalize;
}

static void cpu_applet_class_finalize(CpuAppletClass *klass)
{
}

/*
 * Plugin functions
 */

struct _CpuPlugin
{
	ValaPanelAppletPlugin parent;
};

G_DEFINE_DYNAMIC_TYPE(CpuPlugin, cpu_plugin, vala_panel_applet_plugin_get_type())

static ValaPanelApplet *cpu_plugin_get_applet_widget(ValaPanelAppletPlugin *base,
                                                     ValaPanelToplevel *toplevel,
                                                     GSettings *settings, const char *uuid)
{
	g_return_val_if_fail(toplevel != NULL, NULL);
	g_return_val_if_fail(uuid != NULL, NULL);

	return VALA_PANEL_APPLET(cpu_applet_new(toplevel, settings, uuid));
}

CpuApplet *cpu_plugin_new(GType object_type)
{
	return VALA_PANEL_CPU_PLUGIN(vala_panel_applet_plugin_construct(cpu_applet_get_type()));
}

static void cpu_plugin_class_init(CpuPluginClass *klass)
{
	((ValaPanelAppletPluginClass *)klass)->get_applet_widget = cpu_plugin_get_applet_widget;
}

static void cpu_plugin_init(CpuPlugin *self)
{
}

static void cpu_plugin_class_finalize(CpuPluginClass *klass)
{
}

/*
 * IO Module functions
 */

void g_io_cpu_load(GTypeModule *module)
{
	g_return_if_fail(module != NULL);

	cpu_applet_register_type(module);
	cpu_plugin_register_type(module);

	g_type_module_use(module);
	g_io_extension_point_implement(VALA_PANEL_APPLET_EXTENSION_POINT,
	                               cpu_plugin_get_type(),
	                               "cpu",
	                               10);
}

void g_io_cpu_unload(GIOModule *module)
{
	g_return_if_fail(module != NULL);
}
