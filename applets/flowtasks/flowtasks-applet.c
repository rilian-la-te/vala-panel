/*
 * vala-panel
 * Copyright (C) 2015-2019 Konstantin Pugin <ria.freelander@gmail.com>
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

#include "flowtasks-applet.h"
#include "flowtasks-widget.h"

#define TASKLIST_MIDDLE_CLICK_CLOSE "middle-click-close"
#define TASKLIST_ALL_DESKTOPS "all-desktops"
#define TASKLIST_GROUPING "grouped-tasks"
#define TASKLIST_SHOW_LABELS "show-labels"
#define TASKLIST_SWITCH_UNMIN "switch-workspace-on-unminimize"
#define TASKLIST_UNEXPANDED_LIMIT "unexpanded-limit"

struct _FlowTasksApplet
{
	ValaPanelApplet parent;
	FlowTasksWidget *widget;
};

G_DEFINE_DYNAMIC_TYPE(FlowTasksApplet, flowtasks_applet, vala_panel_applet_get_type())
/*
static void tasklist_settings_changed(GSettings *settings, char *key, void *data)
{
        FlowTasksApplet *self = FLOWTASKS_APPLET(data);
        if (!g_strcmp0(key, TASKLIST_ALL_DESKTOPS))
                xfce_tasklist_set_include_all_workspaces(self->widget,
                                                         g_settings_get_boolean(settings, key));
        if (!g_strcmp0(key, TASKLIST_SWITCH_UNMIN))
                g_object_set(self->widget,
                             TASKLIST_SWITCH_UNMIN,
                             g_settings_get_boolean(settings, key),
                             NULL);
        if (!g_strcmp0(key, TASKLIST_GROUPING))
                xfce_tasklist_set_grouping(self->widget,
                                           g_settings_get_boolean(settings, key)
                                               ? XFCE_TASKLIST_GROUPING_ALWAYS
                                               : XFCE_TASKLIST_GROUPING_NEVER);
        if (!g_strcmp0(key, TASKLIST_MIDDLE_CLICK_CLOSE))
                g_object_set(self->widget,
                             "middle-click",
                             g_settings_get_boolean(settings, key)
                                 ? XFCE_TASKLIST_MIDDLE_CLICK_CLOSE_WINDOW
                                 : XFCE_TASKLIST_MIDDLE_CLICK_NOTHING,
                             NULL);
        if (!g_strcmp0(key, TASKLIST_SHOW_LABELS))
                xfce_tasklist_set_show_labels(self->widget, g_settings_get_boolean(settings, key));
}

static void tasklist_notify_orientation_connect(GObject *topo, GParamSpec *pspec, void *data)
{
        ValaPanelToplevel *top = VALA_PANEL_TOPLEVEL(topo);
        if (!XFCE_IS_TASKLIST(data))
                return;
        XfceTasklist *self = XFCE_TASKLIST(data);
        GtkOrientation orient;
        ValaPanelGravity gravity;
        if (!g_strcmp0(pspec->name, VALA_PANEL_KEY_ORIENTATION))
        {
                g_object_get(top, VALA_PANEL_KEY_ORIENTATION, &orient, VALA_PANEL_KEY_GRAVITY, &gravity, NULL);
                xfce_tasklist_set_orientation(self, orient);
                xfce_tasklist_update_edge(self, vala_panel_edge_from_gravity(gravity));
        }
}
*/
FlowTasksApplet *flowtasks_applet_new(ValaPanelToplevel *toplevel, GSettings *settings,
                                      const char *uuid)
{
	FlowTasksApplet *self = FLOWTASKS_APPLET(
	    vala_panel_applet_construct(flowtasks_applet_get_type(), toplevel, settings, uuid));
	return self;
}
static void flowtasks_applet_constructed(GObject *obj)
{
	FlowTasksApplet *self       = FLOWTASKS_APPLET(obj);
	ValaPanelApplet *base       = VALA_PANEL_APPLET(self);
	ValaPanelToplevel *toplevel = vala_panel_applet_get_toplevel(base);
	GSettings *settings         = vala_panel_applet_get_settings(base);
	GActionMap *map             = G_ACTION_MAP(vala_panel_applet_get_action_group(base));
	GtkOrientation orient;
	ValaPanelGravity gravity;
	g_object_get(toplevel, VALA_PANEL_KEY_ORIENTATION, &orient, VALA_PANEL_KEY_GRAVITY, &gravity, NULL);
	g_simple_action_set_enabled(
	    G_SIMPLE_ACTION(g_action_map_lookup_action(map, VALA_PANEL_APPLET_ACTION_CONFIGURE)),
	    true);
	FlowTasksWidget *widget = g_object_new(flow_tasks_widget_get_type(), NULL);
	self->widget            = widget;
	/*	g_signal_connect_after(toplevel,
	                               "notify",
	                               G_CALLBACK(tasklist_notify_orientation_connect),
	                               self->widget);
	        xfce_tasklist_set_button_relief(self->widget, GTK_RELIEF_NONE);
	        g_signal_connect(settings, "changed", G_CALLBACK(tasklist_settings_changed), self);
	        xfce_tasklist_set_include_all_workspaces(self->widget,
	                                                 g_settings_get_boolean(settings,
	                                                                        TASKLIST_ALL_DESKTOPS));
	        g_object_set(self->widget,
	                     TASKLIST_SWITCH_UNMIN,
	                     g_settings_get_boolean(settings, TASKLIST_SWITCH_UNMIN),
	                     NULL);
	        xfce_tasklist_set_grouping(self->widget,
	                                   g_settings_get_boolean(settings, TASKLIST_GROUPING)
	                                       ? XFCE_TASKLIST_GROUPING_ALWAYS
	                                       : XFCE_TASKLIST_GROUPING_NEVER);
	        g_object_set(self->widget,
	                     "middle-click",
	                     g_settings_get_boolean(settings, TASKLIST_MIDDLE_CLICK_CLOSE)
	                         ? XFCE_TASKLIST_MIDDLE_CLICK_CLOSE_WINDOW
	                         : XFCE_TASKLIST_MIDDLE_CLICK_NOTHING,
	                     NULL);
	        xfce_tasklist_set_show_labels(self->widget,
	                                      g_settings_get_boolean(settings,
	   TASKLIST_SHOW_LABELS)); xfce_tasklist_set_orientation(self->widget, orient);
	        xfce_tasklist_update_edge(self->widget, vala_panel_edge_from_gravity(gravity));*/
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(widget));
	gtk_widget_show(GTK_WIDGET(widget));
	gtk_widget_show(GTK_WIDGET(self));
}

static GtkWidget *flowtasks_applet_get_settings_ui(ValaPanelApplet *base)
{
	GtkWidget *config = generic_config_widget(vala_panel_applet_get_settings(base),
	                                          _("Show windows from all desktops"),
	                                          TASKLIST_ALL_DESKTOPS,
	                                          CONF_BOOL,
	                                          _("Show window`s workspace on unminimize"),
	                                          TASKLIST_SWITCH_UNMIN,
	                                          CONF_BOOL,
	                                          _("Close windows on middle click"),
	                                          TASKLIST_MIDDLE_CLICK_CLOSE,
	                                          CONF_BOOL,
	                                          _("Group windows when needed"),
	                                          TASKLIST_GROUPING,
	                                          CONF_BOOL,
	                                          _("Show task labels"),
	                                          TASKLIST_SHOW_LABELS,
	                                          CONF_BOOL,
	                                          NULL); /* Configuration/option dialog */
	gtk_widget_show(GTK_WIDGET(config));

	return GTK_WIDGET(config);
}

static void flowtasks_applet_init(G_GNUC_UNUSED FlowTasksApplet *self)
{
}

static void flowtasks_applet_displose(GObject *base)
{
	FlowTasksApplet *self = FLOWTASKS_APPLET(base);
	g_signal_handlers_disconnect_by_data(self, self->widget);
	g_signal_handlers_disconnect_by_data(self, self);
	G_OBJECT_CLASS(flowtasks_applet_parent_class)->dispose(base);
}

static void flowtasks_applet_class_init(FlowTasksAppletClass *klass)
{
	((ValaPanelAppletClass *)klass)->get_settings_ui = flowtasks_applet_get_settings_ui;
	G_OBJECT_CLASS(klass)->constructed               = flowtasks_applet_constructed;
	G_OBJECT_CLASS(klass)->dispose                   = flowtasks_applet_displose;
}

static void flowtasks_applet_class_finalize(G_GNUC_UNUSED FlowTasksAppletClass *klass)
{
}

/*
 * IO Module functions
 */

void g_io_flowtasks_load(GTypeModule *module)
{
	g_return_if_fail(module != NULL);

	flowtasks_applet_register_type(module);

	g_io_extension_point_implement(VALA_PANEL_APPLET_EXTENSION_POINT,
	                               flowtasks_applet_get_type(),
	                               "org.valapanel.flowtasks",
	                               10);
}

void g_io_flowtasks_unload(GIOModule *module)
{
	g_return_if_fail(module != NULL);
}
