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

#define NET_SAMPLE_COUNT 5
#define SAMPLE_SCALE_MULT 0.1

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
	static bool first_run = true;
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
			first_run = false;
			break;
		}

		unsigned int curtmp1 = 0;
		unsigned int curtmp2 = 0;
		/* Average the samples */
		for (int i = 0; i < mon->average_samples; i++)
		{
			curtmp1 +=
			    net.down[(net.cur_idx + NET_SAMPLE_COUNT - i) % NET_SAMPLE_COUNT];
			curtmp2 += net.up[(net.cur_idx + NET_SAMPLE_COUNT - i) % NET_SAMPLE_COUNT];
		}
		net.down_rate = curtmp1 / (double)mon->average_samples;
		net.up_rate   = curtmp2 / (double)mon->average_samples;
		/* Count current values for tooltip */
		mon->down_current = net.down_rate;
		mon->up_current   = net.up_rate;
		/* Count current values for tooltip */
		{
			/* Check if we need downscaling */
			double max_up_sample = 0.0, max_down_sample = 0.0;
			for (int i = 0; i < mon->pixmap_width; i++)
			{
				max_up_sample   = MAX(max_up_sample, mon->up_stats[i]);
				max_down_sample = MAX(max_down_sample, mon->down_stats[i]);
			}
			if (max_up_sample < SAMPLE_SCALE_MULT &&
			    max_down_sample < SAMPLE_SCALE_MULT)
			{
				/* We need downscaling, process it */
				for (int i = 0; i < mon->pixmap_width; i++)
				{
					mon->down_stats[i] /= SAMPLE_SCALE_MULT;
					mon->up_stats[i] /= SAMPLE_SCALE_MULT;
				}
				net.max_up *= SAMPLE_SCALE_MULT;
				net.max_down *= SAMPLE_SCALE_MULT;
			}
			/* We need one maximum to maintain consistent curves */
			net.max_up   = MAX(net.max_up, net.max_down);
			net.max_down = MAX(net.max_up, net.max_down);

			/* Normalize by maximum speed (a priori unknown,
			 so we must do this all the time). */
			if (net.max_down < net.down_rate)
			{
				for (int i = 0; i < mon->pixmap_width; i++)
					mon->down_stats[i] *= (net.max_down / net.down_rate);
				net.max_down  = net.down_rate;
				net.down_rate = 1.0;
			}
			else if (net.max_down != 0)
				net.down_rate /= net.max_down;
		}
		{
			if (net.max_up < net.up_rate)
			{
				for (int i = 0; i < mon->pixmap_width; i++)
					mon->up_stats[i] *= (net.max_up / net.up_rate);
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

	mon->down_stats[mon->ring_cursor] = net.down_rate;
	mon->up_stats[mon->ring_cursor]   = net.up_rate;

	mon->ring_cursor += 1;
	if (mon->ring_cursor >= mon->pixmap_width)
		mon->ring_cursor = 0;
	netmon_redraw_pixmap(mon);
	return true;
}

static double count_coeff(double rate, double max)
{
	double bytes = rate * max;
	if (bytes > 1073741824)
		return bytes / 1073741824;
	else if (bytes > 1048576)
		return bytes / 1048576;
	else if (bytes > 1024)
		return bytes / 1024;
	else
		return bytes;
}

static const char *get_relevant_char(double bytes)
{
	if (bytes > 1073741824)
		return _("GB/s");
	else if (bytes > 1048576)
		return _("MB/s");
	else if (bytes > 1024)
		return _("KB/s");
	else
		return _("B/s");
}

G_GNUC_INTERNAL void tooltip_update_net(NetMon *m)
{
	if (m != NULL && m->down_stats != NULL && m->up_stats != NULL)
	{
		int ring_pos = (m->ring_cursor == 0) ? m->pixmap_width - 1 : m->ring_cursor - 1;
		if (m->da != NULL && m->down_stats != NULL && m->up_stats != NULL)
		{
			g_autofree char *tooltip_txt =
			    g_strdup_printf(_("Net receive: %.3f %s \n Net transmit: %.3f %s\n"),
			                    count_coeff(m->down_stats[ring_pos], m->down_current),
			                    get_relevant_char(m->down_current),
			                    count_coeff(m->up_stats[ring_pos], m->up_current),
			                    get_relevant_char(m->up_current));
			gtk_widget_set_tooltip_text(GTK_WIDGET(m->da), tooltip_txt);
		}
	}
}
