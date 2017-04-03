#ifndef APPLETWIDGET_H
#define APPLETWIDGET_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(ValaPanelAppletWidget, vala_panel_applet_widget, VALA_PANEL, APPLET_WIDGET,
                         GtkBin)

struct _ValaPanelAppletWidgetClass
{
	GObjectClass parent_class;
	//	void (*update_popup)(ValaPanelAppletWidget *self, ValaPanelPopupManager *mgr);
	void (*invoke_applet_action)(ValaPanelAppletWidget *self, const gchar *action,
	                             GVariantDict *params);
	GtkWidget *(*get_settings_ui)(ValaPanelAppletWidget *self);
	gpointer padding[12];
};

// void vala_panel_applet_widget_update_popup(ValaPanelAppletWidget *self, ValaPanelPopupManager
// *mgr);
void vala_panel_applet_widget_invoke_applet_action(ValaPanelAppletWidget *self, const char *action,
                                                   GVariantDict *param);
GtkWidget *vala_panel_applet_widget_get_settings_ui(ValaPanelAppletWidget *self);

G_END_DECLS

#endif // APPLETWIDGET_H
