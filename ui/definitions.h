/*
 * vala-panel
 * Copyright (C) 2015-2016 Konstantin Pugin <ria.freelander@gmail.com>
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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#define gtk_widget_destroy0(x) g_clear_pointer(&x, gtk_widget_destroy)
#define g_object_unref0(x) g_clear_pointer(&x, g_object_unref)
#define g_free0(x) g_clear_pointer(&x, g_free)

#define g_value_replace_string(string, value)                                                      \
                                                                                                   \
	{                                                                                          \
		g_free0(string);                                                                   \
		string = g_value_dup_string(value);                                                \
	}

#define g_ascii_inplace_tolower(string)                                                            \
	{                                                                                          \
		for (int i = 0; string[i] != '\0'; i++)                                            \
			g_ascii_tolower(string[i]);                                                \
	}

#define vala_panel_bind_gsettings(obj, settings, prop)                                             \
	g_settings_bind(settings,                                                                  \
	                prop,                                                                      \
	                G_OBJECT(obj),                                                             \
	                prop,                                                                      \
	                (GSettingsBindFlags)(G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET |           \
	                                     G_SETTINGS_BIND_DEFAULT));

#define vala_panel_orient_from_edge(edge)                                                          \
	((edge == GTK_POS_TOP) || (edge == GTK_POS_BOTTOM)) ? GTK_ORIENTATION_HORIZONTAL           \
	                                                    : GTK_ORIENTATION_VERTICAL

#define vala_panel_orient_from_gravity(gravity)                                                    \
	(((gravity) < 6) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL)

#define vala_panel_edge_from_gravity(gravity)                                                      \
	(gravity < 3)                                                                              \
	    ? GTK_POS_TOP                                                                          \
	    : (gravity < 6) ? GTK_POS_BOTTOM : (gravity < 9) ? GTK_POS_LEFT : GTK_POS_RIGHT

#define vala_panel_invert_orient(orient)                                                           \
	orient == GTK_ORIENTATION_HORIZONTAL ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL

#define vala_panel_effective_height(orient)                                                        \
	orient == GTK_ORIENTATION_HORIZONTAL ? gtk_widget_get_allocated_height(GTK_WIDGET(top))    \
	                                     : gtk_widget_get_allocated_width(GTK_WIDGET(top))

#define vala_panel_effective_width(orient)                                                         \
	orient == GTK_ORIENTATION_HORIZONTAL ? gtk_widget_get_allocated_width(GTK_WIDGET(top))     \
	                                     : gtk_widget_get_allocated_height(GTK_WIDGET(top))

#define vala_panel_transpose_area(marea)                                                           \
	{                                                                                          \
		int i        = marea.height;                                                       \
		marea.height = marea.width;                                                        \
		marea.width  = i;                                                                  \
		i            = marea.y;                                                            \
		marea.y      = marea.x;                                                            \
		marea.x      = i;                                                                  \
	}

#define vala_panel_str_is_empty(str) !str ? true : !g_strcmp0(str, "") ? true : false

#define vala_panel_dup_array(DST, SRC, LEN)                                                        \
	{                                                                                          \
		size_t TMPSZ = sizeof(*(SRC)) * (LEN);                                             \
		if (((DST) = malloc(TMPSZ)) != NULL)                                               \
			memcpy((DST), (SRC), TMPSZ);                                               \
	}

#endif // DEFINITIONS_H
