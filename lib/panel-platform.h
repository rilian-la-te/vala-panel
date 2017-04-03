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

G_BEGIN_DECLS

G_DECLARE_INTERFACE(ValaPanelPlatform, vala_panel_platform, VALA_PANEL, PLATFORM, GObject)

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

struct _ValaPanelPlatformInterface
{
	GTypeInterface g_iface;
	/*loading*/
	bool (*start_panels_from_profile)(ValaPanelPlatform *self, GtkApplication *app,
	                                  const char *profile);
	/*struts*/
	long (*can_strut)(ValaPanelPlatform *f, GtkWindow *top);
	void (*update_strut)(ValaPanelPlatform *f, GtkWindow *top);
	/*positioning requests*/
	void (*move_to_coords)(ValaPanelPlatform *f, GtkWindow *top, int x, int y);
	void (*move_to_side)(ValaPanelPlatform *f, GtkWindow *top, GtkPositionType alloc);
	/*GSettings management*/
	GSettings *(*get_settings_for_scheme)(ValaPanelPlatform *self, const char *scheme,
	                                      const char *path);
	void (*remove_settings_path)(ValaPanelPlatform *self, const char *path, const char *name);
	gpointer padding[12];
};

bool vala_panel_platform_start_panels_from_profile(ValaPanelPlatform *self, GtkApplication *app,
                                                   const char *profile);
long vala_panel_platform_can_strut(ValaPanelPlatform *f, GtkWindow *top);
void vala_panel_platform_update_strut(ValaPanelPlatform *f, GtkWindow *top);
void vala_panel_platform_move_to_coords(ValaPanelPlatform *f, GtkWindow *top, int x, int y);
void vala_panel_platform_move_to_side(ValaPanelPlatform *f, GtkWindow *top, GtkPositionType alloc);
GSettings *vala_panel_platform_get_settings_for_scheme(ValaPanelPlatform *self, const char *scheme,
                                                       const char *path);
void vala_panel_platform_remove_settings_path(ValaPanelPlatform *self, const char *path,
                                              const char *child_name);

G_END_DECLS

#endif // PANELPOSITIONIFACE_H
