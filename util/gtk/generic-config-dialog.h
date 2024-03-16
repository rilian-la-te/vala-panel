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

#ifndef GENERICCONFIGDIALOG_H
#define GENERICCONFIGDIALOG_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum
{
	CONF_STR,
	CONF_INT,
	CONF_BOOL,
	CONF_FILE,
	CONF_FILE_ENTRY,
	CONF_DIRECTORY,
	CONF_DIRECTORY_ENTRY,
	CONF_TRIM,
} ValaPanelConfiguratorType;

/**
 * vala_panel_generic_cfg_widgetv: (skip)
 * Generate configuration for specific keys and values without need to create a custom widget
 * @settings: a #GSettings
 * @...: variable args with specific format
 *
 * Returns: (transfer full): a #GtkWidget for configuring an applet with provided GSettings
 */
GtkWidget *vala_panel_generic_cfg_widgetv(GSettings *settings, ...);

/**
 * vala_panel_generic_cfg_widget:
 * Generate configuration for specific keys and values without need to create a custom widget
 * @settings: a #GSettings
 * @names: (array length=n_entries): names of config entries (shown in UI, should be translatable)
 * @keys: (array length=n_entries): #GSettings keys of config entries (should be defined in provided
 * schema)
 * @types: (array length=n_entries): a #GenericConfigType of provided settings. External widgets is
 * not supported in this version of function.
 * @n_entries: number of config entries
 *
 * Returns: (transfer full): a #GtkWidget for configuring an applet with provided GSettings
 */
GtkWidget *vala_panel_generic_cfg_widget(GSettings *settings, const char **names,
                                            const char **keys, ValaPanelConfiguratorType *types,
                                            uint n_entries);

G_END_DECLS

#endif // GENERICCONFIGDIALOG_H
