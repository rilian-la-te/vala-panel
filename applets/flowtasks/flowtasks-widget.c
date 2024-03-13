/*
 * vala-panel
 * Copyright (C) 2020 Konstantin Pugin <ria.freelander@gmail.com>
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

#include "client.h"

#include "flowtasks-backend-wnck.h"
#include "flowtasks-widget.h"
#include "task-model.h"
#include "task.h"

struct _FlowTasksWidget
{
	GtkFlowBox parent;
	ValaPanelTaskModel *model;
};

G_DEFINE_TYPE(FlowTasksWidget, flow_tasks_widget, GTK_TYPE_FLOW_BOX);

static void flow_tasks_widget_init(FlowTasksWidget *self)
{
}

GtkWidget *flow_tasks_widget_func(gpointer item, gpointer user_data)
{
	ValaPanelTask *task = VALA_PANEL_TASK(item);
	GtkWidget *widget   = gtk_flow_box_child_new();
	GtkWidget *image    = gtk_image_new();
	GtkWidget *label    = gtk_label_new("");
	GtkWidget *box      = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	g_object_bind_property(task, VT_KEY_TITLE, label, "label", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
	g_object_bind_property(task, VT_KEY_ICON, image, "gicon", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
	g_object_bind_property(task, VT_KEY_TOOLTIP, widget, "tooltip-markup", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
	gtk_box_pack_start(GTK_BOX(box), image, false, true, 0);
	gtk_box_pack_start(GTK_BOX(box), label, false, true, 0);
	gtk_container_add(GTK_CONTAINER(widget), box);
	gtk_widget_show_all(widget);
	return widget;
}

static void flow_tasks_widget_constructed(GObject *obj)
{
	g_return_if_fail(FLOW_TASKS_IS_WIDGET(obj));
	FlowTasksWidget *self = FLOW_TASKS_WIDGET(obj);
	const char *plt       = vala_panel_get_current_platform_name();
	if (!g_strcmp0(plt, "x11"))
	{
		self->model = VALA_PANEL_TASK_MODEL(g_object_new(wnck_task_model_get_type(), NULL));
	}
	else
		g_warning("Platform is not supported. Desktop file is broken.");
	gtk_flow_box_bind_model(GTK_FLOW_BOX(self),
						G_LIST_MODEL(self->model),
						flow_tasks_widget_func,
						self,
						NULL);
	G_OBJECT_CLASS(flow_tasks_widget_parent_class)->constructed(obj);
}

static void flow_tasks_widget_dispose(GObject *obj)
{
	g_return_if_fail(FLOW_TASKS_IS_WIDGET(obj));
	FlowTasksWidget *self = FLOW_TASKS_WIDGET(obj);
	g_object_unref(self->model);
	G_OBJECT_CLASS(flow_tasks_widget_parent_class)->dispose(obj);
}

static void flow_tasks_widget_class_init(FlowTasksWidgetClass *klass)
{
	GObjectClass *oclass = G_OBJECT_CLASS(klass);
	oclass->constructed  = flow_tasks_widget_constructed;
	// 	oclass->set_property = vala_panel_task_model_set_property;
	// 	oclass->get_property = vala_panel_task_model_get_property;
	oclass->dispose = flow_tasks_widget_dispose;
	// 	oclass->finalize     = vala_panel_task_model_finalize;
}
