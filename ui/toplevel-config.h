/*
 * vala-panel
 * Copyright (C) 2017 Konstantin Pugin <ria.freelander@gmail.com>
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

#ifndef TOPLEVELCONFIG_H
#define TOPLEVELCONFIG_H

#include "toplevel.h"
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelToplevelConfig, vala_panel_toplevel_config, VALA_PANEL,
                     TOPLEVEL_CONFIG, GtkDialog)

struct _ValaPanelToplevelConfig
{
	GtkDialog parent_instance;
	GtkStack *prefs_stack;
	ValaPanelToplevel *_toplevel;
	GtkComboBox *monitors_box;
	GtkListStore *store_monitors;
	GtkSpinButton *spin_iconsize;
	GtkSpinButton *spin_height;
	GtkSpinButton *spin_width;
	GtkSpinButton *spin_corners;
	GtkFontButton *font_selector;
	GtkBox *font_box;
	GtkColorButton *color_background;
	GtkColorButton *color_foreground;
	GtkFileChooserButton *file_background;
	GtkListBox *plugin_list;
	GtkStack *applet_info_stack;
	GtkListBox *listbox_new_applet;
	GtkButton *adding_button;
	GtkButton *up_button;
	GtkButton *down_button;
	GtkButton *about_button;
};

void vala_panel_toplevel_config_select_applet(ValaPanelToplevelConfig *self, const char *uuid);

G_END_DECLS

#endif
