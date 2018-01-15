/*
 * vala-panel
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
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

#ifndef GLISTMODELFILTER_H
#define GLISTMODELFILTER_H

#include <gio/gio.h>
#include <glib-object.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelListModelFilter, vala_panel_list_model_filter, VALA_PANEL,
                     LIST_MODEL_FILTER, GObject)

typedef bool (*ValaPanelListModelFilterFunc)(gpointer, gpointer);

ValaPanelListModelFilter *vala_panel_list_model_filter_new(GListModel *base_model);
void vala_panel_list_model_filter_set_filter_func(ValaPanelListModelFilter *self,
                                                  ValaPanelListModelFilterFunc func,
                                                  gpointer user_data);
void vala_panel_list_model_filter_invalidate(ValaPanelListModelFilter *self);
void vala_panel_list_model_filter_set_max_results(ValaPanelListModelFilter *self, uint max_results);
G_END_DECLS

#endif // GLISTMODELFILTER_H
