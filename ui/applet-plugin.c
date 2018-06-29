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
#include "applet-plugin.h"

G_DEFINE_ABSTRACT_TYPE(ValaPanelAppletPlugin, vala_panel_applet_plugin, G_TYPE_OBJECT)

static void vala_panel_applet_plugin_init(ValaPanelAppletPlugin *self)
{
}

static void vala_panel_applet_plugin_class_init(ValaPanelAppletPluginClass *klass)
{
}

ValaPanelApplet *vala_panel_applet_plugin_get_applet_widget(ValaPanelAppletPlugin *self,
                                                            ValaPanelToplevel *top,
                                                            GSettings *settings, const char *uuid)
{
	if (self)
		return VALA_PANEL_APPLET_PLUGIN_GET_CLASS(self)->get_applet_widget(self,
		                                                                   top,
		                                                                   settings,
		                                                                   uuid);
	return NULL;
}

ValaPanelAppletPlugin *vala_panel_applet_plugin_construct(GType type)
{
	return VALA_PANEL_APPLET_PLUGIN(g_object_new(type, NULL));
}
