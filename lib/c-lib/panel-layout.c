#include "panel-layout.h"

struct _ValaPanelAppletLayout
{
	GtkBox __parent__;
	GtkWidget *start;
	GtkWidget *center;
	GtkWidget *end;
};

G_DEFINE_TYPE(ValaPanelAppletLayout, vala_panel_applet_layout, GTK_TYPE_BOX)

static void vala_panel_applet_layout_init(ValaPanelAppletLayout *self)
{
	self->start  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	self->center = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	self->end    = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	g_object_bind_property(self, "orientation", self->start, "orientation", (GBindingFlags)0);
	g_object_bind_property(self, "orientation", self->center, "orientation", (GBindingFlags)0);
	g_object_bind_property(self, "orientation", self->end, "orientation", (GBindingFlags)0);
	gtk_box_pack_start(GTK_BOX(self), self->start, true, true, 0);
	gtk_box_pack_end(GTK_BOX(self), self->end, true, true, 0);
	gtk_box_set_center_widget(GTK_BOX(self), self->center);
}

static void vala_panel_applet_layout_class_init(ValaPanelAppletLayoutClass *klass)
{
}
