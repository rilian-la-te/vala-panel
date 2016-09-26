#include "panel-layout.h"

struct _ValaPanelAppletLayout
{
	GtkBox __parent__;
	GtkWidget *left;
	GtkWidget *center;
	GtkWidget *right;
};

G_DEFINE_TYPE(ValaPanelAppletLayout, vala_panel_applet_layout, GTK_TYPE_BOX)

static void vala_panel_applet_layout_init(ValaPanelAppletLayout *self)
{
	self->left   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	self->center = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	self->right  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	g_object_bind_property(self, "orientation", self->left, "orientation", (GBindingFlags)0);
	g_object_bind_property(self, "orientation", self->center, "orientation", (GBindingFlags)0);
	g_object_bind_property(self, "orientation", self->right, "orientation", (GBindingFlags)0);
}

static void vala_panel_applet_layout_class_init(ValaPanelAppletLayoutClass *klass)
{
}
