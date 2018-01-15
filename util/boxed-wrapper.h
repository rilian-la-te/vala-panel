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

#ifndef BOXEDWRAPPER_H
#define BOXEDWRAPPER_H

#include <glib-object.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(BoxedWrapper, boxed_wrapper, VALA_PANEL, BOXED_WRAPPER, GObject)

BoxedWrapper *boxed_wrapper_new(GType boxed_type);
gconstpointer boxed_wrapper_get_boxed(const BoxedWrapper *self);
gpointer boxed_wrapper_dup_boxed(const BoxedWrapper *self);
void boxed_wrapper_set_boxed(BoxedWrapper *self, gconstpointer boxed);

G_END_DECLS

#endif // BOXEDWRAPPER_H
