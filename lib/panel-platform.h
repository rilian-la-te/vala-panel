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

#ifndef PANELPOSITIONIFACE_H
#define PANELPOSITIONIFACE_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "lib/constants.h"
#include "lib/settings-manager.h"

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(ValaPanelPlatform, vala_panel_platform, VALA_PANEL, PLATFORM, GObject)

typedef enum {
	ALIGN_START  = 0,
	ALIGN_CENTER = 1,
	ALIGN_END    = 2,
} PanelAlignmentType;

typedef enum {
	XXS  = 16,
	XS   = 22,
	S    = 24,
	M    = 32,
	L    = 48,
	XL   = 96,
	XXL  = 128,
	XXXL = 256
} PanelIconSizeHints;

typedef enum {
	AH_HIDDEN,
	AH_WAITING,
	AH_GRAB,
	AH_VISIBLE,
} PanelAutohideState;

struct _ValaPanelPlatformClass
{
	GObjectClass __parent__;
	/*loading*/
	bool (*start_panels_from_profile)(ValaPanelPlatform *self, GtkApplication *app,
	                                  const char *profile);
	/*struts*/
	bool (*can_strut)(ValaPanelPlatform *f, GtkWindow *top);
	void (*update_strut)(ValaPanelPlatform *f, GtkWindow *top);
	/*positioning requests*/
	void (*move_to_coords)(ValaPanelPlatform *f, GtkWindow *top, int x, int y);
	void (*move_to_side)(ValaPanelPlatform *f, GtkWindow *top, GtkPositionType alloc,
	                     int monitor);
	/*GSettings management*/
	gpointer padding[12];
};

bool vala_panel_platform_start_panels_from_profile(ValaPanelPlatform *self, GtkApplication *app,
                                                   const char *profile);
bool vala_panel_platform_init_settings(ValaPanelPlatform *self, GSettingsBackend *backend);
bool vala_panel_platform_init_settings_full(ValaPanelPlatform *self, const char *schema,
                                            const char *path, GSettingsBackend *backend);
ValaPanelCoreSettings *vala_panel_platform_get_settings(ValaPanelPlatform *self);
bool vala_panel_platform_can_strut(ValaPanelPlatform *f, GtkWindow *top);
void vala_panel_platform_update_strut(ValaPanelPlatform *f, GtkWindow *top);
void vala_panel_platform_move_to_coords(ValaPanelPlatform *f, GtkWindow *top, int x, int y);
void vala_panel_platform_move_to_side(ValaPanelPlatform *f, GtkWindow *top, GtkPositionType alloc,
                                      int monitor);

G_END_DECLS

#endif // PANELPOSITIONIFACE_H
