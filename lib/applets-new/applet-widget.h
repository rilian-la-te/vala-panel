#ifndef APPLETWIDGET_H
#define APPLETWIDGET_H

#include <glib.h>
#include <glib-object.h>
#include "popup-manager.h"

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(ValaPanelAppletWidget,vala_panel_applet_widget,VALA_PANEL,APPLET_WIDGET,GObject)

struct _ValaPanelAppletWidgetClass
{
    GObjectClass parent_class;
    void (*update_popup) (ValaPanelAppletWidget* self, ValaPanelPopupManager* mgr);
    void (*invoke_applet_action) (ValaPanelAppletWidget* self, const gchar* action, GVariantDict* params);
    GtkWidget* (*get_settings_ui) (ValaPanelAppletWidget* self);
    GSettings* (*get_applet_settings) (ValaPanelAppletWidget* self);
    gpointer padding[12];
};

G_END_DECLS

#endif // APPLETWIDGET_H
