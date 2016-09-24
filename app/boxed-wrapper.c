#include "boxed-wrapper.h"

struct _BoxedWrapper
{
	GObject __parent__;
	gpointer boxed;
	GType boxed_type;
};

G_DEFINE_TYPE(BoxedWrapper, boxed_wrapper, G_TYPE_OBJECT)

static void boxed_wrapper_finalize(BoxedWrapper *self)
{
	if (self->boxed_type && self->boxed)
		g_boxed_free(self->boxed_type, self->boxed);
}

static void boxed_wrapper_init(BoxedWrapper *self)
{
	self->boxed      = NULL;
	self->boxed_type = G_TYPE_NONE;
}

static void boxed_wrapper_class_init(BoxedWrapperClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = boxed_wrapper_finalize;
}

BoxedWrapper *boxed_wrapper_new(GType boxed_type)
{
	BoxedWrapper *wr = VALA_PANEL_BOXED_WRAPPER(g_object_new(boxed_wrapper_get_type(), NULL));
	wr->boxed_type   = boxed_type;
	return wr;
}

gconstpointer boxed_wrapper_get_boxed(const BoxedWrapper *self)
{
	return self->boxed;
}

void boxed_wrapper_set_boxed(BoxedWrapper *self, gconstpointer boxed)
{
	if (self->boxed)
		g_boxed_free(self->boxed_type, self->boxed);
	self->boxed = g_boxed_copy(self->boxed_type, boxed);
}

gpointer boxed_wrapper_dup_boxed(const BoxedWrapper *self)
{
	return g_boxed_copy(self->boxed_type, self->boxed);
}
