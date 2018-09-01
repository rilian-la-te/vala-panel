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

#include "cpu.h"

/*
 * CPU monitor functions
 */

typedef struct
{                                          /* Value from /proc/stat */
	long long unsigned int u, n, s, i; /* User, nice, system, idle */
} cpu_stat;

G_GNUC_INTERNAL bool cpu_update(Monitor *c)
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

G_GNUC_INTERNAL void tooltip_update_cpu(Monitor *m)
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
