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

typedef enum {
	CONF_STR,
	CONF_INT,
	CONF_BOOL,
	CONF_FILE,
	CONF_FILE_ENTRY,
	CONF_DIRECTORY,
	CONF_DIRECTORY_ENTRY,
	CONF_TRIM,
	CONF_EXTERNAL
} GenericConfigType;

GtkDialog *generic_config_dlg(const char *title, GtkWindow *parent, GSettings *settings, ...);
GtkWidget *generic_config_widget(GSettings *settings, ...);

G_END_DECLS

#endif // GENERICCONFIGDIALOG_H
