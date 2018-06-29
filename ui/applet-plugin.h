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
#ifndef APPLETPLUGIN_H
#define APPLETPLUGIN_H

#include "applet-widget-api.h"
#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VALA_PANEL_APPLET_EXPANDABLE "ValaPanel-Expandable"
#define VALA_PANEL_APPLET_EXCLUSIVE "ValaPanel-Exclusive"
#define VALA_PANEL_APPLET_EXTENSION_POINT "vala-panel-applet-module"

G_DECLARE_DERIVABLE_TYPE(ValaPanelAppletPlugin, vala_panel_applet_plugin, VALA_PANEL, APPLET_PLUGIN,
                         GObject)

#define VALA_PANEL_TYPE_APPLET_PLUGIN vala_panel_applet_plugin_get_type()

struct _ValaPanelAppletPluginClass
{
	ValaPanelApplet *(*get_applet_widget)(ValaPanelToplevel *top, GSettings *settings,
	                                      const char *uuid);
};

ValaPanelApplet *vala_panel_applet_plugin_get_applet_widget(ValaPanelAppletPlugin *self,
                                                            ValaPanelToplevel *top,
                                                            GSettings *settings, const char *uuid);

ValaPanelAppletPlugin *vala_panel_applet_plugin_construct(GType type);

G_END_DECLS

#endif // APPLETPLUGIN_H
