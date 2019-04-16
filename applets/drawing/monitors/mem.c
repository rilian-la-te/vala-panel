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
#include <stdbool.h>
#include <stdio.h>

#include "mem.h"

/*
 * Memory monitor functions
 */
G_GNUC_INTERNAL bool update_mem(Monitor *m)
{
	char buf[80];
	long mem_total = 0;
	long mem_free  = 0;
	uint readmask  = 0x2 | 0x1;
	if (m->stats == NULL || m->pixmap == NULL)
		return true;
	FILE *meminfo = fopen("/proc/meminfo", "r");
	if (meminfo == NULL)
	{
		g_warning("monitors: Could not open /proc/meminfo: %d, %s", errno, strerror(errno));
		return false;
	}

	/* Use new 3.14 MemAvailable spec */
	while (readmask != 0 && fgets(buf, 80, meminfo) != NULL)
	{
		if (sscanf(buf, "MemTotal: %ld kB\n", &mem_total) == 1)
		{
			readmask ^= 0x1;
			continue;
		}
		if (sscanf(buf, "MemAvailable: %ld kB\n", &mem_free) == 1)
		{
			readmask ^= 0x2;
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

	m->total                 = mem_total;
	m->stats[m->ring_cursor] = (mem_total - mem_free) / (double)mem_total;

	m->ring_cursor += 1;
	if (m->ring_cursor >= m->pixmap_width)
		m->ring_cursor = 0;
	/* Redraw the pixmap, with the new sample */
	monitor_redraw_pixmap(m);
	return true;
}

G_GNUC_INTERNAL void tooltip_update_mem(Monitor *m)
{
	if (m != NULL && m->stats != NULL)
	{
		int ring_pos = (m->ring_cursor == 0) ? m->pixmap_width - 1 : m->ring_cursor - 1;
		if (m->da != NULL && m->stats != NULL)
		{
			g_autofree char *tooltip_txt =
			    g_strdup_printf(_("RAM usage: %.1fMB (%.2f%%)"),
			                    m->stats[ring_pos] * m->total / 1024,
			                    m->stats[ring_pos] * 100);
			gtk_widget_set_tooltip_text(GTK_WIDGET(m->da), tooltip_txt);
		}
	}
}
