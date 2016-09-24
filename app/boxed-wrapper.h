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
