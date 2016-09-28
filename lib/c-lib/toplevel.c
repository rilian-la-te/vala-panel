#include "toplevel.h"
#include "panel-layout.h"
#include "panel-manager.h"

struct _ValaPanelToplevelUnit
{
	GtkApplicationWindow __parent__;
	ValaPanelAppletManager *manager;
	ValaPanelAppletLayout *layout;
	GSettings *toplevel_setings;
	bool initialized;
	bool dock;
	bool autohide;
	char *uid;
	int height;
	int widgth;
	GtkOrientation orientation;
	GtkPositionType edge;
	GtkDialog *pref_dialog;
};

G_DEFINE_TYPE(ValaPanelToplevelUnit, vala_panel_toplevel_unit, GTK_TYPE_APPLICATION_WINDOW)

static void stop_ui(ValaPanelToplevelUnit *self)
{
	if (self->autohide)
		vala_panel_manager_ah_stop(vala_panel_applet_manager_get_manager(self->manager),
		                           self);
	if (self->pref_dialog != NULL)
		gtk_dialog_response(self->pref_dialog, GTK_RESPONSE_CLOSE);
	if (self->initialized)
	{
		gdk_flush();
		self->initialized = false;
	}
	if (gtk_bin_get_child(GTK_BIN(self)))
	{
		gtk_widget_hide(GTK_WIDGET(self->layout));
	}
}

static void start_ui(ValaPanelToplevelUnit *self)
{
	//    a.x = a.y = a.width = a.height = 0;
	gtk_window_set_wmclass(GTK_WINDOW(self), "panel", "vala-panel");
	gtk_application_add_window(gtk_window_get_application(GTK_WINDOW(self)), GTK_WINDOW(self));
	gtk_widget_add_events(GTK_WIDGET(self), GDK_BUTTON_PRESS_MASK);
	gtk_widget_realize(GTK_WIDGET(self));
	// Move this to init, layout must not be reinit in start/stop UI
	self->layout = vala_panel_applet_layout_new(self->orientation, 0);
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(self->layout));
	gtk_widget_show(GTK_WIDGET(self->layout));
	gtk_window_set_type_hint(GTK_WINDOW(self),
	                         (self->dock) ? GDK_WINDOW_TYPE_HINT_DOCK
	                                      : GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_widget_show(GTK_WIDGET(self));
	gtk_window_stick(GTK_WINDOW(self));
	vala_panel_applet_layout_load_applets(self->layout, self->manager, self->toplevel_setings);
	gtk_window_present(GTK_WINDOW(self));
	self->autohide    = g_settings_get_boolean(self->toplevel_setings, VALA_PANEL_KEY_AUTOHIDE);
	self->initialized = true;
}

void vala_panel_toplevel_unit_init(ValaPanelToplevelUnit *self)
{
}

void vala_panel_toplevel_unit_class_init(ValaPanelToplevelUnitClass *parent)
{
}
