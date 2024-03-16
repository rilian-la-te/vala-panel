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

#define vala_panel_bind_gsettings(obj, settings, prop)                                             \
	g_settings_bind(settings,                                                                  \
	                prop,                                                                      \
	                G_OBJECT(obj),                                                             \
	                prop,                                                                      \
	                (GSettingsBindFlags)(G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET |           \
	                                     G_SETTINGS_BIND_DEFAULT));

/**
 * vala_panel_orient_from_gravity:
 * @gravity: (type ValaPanelGravity): a #ValaPanelGravity
 *
 * Returns: (type GtkOrientation): #GtkOrientation for this #ValaPanelGravity
 */
int vala_panel_orient_from_gravity(int gravity);

/**
 * vala_panel_edge_from_gravity:
 * @gravity: (type ValaPanelGravity): a #ValaPanelGravity
 *
 * Returns: (type GtkPositionType): #GtkPositionType for this #ValaPanelGravity
 */
int vala_panel_edge_from_gravity(int gravity);

#endif // DEFINITIONS_H
