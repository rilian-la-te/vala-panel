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

#include "boxed-wrapper.h"

struct _ValaPanelBoxedWrapper
{
	GObject __parent__;
	gpointer boxed;
	GType boxed_type;
};

G_DEFINE_TYPE(ValaPanelBoxedWrapper, vala_panel_boxed_wrapper, G_TYPE_OBJECT)

static void vala_panel_boxed_wrapper_finalize(GObject *obj)
{
	ValaPanelBoxedWrapper *self = VALA_PANEL_BOXED_WRAPPER(obj);
	if (self->boxed_type && self->boxed)
		g_boxed_free(self->boxed_type, self->boxed);
	G_OBJECT_CLASS(vala_panel_boxed_wrapper_parent_class)->finalize(obj);
}

static void vala_panel_boxed_wrapper_init(ValaPanelBoxedWrapper *self)
{
	self->boxed      = NULL;
	self->boxed_type = G_TYPE_NONE;
}

static void vala_panel_boxed_wrapper_class_init(ValaPanelBoxedWrapperClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = vala_panel_boxed_wrapper_finalize;
}

ValaPanelBoxedWrapper *vala_panel_boxed_wrapper_new(GType boxed_type)
{
	ValaPanelBoxedWrapper *wr = VALA_PANEL_BOXED_WRAPPER(g_object_new(vala_panel_boxed_wrapper_get_type(), NULL));
	wr->boxed_type   = boxed_type;
	return wr;
}

gconstpointer vala_panel_boxed_wrapper_get_boxed(const ValaPanelBoxedWrapper *self)
{
	return self->boxed;
}

void vala_panel_boxed_wrapper_set_boxed(ValaPanelBoxedWrapper *self, gconstpointer boxed)
{
	if (self->boxed)
		g_boxed_free(self->boxed_type, self->boxed);
	self->boxed = g_boxed_copy(self->boxed_type, boxed);
}

gpointer vala_panel_boxed_wrapper_dup_boxed(const ValaPanelBoxedWrapper *self)
{
	return g_boxed_copy(self->boxed_type, self->boxed);
}
