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

ValaPanelLayout *vala_panel_layout_new(ValaPanelToplevel *top, GtkOrientation orient, gint spacing);
ValaPanelLayout *vala_panel_layout_construct(GType object_type, ValaPanelToplevel *top,
                                             GtkOrientation orient, gint spacing);
void vala_panel_layout_init_applets(ValaPanelLayout *self);
ValaPanelApplet *vala_panel_layout_insert_applet(ValaPanelLayout *self, const char *type,
                                                 ValaPanelAppletPackType pack, uint pos);
ValaPanelApplet *vala_panel_layout_place_applet(ValaPanelLayout *self, AppletInfoData *data,
                                                ValaPanelUnitSettings *s);
void vala_panel_layout_remove_applet(ValaPanelLayout *self, ValaPanelApplet *applet);
void vala_panel_layout_applets_repack(ValaPanelLayout *self);
void vala_panel_layout_update_applet_positions(ValaPanelLayout *self);
bool vala_panel_layout_can_move_to_direction(ValaPanelLayout *self, ValaPanelApplet *prev,
                                             ValaPanelApplet *next, GtkPackType direction);
void vala_panel_layout_move_applet_one_step(ValaPanelLayout *self, ValaPanelApplet *prev,
                                            ValaPanelApplet *next, GtkPackType direction);
GList *vala_panel_layout_get_applets_list(ValaPanelLayout *self);
ValaPanelUnitSettings *vala_panel_layout_get_applet_settings(ValaPanelApplet *pl);
ValaPanelAppletPackType vala_panel_layout_get_applet_pack_type(ValaPanelApplet *pl);
unsigned int vala_panel_layout_get_applet_position(ValaPanelLayout *self, ValaPanelApplet *pl);
const char *vala_panel_layout_get_toplevel_id(ValaPanelLayout *self);
ValaPanelAppletManager *vala_panel_layout_get_manager();

G_END_DECLS

#endif // PANELLAYOUT_H
