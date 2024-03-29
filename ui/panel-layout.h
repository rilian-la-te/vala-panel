/*
 * vala-panel
 * Copyright (C) 2015-2018 Konstantin Pugin <ria.freelander@gmail.com>
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
#ifndef PANELLAYOUT_H
#define PANELLAYOUT_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "applet-manager.h"
#include "toplevel.h"
#include "util.h"

G_BEGIN_DECLS

typedef enum
{
	PACK_START = GTK_PACK_START,
	PACK_END   = GTK_PACK_END,
	PACK_CENTER,
} ValaPanelAppletPackType;

G_DECLARE_FINAL_TYPE(ValaPanelLayout, vala_panel_layout, VALA_PANEL, LAYOUT, GtkBox)

/**
 * vala_panel_layout_get_applets_list:
 * @self: a #ValaPanelLayout
 * 
 * Get a list of applets for these layout. 
 * Returns: (transfer none) (element-type ValaPanelApplet): list of #ValaPanelApplet instances inside layout
 */
GList *vala_panel_layout_get_applets_list(ValaPanelLayout *self);

G_END_DECLS

#endif // PANELLAYOUT_H
