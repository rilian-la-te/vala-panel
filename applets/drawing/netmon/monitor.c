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
#define NET_SAMPLE_COUNT 20
#define min(a, b) (a) < (b) ? a : b
#define max(a, b) (a) > (b) ? a : b

struct net_stat
{
	long long last_down, last_up;
	int cur_idx;
	long long down[NET_SAMPLE_COUNT], up[NET_SAMPLE_COUNT];
	double down_rate, up_rate;
	double max_down, max_up;
};

/*
 * Generic netmon functions and events
 */

static bool netmon_update_values(NetMon *mon)
{
	static struct net_stat net = { .cur_idx = 0, .max_down = 0, .max_up = 0 };

	FILE *fp;
	if (!(fp = fopen("/proc/net/dev", "r")))
	{
		return 0;
	}
	char buf[256];
	/* Ignore first two lines - header */
	fgets(buf, 255, fp);
	fgets(buf, 255, fp);
	static int first_run = 1;
	while (!feof(fp))
	{
		if (fgets(buf, 255, fp) == NULL)
		{
			break;
		}
		char *p = buf;
		while (isspace((int)*p))
			p++;
		char *curdev = p;
		while (*p && *p != ':')
			p++;
		if (*p == '\0')
			continue;
		*p = '\0';

		if (g_strcmp0(curdev, mon->interface_name))
			continue;

		long long down, up;
		sscanf(p + 1, "%lld %*d %*d %*d %*d %*d %*d %*d %lld", &down, &up);
		if (down < net.last_down)
			net.last_down = 0; // Overflow
		if (up < net.last_up)
			net.last_up = 0; // Overflow
		net.down[net.cur_idx] = (down - net.last_down);
		net.up[net.cur_idx]   = (up - net.last_up);
		net.last_down         = down;
		net.last_up           = up;
		if (first_run)
		{
			first_run = 0;
			break;
		}

		unsigned int curtmp1 = 0;
		unsigned int curtmp2 = 0;
		/* Average the samples */
		int i;
		for (i = 0; i < mon->average_samples; i++)
		{
			curtmp1 +=
			    net.down[(net.cur_idx + NET_SAMPLE_COUNT - i) % NET_SAMPLE_COUNT];
			curtmp2 += net.up[(net.cur_idx + NET_SAMPLE_COUNT - i) % NET_SAMPLE_COUNT];
		}
		net.down_rate = curtmp1 / (double)mon->average_samples;
		net.up_rate   = curtmp2 / (double)mon->average_samples;
		if (mon->rx_total > 0)
		{
			net.down_rate /= (double)mon->rx_total;
			net.down_rate = min(1.0, net.down_rate);
		}
		else
		{
			/* Normalize by maximum speed (a priori unknown,
			 so we must do this all the time). */
			if (net.max_down < net.down_rate)
			{
				for (i = 0; i < mon->pixmap_width; i++)
				{
					mon->rx_stats[i] *= (net.max_down / net.down_rate);
				}
				net.max_down  = net.down_rate;
				net.down_rate = 1.0;
			}
			else if (net.max_down != 0)
				net.down_rate /= net.max_down;
		}
		if (mon->tx_total > 0)
		{
			net.up_rate /= (double)mon->tx_total;
			net.up_rate = min(1.0, net.up_rate);
		}
		else
		{
			if (net.max_up < net.up_rate)
			{
				for (i = 0; i < mon->pixmap_width; i++)
				{
					mon->tx_stats[i] *= (net.max_up / net.up_rate);
				}
				net.max_up  = net.up_rate;
				net.up_rate = 1.0;
			}
			else if (net.max_up != 0)
				net.up_rate /= net.max_up;
		}
		net.cur_idx = (net.cur_idx + 1) % NET_SAMPLE_COUNT;
		break; // Ignore the rest
	}
	fclose(fp);

	mon->rx_stats[mon->ring_cursor] = net.down_rate;
	mon->tx_stats[mon->ring_cursor] = net.up_rate;

	return 0;
}

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
		              (1.0 - mon->rx_stats[drawing_cursor]) * mon->pixmap_height);
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
		              (1.0 - mon->tx_stats[drawing_cursor]) * mon->pixmap_height);
	}
	cairo_stroke(cr);
	/* Finish */
	cairo_status(cr);
	cairo_destroy(cr);
	/* Redraw pixmap */
	gtk_widget_queue_draw(GTK_WIDGET(mon->da));
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
		//		if (mon->stats == NULL || (new_pixmap_width != mon->pixmap_width))
		//		{
		//			double *new_stats = g_new0(double, sizeof(double) *
		// new_pixmap_width); 			if (new_stats == NULL)
		// return G_SOURCE_REMOVE;

		//			if (mon->stats != NULL)
		//			{
		//				/* New allocation is larger.
		//				 * Add new "oldest" samples of zero following the
		// cursor*/ 				if (new_pixmap_width > mon->pixmap_width)
		//				{
		//					/* Number of values between the ring cursor and
		//the end of
		//					 * the buffer */
		//					int nvalues = mon->pixmap_width -
		// mon->ring_cursor; 					memcpy(new_stats,
		// mon->stats, 					       mon->ring_cursor *
		// sizeof(double)); 					memcpy(&new_stats[nvalues],
		// &mon->stats[mon->ring_cursor], 					       nvalues * sizeof(double));
		//				}
		//				/* New allocation is smaller, but still larger than
		//the ring
		//				 * buffer cursor */
		//				else if (mon->ring_cursor <= new_pixmap_width)
		//				{
		//					/* Numver of values that can be stored between
		//the end of
		//					 * the new buffer and the ring cursor */
		//					int nvalues = new_pixmap_width -
		// mon->ring_cursor; 					memcpy(new_stats,
		// mon->stats, 					       mon->ring_cursor *
		// sizeof(double));
		// memcpy(&new_stats[mon->ring_cursor], 					       &mon->stats[mon->pixmap_width - nvalues],
		// nvalues * sizeof(double));
		//				}
		//				/* New allocation is smaller, and also smaller than
		//the ring
		//				 * buffer cursor.  Discard all oldest samples
		//following the ring
		//				 * buffer cursor and additional samples at the
		//beginning of the
		//				 * buffer. */
		//				else
		//				{
		//					memcpy((void *)new_stats,
		//					       (void *)&mon
		//					           ->stats[mon->ring_cursor -
		// new_pixmap_width], 					       new_pixmap_width *
		// sizeof(double));
		//				}
		//			}
		//			g_clear_pointer(&mon->stats, g_free);
		//			mon->stats = new_stats;
		//		}
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

G_GNUC_INTERNAL void netmon_init_no_height(NetMon *mon, const char *color)
{
	mon->da = GTK_DRAWING_AREA(gtk_drawing_area_new());
	gtk_widget_add_events(GTK_WIDGET(mon->da),
	                      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
	                          GDK_BUTTON_MOTION_MASK);
	//	gdk_rgba_parse(&mon->foreground_color, color);
	g_signal_connect(mon->da, "configure-event", G_CALLBACK(configure_event), mon);
	g_signal_connect(mon->da, "draw", G_CALLBACK(draw), mon);
}

G_GNUC_INTERNAL void netmon_dispose(NetMon *mon)
{
	g_clear_pointer(&mon->da, gtk_widget_destroy);
	g_clear_pointer(&mon->pixmap, cairo_surface_destroy);
	//	g_clear_pointer(&mon->stats, g_free);
	g_clear_pointer(&mon, g_free);
}
