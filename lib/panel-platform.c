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

#include "panel-platform.h"

G_DEFINE_INTERFACE(ValaPanelPlatform, vala_panel_platform, G_TYPE_OBJECT)

void vala_panel_platform_default_init(ValaPanelPlatformInterface *self)
{
}

long vala_panel_platform_can_strut(ValaPanelPlatform *self, GtkWindow *top)
{
	if (self)
		return VALA_PANEL_PLATFORM_GET_IFACE(self)->can_strut(self, top);
	return -1;
}

void vala_panel_platform_update_strut(ValaPanelPlatform *self, GtkWindow *top)
{
	if (self)
		VALA_PANEL_PLATFORM_GET_IFACE(self)->update_strut(self, top);
}

void vala_panel_platform_move_to_coords(ValaPanelPlatform *self, GtkWindow *top, int x, int y)
{
	if (self)
		VALA_PANEL_PLATFORM_GET_IFACE(self)->move_to_coords(self, top, x, y);
}

void vala_panel_platform_move_to_side(ValaPanelPlatform *self, GtkWindow *top,
                                      GtkPositionType alloc)
{
	if (self)
		VALA_PANEL_PLATFORM_GET_IFACE(self)->move_to_side(self, top, alloc);
}

bool vala_panel_platform_start_panels_from_profile(ValaPanelPlatform *self, GtkApplication *app,
                                                   const char *profile)
{
	if (self)
		return VALA_PANEL_PLATFORM_GET_IFACE(self)->start_panels_from_profile(self,
		                                                                      app,
		                                                                      profile);
	return false;
}

GSettings *vala_panel_platform_get_settings_for_scheme(ValaPanelPlatform *self, const char *scheme,
                                                       const char *path)
{
	if (self)
		return VALA_PANEL_PLATFORM_GET_IFACE(self)->get_settings_for_scheme(self,
		                                                                    scheme,
		                                                                    path);
	return NULL;
}

void vala_panel_platform_remove_settings_path(ValaPanelPlatform *self, const char *path,
                                              const char *child_name)
{
	if (self)
		VALA_PANEL_PLATFORM_GET_IFACE(self)->remove_settings_path(self, path, child_name);
	return;
}
