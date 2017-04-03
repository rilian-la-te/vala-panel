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

#ifndef MENUMAKER_H
#define MENUMAKER_H
#include <gio/gio.h>
#include <glib.h>
#include <gtk/gtk.h>

#define ATTRIBUTE_DND_SOURCE "x-valapanel-dnd-source"
#define ATTRIBUTE_TOOLTIP "x-valapanel-tooltip"

void apply_menu_properties(GList *w, GMenuModel *menu);
void append_all_sections(GMenu *menu1, GMenuModel *menu2);

#endif // MENUMAKER_H
