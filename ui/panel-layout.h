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
	PACK_START  = 0,
	PACK_CENTER = 2,
	PACK_END    = 1,
} PanelAppletPackType;

G_DECLARE_FINAL_TYPE(ValaPanelLayout, vala_panel_layout, VALA_PANEL, LAYOUT, GtkBox)

ValaPanelLayout *vala_panel_layout_new(ValaPanelToplevel *top, GtkOrientation orient, gint spacing);
ValaPanelLayout *vala_panel_layout_construct(GType object_type, ValaPanelToplevel *top,
                                             GtkOrientation orient, gint spacing);
void vala_panel_layout_init_applets(ValaPanelLayout *self);
void vala_panel_layout_add_applet(ValaPanelLayout *self, const gchar *type);
void vala_panel_layout_place_applet(ValaPanelLayout *self, AppletInfoData *data,
                                    ValaPanelUnitSettings *s);
void vala_panel_layout_remove_applet(ValaPanelLayout *self, ValaPanelApplet *applet);
void vala_panel_layout_applet_destroyed(ValaPanelLayout *self, const char *uuid);
void vala_panel_layout_update_applet_positions(ValaPanelLayout *self);
GList *vala_panel_layout_get_applets_list(ValaPanelLayout *self);
ValaPanelUnitSettings *vala_panel_layout_get_applet_settings(ValaPanelApplet *pl);
uint vala_panel_layout_get_applet_position(ValaPanelLayout *self, ValaPanelApplet *pl);
void vala_panel_layout_set_applet_position(ValaPanelLayout *self, ValaPanelApplet *pl, int pos);
const char *vala_panel_layout_get_toplevel_id(ValaPanelLayout *self);
ValaPanelAppletManager *vala_panel_layout_get_manager();

G_END_DECLS

#endif // PANELLAYOUT_H
