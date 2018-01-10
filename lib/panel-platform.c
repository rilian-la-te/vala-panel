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
#include "definitions.h"

typedef struct
{
	ValaPanelCoreSettings *core_settings;
} ValaPanelPlatformPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(ValaPanelPlatform, vala_panel_platform, G_TYPE_OBJECT)

bool vala_panel_platform_can_strut(ValaPanelPlatform *self, GtkWindow *top)
{
	if (self)
		return VALA_PANEL_PLATFORM_GET_CLASS(self)->can_strut(self, top);
	return -1;
}

void vala_panel_platform_update_strut(ValaPanelPlatform *self, GtkWindow *top)
{
	if (self)
		VALA_PANEL_PLATFORM_GET_CLASS(self)->update_strut(self, top);
}

void vala_panel_platform_move_to_coords(ValaPanelPlatform *self, GtkWindow *top, int x, int y)
{
	if (self)
		VALA_PANEL_PLATFORM_GET_CLASS(self)->move_to_coords(self, top, x, y);
}

void vala_panel_platform_move_to_side(ValaPanelPlatform *self, GtkWindow *top, PanelGravity alloc,
                                      int monitor)
{
	if (self)
		VALA_PANEL_PLATFORM_GET_CLASS(self)->move_to_side(self, top, alloc, monitor);
}

bool vala_panel_platform_init_settings(ValaPanelPlatform *self, GSettingsBackend *backend)
{
	return vala_panel_platform_init_settings_full(self,
	                                              VALA_PANEL_BASE_SCHEMA,
	                                              VALA_PANEL_OBJECT_PATH,
	                                              backend);
}

bool vala_panel_platform_init_settings_full(ValaPanelPlatform *self, const char *schema,
                                            const char *path, GSettingsBackend *backend)
{
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	priv->core_settings = vala_panel_core_settings_new(schema, path, backend);
	return vala_panel_core_settings_init_unit_list(priv->core_settings);
}

bool vala_panel_platform_start_panels_from_profile(ValaPanelPlatform *self, GtkApplication *app,
                                                   const char *profile)
{
	if (self)
		return VALA_PANEL_PLATFORM_GET_CLASS(self)->start_panels_from_profile(self,
		                                                                      app,
		                                                                      profile);
	return false;
}

ValaPanelCoreSettings *vala_panel_platform_get_settings(ValaPanelPlatform *self)
{
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	return priv->core_settings;
}

void vala_panel_platform_init(ValaPanelPlatform *self)
{
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	priv->core_settings = NULL;
}

void vala_panel_platform_finalize(GObject *obj)
{
	ValaPanelPlatform *self = VALA_PANEL_PLATFORM(obj);
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	if (priv->core_settings)
		vala_panel_core_settings_free(priv->core_settings);
}

void vala_panel_platform_class_init(ValaPanelPlatformClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = vala_panel_platform_finalize;
}

bool vala_panel_platform_edge_available(ValaPanelPlatform *self, GtkWindow *top,
                                        PanelGravity gravity, int monitor)
{
	if (self)
		return VALA_PANEL_PLATFORM_GET_CLASS(self)->edge_available(self,
		                                                           top,
		                                                           gravity,
		                                                           monitor);
	return false;
}
