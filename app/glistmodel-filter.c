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

#include "glistmodel-filter.h"
#include "boxed-wrapper.h"
#include "info-data.h"

struct _ValaPanelListModelFilter
{
	GObject __parent__;
	GListModel *base_model;
	ValaPanelListModelFilterFunc filter_func;
	gpointer user_data;
	uint max_results;
	uint filter_matches;
};

enum
{
	PROP_DUMMY,
	PROP_BASE_MODEL,
	N_PROPERTIES
};

static void g_list_model_iface_init(GListModelInterface *iface);

G_DEFINE_TYPE_WITH_CODE(ValaPanelListModelFilter, vala_panel_list_model_filter, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, g_list_model_iface_init))

static GType vala_panel_list_model_filter_get_item_type(GListModel *lst)
{
	ValaPanelListModelFilter *self = VALA_PANEL_LIST_MODEL_FILTER(lst);
	return g_list_model_get_item_type(self->base_model);
}

static uint vala_panel_list_model_filter_get_n_items(GListModel *lst)
{
	ValaPanelListModelFilter *self = VALA_PANEL_LIST_MODEL_FILTER(lst);
	return self->filter_matches > self->max_results ? self->max_results : self->filter_matches;
}

static gpointer vala_panel_list_model_filter_get_item(GListModel *lst, uint pos)
{
	ValaPanelListModelFilter *self = VALA_PANEL_LIST_MODEL_FILTER(lst);
	gpointer item                  = NULL;
	if (pos > self->max_results && pos != (uint)-1)
		return NULL;
	int n_items_base = (int)g_list_model_get_n_items(self->base_model);
	for (int i = 0, counter = 0; (i < n_items_base) && (counter <= (int)pos); i++)
	{
		item = g_list_model_get_item(self->base_model, (uint)i);
		if (self->filter_func(item, self->user_data))
			counter++;
	}
	BoxedWrapper *wr = boxed_wrapper_new(g_list_model_get_item_type(self->base_model));
	boxed_wrapper_set_boxed(wr, item);
	return wr;
}

static void vala_panel_list_model_filter_get_property(GObject *object, guint property_id,
                                                      GValue *value, GParamSpec *pspec)
{
	ValaPanelListModelFilter *self = VALA_PANEL_LIST_MODEL_FILTER(object);
	switch (property_id)
	{
	case PROP_BASE_MODEL:
	{
		g_value_set_object(value, self->base_model);
		break;
	}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void vala_panel_filter_base_changed(GListModel *list, guint position, guint removed,
                                           guint added, gpointer user_data)
{
	ValaPanelListModelFilter *self = VALA_PANEL_LIST_MODEL_FILTER(user_data);
	vala_panel_list_model_filter_invalidate(self);
}

static void vala_panel_list_model_filter_set_property(GObject *object, guint property_id,
                                                      const GValue *value, GParamSpec *pspec)
{
	ValaPanelListModelFilter *self = VALA_PANEL_LIST_MODEL_FILTER(object);
	switch (property_id)
	{
	case PROP_BASE_MODEL:
	{
		GListModel *mdl  = G_LIST_MODEL(g_value_get_object(value));
		self->base_model = mdl;
		g_signal_connect(mdl,
		                 "items-changed",
		                 (GCallback)vala_panel_filter_base_changed,
		                 self);
		break;
	}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void g_list_model_iface_init(GListModelInterface *iface)
{
	iface->get_item_type = vala_panel_list_model_filter_get_item_type;
	iface->get_item      = vala_panel_list_model_filter_get_item;
	iface->get_n_items   = vala_panel_list_model_filter_get_n_items;
}

static void vala_panel_list_model_filter_init(ValaPanelListModelFilter *self)
{
}

static void vala_panel_list_model_filter_class_init(ValaPanelListModelFilterClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = vala_panel_list_model_filter_set_property;
	object_class->get_property = vala_panel_list_model_filter_get_property;

	g_object_class_install_property(object_class,
	                                PROP_BASE_MODEL,
	                                g_param_spec_object("base-model",
	                                                    "",
	                                                    "",
	                                                    G_TYPE_LIST_MODEL,
	                                                    G_PARAM_CONSTRUCT_ONLY |
	                                                        G_PARAM_READWRITE |
	                                                        G_PARAM_STATIC_STRINGS));
}

void vala_panel_list_model_filter_set_max_results(ValaPanelListModelFilter *self, uint max_results)
{
	self->max_results = max_results;
}

void vala_panel_list_model_filter_set_filter_func(ValaPanelListModelFilter *self,
                                                  ValaPanelListModelFilterFunc func,
                                                  gpointer user_data)
{
	self->filter_func = func;
	self->user_data   = user_data;
}

void vala_panel_list_model_filter_invalidate(ValaPanelListModelFilter *self)
{
	uint old_matches     = self->filter_matches;
	self->filter_matches = 0;
	for (int i = 0; ((i < (int)g_list_model_get_n_items(self->base_model)) &&
	                 (self->filter_matches < self->max_results));
	     i++)
	{
		gpointer item = g_list_model_get_item(self->base_model, i);
		if (self->filter_func(item, self->user_data))
			self->filter_matches++;
	}
	g_list_model_items_changed(G_LIST_MODEL(self), 0, old_matches, self->filter_matches);
}

ValaPanelListModelFilter *vala_panel_list_model_filter_new(GListModel *base_model)
{
	return VALA_PANEL_LIST_MODEL_FILTER(
	    g_object_new(vala_panel_list_model_filter_get_type(), "base-model", base_model, NULL));
}
