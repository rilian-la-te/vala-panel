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

#include "monitor.h"

#define BORDER_SIZE 2 /* Pixels               */

/*
 * Generic monitor functions and events
 */

G_GNUC_INTERNAL bool monitor_update(Monitor *mon)
{
	if (mon->tooltip_update != NULL && mon->da != NULL)
		mon->tooltip_update(mon);
	return mon->update(mon);
}

G_GNUC_INTERNAL void monitor_redraw_pixmap(Monitor *mon)
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

G_GNUC_INTERNAL void monitor_init_no_height(Monitor *mon, const char *color)
{
	mon->da = GTK_DRAWING_AREA(gtk_drawing_area_new());
	gtk_widget_add_events(GTK_WIDGET(mon->da),
	                      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
	                          GDK_BUTTON_MOTION_MASK);
	gdk_rgba_parse(&mon->foreground_color, color);
	g_signal_connect(mon->da, "configure-event", G_CALLBACK(configure_event), mon);
	g_signal_connect(mon->da, "draw", G_CALLBACK(draw), mon);
}

G_GNUC_INTERNAL void monitor_dispose(Monitor *mon)
{
	gtk_widget_destroy(GTK_WIDGET(mon->da));
	g_clear_pointer(&mon->pixmap, cairo_surface_destroy);
	g_clear_pointer(&mon->stats, g_free);
	g_clear_pointer(&mon, g_free);
}
