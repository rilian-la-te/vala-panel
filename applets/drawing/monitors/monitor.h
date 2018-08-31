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

#ifndef MONITOR_H
#define MONITOR_H

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

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

G_GNUC_INTERNAL void monitor_init_no_height(Monitor *mon, const char *color);
G_GNUC_INTERNAL void monitor_redraw_pixmap(Monitor *mon);
G_GNUC_INTERNAL bool monitor_update(Monitor *mon);
G_GNUC_INTERNAL void monitor_dispose(Monitor *mon);

G_END_DECLS

#endif // MONITORS_H
