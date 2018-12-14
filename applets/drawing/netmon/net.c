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

#include "net.h"

#define NET_MAX_MBPS 1024 /* We assume than we use gigabyte card (on most desktops we do)*/
#define BYTE_TO_MBPS 1024 * 1024

typedef struct
{                                       /* Value from /proc/net/dev */
	long long unsigned int b, e, m; /* Bytes, errors, max_value */
} net_stat;

static inline bool scan_net_file(net_stat *rx_stat, net_stat *tx_stat)
{
	char buf[256];
	long long unsigned int rxb, rxe, txb, txe;
	long long unsigned int crxb = 0, crxe = 0, ctxb = 0, ctxe = 0;
	FILE *fp = fopen("/proc/net/dev", "r");
	if (fp == NULL)
		return false;
	/* Ignore first two lines - it is a table header */
	if (fgets(buf, 255, fp) == NULL)
		return false;
	if (fgets(buf, 255, fp) == NULL)
		return false;
	while (!feof(fp))
	{
		if (fgets(buf, 255, fp) == NULL)
			break;

		int fscanf_result = 0;
		/* Ensure that fscanf succeeded. */
		sscanf(buf,
		       "%*s %lld, %*d %lld %*d %*d %*d %*d %*d %lld %*d %lld",
		       &rxb,
		       &rxe,
		       &txb,
		       &txe);
		if (fscanf_result != 4)
			return false;
		crxb += rxb;
		crxe += rxe;
		ctxb += txb;
		ctxe += txe;
	}
	if (rx_stat)
	{
		rx_stat->b = crxb;
		rx_stat->e = crxe;
	}
	if (tx_stat)
	{
		tx_stat->b = ctxb;
		tx_stat->e = ctxe;
	}
	return true;
}

//            /* Compute delta from previous statistics. */
//            cpu_stat cpu_delta;
//            cpu_delta.u = cpu.u - previous_cpu_stat.u;
//            cpu_delta.n = cpu.n - previous_cpu_stat.n;
//            cpu_delta.s = cpu.s - previous_cpu_stat.s;
//            cpu_delta.i = cpu.i - previous_cpu_stat.i;

//            /* Copy current to previous. */
//            memcpy(&previous_cpu_stat, &cpu, sizeof(cpu_stat));

//            /* Compute user+nice+system as a fraction of total.
//             * Introduce this sample to ring buffer, increment and wrap ring buffer
//             * cursor. */
//            float cpu_uns            = cpu_delta.u + cpu_delta.n + cpu_delta.s;
//            c->stats[c->ring_cursor] = cpu_uns / (cpu_uns + cpu_delta.i);
//            c->ring_cursor += 1;
//            if (c->ring_cursor >= c->pixmap_width)
//                c->ring_cursor = 0;

/*
 * Network monitor functions
 */
G_GNUC_INTERNAL bool update_net_rx(NetMon *m)
{
	/* Commented because I do not know how to handle overflows (note to both functions)*/
	//    static net_stat previous_net_stat = { 0, 0 };

	m->total = NET_MAX_MBPS;
	if ((m->stats != NULL) && (m->pixmap != NULL))
	{
		/* Open statistics file and scan out net usage. */
		net_stat net;
		bool success = scan_net_file(&net, NULL);
		if (success)
		{
			m->stats[m->ring_cursor] =
			    (net.b - net.e) / (double)(BYTE_TO_MBPS * NET_MAX_MBPS);

			m->ring_cursor += 1;
			if (m->ring_cursor >= m->pixmap_width)
				m->ring_cursor = 0;

			/* Redraw with the new sample. */
			netmon_redraw_pixmap(m);
		}
	}
	return G_SOURCE_CONTINUE;
}

G_GNUC_INTERNAL void tooltip_update_net_rx(NetMon *m)
{
	if (m != NULL && m->stats != NULL)
	{
		int ring_pos = (m->ring_cursor == 0) ? m->pixmap_width - 1 : m->ring_cursor - 1;
		if (m->da != NULL && m->stats != NULL)
		{
			g_autofree char *tooltip_txt =
			    g_strdup_printf(_("Net receive: %.1fMB/s"),
			                    m->stats[ring_pos] * NET_MAX_MBPS);
			gtk_widget_set_tooltip_text(GTK_WIDGET(m->da), tooltip_txt);
		}
	}
}

G_GNUC_INTERNAL bool update_net_tx(NetMon *m)
{
	m->total = NET_MAX_MBPS;
	if ((m->stats != NULL) && (m->pixmap != NULL))
	{
		/* Open statistics file and scan out net usage. */
		net_stat net;
		bool success = scan_net_file(NULL, &net);
		if (success)
		{
			m->stats[m->ring_cursor] =
			    (net.b - net.e) / (double)(BYTE_TO_MBPS * NET_MAX_MBPS);

			m->ring_cursor += 1;
			if (m->ring_cursor >= m->pixmap_width)
				m->ring_cursor = 0;

			/* Redraw with the new sample. */
			netmon_redraw_pixmap(m);
		}
	}
	return G_SOURCE_CONTINUE;
}

G_GNUC_INTERNAL void tooltip_update_net_tx(NetMon *m)
{
	if (m != NULL && m->stats != NULL)
	{
		int ring_pos = (m->ring_cursor == 0) ? m->pixmap_width - 1 : m->ring_cursor - 1;
		if (m->da != NULL && m->stats != NULL)
		{
			g_autofree char *tooltip_txt =
			    g_strdup_printf(_("Net transmit: %.1fMB/s"),
			                    m->stats[ring_pos] * NET_MAX_MBPS);
			gtk_widget_set_tooltip_text(GTK_WIDGET(m->da), tooltip_txt);
		}
	}
}
