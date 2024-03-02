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
#include "applet-manager.h"
#include "definitions.h"
#include "toplevel.h"

typedef struct
{
	ValaPanelCoreSettings *core_settings;
	ValaPanelAppletManager *manager;
	GHashTable *toplevels;
} ValaPanelPlatformPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(ValaPanelPlatform, vala_panel_platform, G_TYPE_OBJECT)

bool vala_panel_platform_can_strut(ValaPanelPlatform *self, GtkWindow *top)
{
	if (self)
		return VALA_PANEL_PLATFORM_GET_CLASS(self)->can_strut(self, top);
	return false;
}

void vala_panel_platform_update_strut(ValaPanelPlatform *self, GtkWindow *top)
{
	if (self)
		VALA_PANEL_PLATFORM_GET_CLASS(self)->update_strut(self, top);
}

void vala_panel_platform_move_to_side(ValaPanelPlatform *self, GtkWindow *top, ValaPanelGravity alloc,
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
	priv->core_settings = vp_core_settings_new(schema, path, backend);
	return vp_core_settings_init_unit_list(priv->core_settings);
}

const char *vala_panel_platform_get_name(ValaPanelPlatform *self)
{
	if (self)
		return VALA_PANEL_PLATFORM_GET_CLASS(self)->get_name(self);
	return "";
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

GdkMonitor *vala_panel_platform_get_suitable_monitor(GtkWidget *self, int mon)
{
	GdkDisplay *screen   = gtk_widget_get_display(self);
	GdkMonitor *fallback = gdk_display_get_monitor_at_point(screen, 0, 0);
	GdkMonitor *monitor  = NULL;
	if (mon < 0)
		monitor = gdk_display_get_primary_monitor(screen);
	else
		monitor = gdk_display_get_monitor(screen, mon);
	return GDK_IS_MONITOR(monitor) ? monitor : fallback;
}

void vala_panel_platform_register_unit(ValaPanelPlatform *self, GtkWindow *unit)
{
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	g_hash_table_add(priv->toplevels, unit);
}

void vala_panel_platform_unregister_unit(ValaPanelPlatform *self, GtkWindow *unit)
{
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	g_hash_table_remove(priv->toplevels, unit);
}

ValaPanelCoreSettings *vala_panel_platform_get_settings(ValaPanelPlatform *self)
{
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	return priv->core_settings;
}

G_GNUC_INTERNAL ValaPanelAppletManager *vp_platform_get_manager(ValaPanelPlatform *self)
{
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	return priv->manager;
}

bool vala_panel_platform_has_units_loaded(ValaPanelPlatform *self)
{
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	return g_hash_table_size(priv->toplevels);
}

bool vala_panel_platform_edge_available(ValaPanelPlatform *self, GtkWindow *top,
                                        ValaPanelGravity gravity, int monitor)
{
	if (self)
		return VALA_PANEL_PLATFORM_GET_CLASS(self)->edge_available(self,
		                                                           top,
		                                                           gravity,
		                                                           monitor);
	return false;
}

static void vala_panel_platform_init(ValaPanelPlatform *self)
{
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	priv->core_settings = NULL;
	priv->manager       = vp_applet_manager_new();
	priv->toplevels     = g_hash_table_new_full(g_direct_hash,
                                                g_direct_equal,
                                                (GDestroyNotify)gtk_widget_destroy,
                                                NULL);
}

static void vala_panel_platform_finalize(GObject *obj)
{
	ValaPanelPlatform *self = VALA_PANEL_PLATFORM(obj);
	ValaPanelPlatformPrivate *priv =
	    (ValaPanelPlatformPrivate *)vala_panel_platform_get_instance_private(self);
	g_hash_table_unref(priv->toplevels);
	if (priv->core_settings)
		vp_core_settings_free(priv->core_settings);
	g_clear_object(&priv->manager);
	G_OBJECT_CLASS(vala_panel_platform_parent_class)->finalize(obj);
}

static void vala_panel_platform_class_init(ValaPanelPlatformClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = vala_panel_platform_finalize;
}
