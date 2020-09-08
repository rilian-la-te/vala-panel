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
#ifndef APPLETMANAGER_H
#define APPLETMANAGER_H

#include "applet-info.h"
#include "applet-widget-api.h"
#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
G_DECLARE_FINAL_TYPE(VPManager, vp_manager, VP, MANAGER, GObject)

typedef struct
{
	ValaPanelAppletInfo *info;
	uint count;
} AppletInfoData;

G_GNUC_INTERNAL VPManager *vp_manager_new(void);
G_GNUC_INTERNAL void vp_manager_applet_unref(VPManager *self, const char *name);
G_GNUC_INTERNAL ValaPanelApplet *vp_manager_get_applet_widget(VPManager *self, const char *type,
                                                              ValaPanelToplevel *top,
                                                              ValaPanelUnitSettings *s);
G_GNUC_INTERNAL ValaPanelAppletInfo *vp_manager_get_applet_info(
    VPManager *self, ValaPanelApplet *pl, ValaPanelCoreSettings *core_settings);
G_GNUC_INTERNAL void vp_manager_reload_applets(VPManager *self);
G_GNUC_INTERNAL GList *vp_manager_get_all_types(VPManager *self);
G_GNUC_INTERNAL bool vp_manager_is_applet_available(VPManager *self, const char *module_name);

G_END_DECLS

#endif // APPLETMANAGER_H
