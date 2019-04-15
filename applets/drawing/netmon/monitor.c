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

#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <stdio.h>

#include "monitor.h"

#define BORDER_SIZE 2 /* Pixels               */

/*
 * Generic netmon functions and events
 */

G_GNUC_INTERNAL bool netmon_update(NetMon *mon)
{
	if (mon->tooltip_update != NULL && mon->da != NULL)
		mon->tooltip_update(mon);
	return mon->update(mon);
}

G_GNUC_INTERNAL void netmon_redraw_pixmap(NetMon *mon)
{
	cairo_t *cr = cairo_create(mon->pixmap);
	cairo_set_line_width(cr, 1.0);
	/* Erase pixmap */
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	/* Draw RX stats */
	gdk_cairo_set_source_rgba(cr, &mon->rx_color);
	for (int i = 0; i < mon->pixmap_width; i++)
	{
		uint drawing_cursor = (mon->ring_cursor + i) % mon->pixmap_width;
		/* Draw one bar of the graph */
		if (mon->use_bar)
			cairo_move_to(cr, i + 0.5, mon->pixmap_height);
		cairo_line_to(cr,
		              i + 0.5,
		              (1.0 - mon->down_stats[drawing_cursor]) * mon->pixmap_height);
	}
	cairo_stroke(cr);
	/* Draw TX stats */
	gdk_cairo_set_source_rgba(cr, &mon->tx_color);
	for (int i = 0; i < mon->pixmap_width; i++)
	{
		uint drawing_cursor = (mon->ring_cursor + i) % mon->pixmap_width;
		/* Draw one bar of the graph */
		if (mon->use_bar)
			cairo_move_to(cr, i + 0.5, mon->pixmap_height);
		cairo_line_to(cr,
		              i + 0.5,
		              (1.0 - mon->up_stats[drawing_cursor]) * mon->pixmap_height);
	}
	cairo_stroke(cr);
	/* Finish */
	cairo_status(cr);
	cairo_destroy(cr);
	/* Redraw pixmap */
	gtk_widget_queue_draw(GTK_WIDGET(mon->da));
}

static void generate_new_stats(double *old_stats, double *new_stats, int old_width, int new_width,
                               int cursor)
{
	/* New allocation is larger.
	 * Add new "oldest" samples of zero following the cursor*/
	if (new_width > old_width)
	{
		/* Number of values between the ring cursor and	the end of
		 * the buffer */
		ulong nvalues = old_width - cursor;
		memcpy(new_stats, old_stats, cursor * sizeof(double));
		memcpy(&new_stats[nvalues], &old_stats[cursor], nvalues * sizeof(double));
	}
	/* New allocation is smaller, but still larger than the ring
	 * buffer cursor */
	else if (cursor <= new_width)
	{
		/* Numver of values that can be stored between the end of
		 * the new buffer and the ring cursor */
		int nvalues = new_width - cursor;
		memcpy(new_stats, old_stats, cursor * sizeof(double));
		memcpy(&new_stats[cursor],
		       &old_stats[old_width - nvalues],
		       nvalues * sizeof(double));
	}
	/* New allocation is smaller, and also smaller than the ring
	 * buffer cursor.  Discard all oldest samples following the ring
	 * buffer cursor and additional samples at the beginning of the
	 * buffer. */
	else
	{
		memcpy((void *)new_stats,
		       (void *)&old_stats[cursor - new_width],
		       new_width * sizeof(double));
	}
}

G_GNUC_INTERNAL bool netmon_resize(GtkWidget *widget, NetMon *mon)
{
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
		if (mon->down_stats == NULL || mon->up_stats == NULL ||
		    (new_pixmap_width != mon->pixmap_width))
		{
			double *new_down_stats = g_new0(double, sizeof(double) * new_pixmap_width);
			double *new_up_stats   = g_new0(double, sizeof(double) * new_pixmap_width);
			if (new_down_stats == NULL || new_up_stats == NULL)
			{
				g_clear_pointer(&new_down_stats, g_free);
				g_clear_pointer(&new_up_stats, g_free);
				return G_SOURCE_REMOVE;
			}
			if (mon->down_stats != NULL)
				generate_new_stats(mon->down_stats,
				                   new_down_stats,
				                   mon->pixmap_width,
				                   new_pixmap_width,
				                   mon->ring_cursor);
			g_clear_pointer(&mon->down_stats, g_free);
			mon->down_stats = new_down_stats;
			if (mon->up_stats != NULL)
				generate_new_stats(mon->up_stats,
				                   new_up_stats,
				                   mon->pixmap_width,
				                   new_pixmap_width,
				                   mon->ring_cursor);
			g_clear_pointer(&mon->up_stats, g_free);
			mon->up_stats = new_up_stats;
		}
		mon->pixmap_width  = new_pixmap_width;
		mon->pixmap_height = new_pixmap_height;
		g_clear_pointer(&mon->pixmap, cairo_surface_destroy);
		mon->pixmap = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		                                         mon->pixmap_width,
		                                         mon->pixmap_height);
		cairo_surface_status(mon->pixmap);
		netmon_redraw_pixmap(mon);
	}
	return G_SOURCE_CONTINUE;
}

static bool configure_event(GtkWidget *widget, GdkEventConfigure *dummy, gpointer data)
{
	NetMon *mon = (NetMon *)data;
	return netmon_resize(widget, mon);
}

static bool draw(GtkWidget *widget, cairo_t *cr, NetMon *mon)
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

G_GNUC_INTERNAL void netmon_init_no_height(NetMon *mon, const char *rx_color, const char *tx_color)
{
	mon->da              = GTK_DRAWING_AREA(gtk_drawing_area_new());
	mon->average_samples = 2;
	gtk_widget_add_events(GTK_WIDGET(mon->da),
	                      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
	                          GDK_BUTTON_MOTION_MASK);
	gdk_rgba_parse(&mon->rx_color, rx_color);
	gdk_rgba_parse(&mon->tx_color, tx_color);
	g_signal_connect(mon->da, "configure-event", G_CALLBACK(configure_event), mon);
	g_signal_connect(mon->da, "draw", G_CALLBACK(draw), mon);
}

G_GNUC_INTERNAL void netmon_dispose(NetMon *mon)
{
	g_clear_pointer(&mon->da, gtk_widget_destroy);
	g_clear_pointer(&mon->pixmap, cairo_surface_destroy);
	g_clear_pointer(&mon->interface_name, g_free);
	g_clear_pointer(&mon->down_stats, g_free);
	g_clear_pointer(&mon->up_stats, g_free);
	g_clear_pointer(&mon, g_free);
}
