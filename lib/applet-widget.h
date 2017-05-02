#ifndef APPLETWIDGET_H
#define APPLETWIDGET_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(ValaPanelApplet, vala_panel_applet, VALA_PANEL, APPLET,
                         GtkBin)

struct _ValaPanelAppletClass
{
        GObjectClass parent_class;
        //	void (*update_popup)(ValaPanelAppletWidget *self, ValaPanelPopupManager *mgr);
        void (*show_menu)(GSimpleAction *act, GVariant *param,
                                     gpointer *params);
        GtkWidget *(*get_config_dialog)(ValaPanelApplet *self);
        void (*update_context_menu)(GMenu* parent_menu);
        void (*set_actions)();
        gpointer padding[12];
};

// void vala_panel_applet_widget_update_popup(ValaPanelAppletWidget *self, ValaPanelPopupManager
// *mgr);
void vala_panel_applet_widget_invoke_applet_action(ValaPanelApplet *self, const char *action,
                                                   GVariantDict *param);
GtkWidget *vala_panel_applet_widget_get_settings_ui(ValaPanelApplet *self);

G_END_DECLS

#endif // APPLETWIDGET_H
