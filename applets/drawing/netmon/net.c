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

#include "net.h"

#define NET_SAMPLE_COUNT 20
#define min(a, b) (a) < (b) ? a : b
#define max(a, b) (a) > (b) ? a : b

/*
 * Network monitor functions
 */

struct net_stat
{
	long long last_down, last_up;
	int cur_idx;
	long long down[NET_SAMPLE_COUNT], up[NET_SAMPLE_COUNT];
	double down_rate, up_rate;
	double max_down, max_up;
};

G_GNUC_INTERNAL bool update_net(NetMon *mon)
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

	return true;
}

G_GNUC_INTERNAL void tooltip_update_net(NetMon *m)
{
	//	if (m != NULL && m->stats != NULL)
	//	{
	//		int ring_pos = (m->ring_cursor == 0) ? m->pixmap_width - 1 : m->ring_cursor
	//- 1; 		if (m->da != NULL && m->stats != NULL)
	//		{
	//			g_autofree char *tooltip_txt =
	//			    g_strdup_printf(_("Net receive: %.1fMB/s"),
	//			                    m->stats[ring_pos] * NET_MAX_MBPS);
	//			gtk_widget_set_tooltip_text(GTK_WIDGET(m->da), tooltip_txt);
	//		}
	//	}
}
