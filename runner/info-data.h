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

#ifndef INFODATA_H
#define INFODATA_H

#include <gio/gio.h>
#include <glib-object.h>
#include <stdbool.h>

G_BEGIN_DECLS

typedef struct info_data
{
	GIcon *icon;
	char *name_markup;
	char *disp_name;
	char *command;
} InfoData;

InfoData *info_data_new_from_info(GAppInfo *info);
InfoData *info_data_new_from_command(const char *command);
void info_data_free(InfoData *data);

G_DECLARE_FINAL_TYPE(InfoDataModel, info_data_model, VALA_PANEL, INFO_DATA_MODEL, GObject)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(InfoData, info_data_free);

InfoDataModel *info_data_model_new();
G_GNUC_INTERNAL GSequence *info_data_model_get_sequence(InfoDataModel *model);

G_END_DECLS

#endif // INFODATA_H
