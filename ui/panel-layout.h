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

G_DECLARE_FINAL_TYPE(ValaPanelLayout, vp_layout, VALA_PANEL, LAYOUT, GtkBox)

G_GNUC_INTERNAL ValaPanelLayout *vp_layout_new(ValaPanelToplevel *top, GtkOrientation orient,
                                               gint spacing);
G_GNUC_INTERNAL void vp_layout_init_applets(ValaPanelLayout *self);
G_GNUC_INTERNAL ValaPanelApplet *vp_layout_insert_applet(ValaPanelLayout *self, const char *type,
                                                         ValaPanelAppletPackType pack, uint pos);
G_GNUC_INTERNAL void vp_layout_remove_applet(ValaPanelLayout *self, ValaPanelApplet *applet);
G_GNUC_INTERNAL void vp_layout_update_applet_positions(ValaPanelLayout *self);
G_GNUC_INTERNAL bool vp_layout_can_move_to_direction(ValaPanelLayout *self, ValaPanelApplet *prev,
                                                     ValaPanelApplet *next, GtkPackType direction);
G_GNUC_INTERNAL void vp_layout_move_applet_one_step(ValaPanelLayout *self, ValaPanelApplet *prev,
                                                    ValaPanelApplet *next, GtkPackType direction);
GList *vp_layout_get_applets_list(ValaPanelLayout *self);
G_GNUC_INTERNAL ValaPanelUnitSettings *vp_layout_get_applet_settings(ValaPanelApplet *pl);
G_GNUC_INTERNAL ValaPanelAppletPackType vp_layout_get_applet_pack_type(ValaPanelApplet *pl);
G_GNUC_INTERNAL unsigned int vp_layout_get_applet_position(ValaPanelLayout *self,
                                                           ValaPanelApplet *pl);
G_GNUC_INTERNAL ValaPanelAppletManager *vp_layout_get_manager(void);

G_END_DECLS

#endif // PANELLAYOUT_H
