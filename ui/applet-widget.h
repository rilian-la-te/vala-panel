#ifndef APPLETWIDGET_H
#define APPLETWIDGET_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define VALA_PANEL_APPLET_ACTION_REMOTE "remote"
#define VALA_PANEL_APPLET_ACTION_CONFIGURE "configure"
#define VALA_PANEL_APPLET_ACTION_REMOVE "remove"
#define VALA_PANEL_APPLET_ACTION_ABOUT "about"

G_DECLARE_DERIVABLE_TYPE(ValaPanelApplet, vala_panel_applet, VALA_PANEL, APPLET, GtkBin)
#define VALA_PANEL_TYPE_APPLET vala_panel_applet_get_type()

struct _ValaPanelAppletClass
{
	GtkBinClass parent_class;
	bool (*remote_command)(ValaPanelApplet *self, const char *command);
	GtkWidget *(*get_settings_ui)(ValaPanelApplet *self);
	void (*update_context_menu)(ValaPanelApplet *self, GMenu *parent_menu);
	gpointer padding[12];
};

bool vala_panel_applet_remote_command(ValaPanelApplet *self, const char *command);
bool vala_panel_applet_is_configurable(ValaPanelApplet *self);
void vala_panel_applet_init_background(ValaPanelApplet *self);
void vala_panel_applet_show_config_dialog(ValaPanelApplet *self);
/**
 * vala_panel_applet_get_settings_ui: (virtual get_settings_ui)
 * @self: a #ValaPanelApplet
 * 
 * Returns: (transfer full): Settings widget
 */
GtkWidget *vala_panel_applet_get_settings_ui(ValaPanelApplet *self);
/**
 * vala_panel_applet_get_background_widget: (get-property background-widget)
 * @self: a #ValaPanelApplet
 * 
 * Returns: (transfer none): #GtkWidget widget (which serves as background now)
 */
GtkWidget *vala_panel_applet_get_background_widget(ValaPanelApplet *self);
/**
 * vala_panel_applet_set_background_widget: (set-property background-widget)
 * @self: a #ValaPanelApplet
 * @w: (in) (not nullable): #GtkWidget which will be set as background one
 * 
 */
void vala_panel_applet_set_background_widget(ValaPanelApplet *self, GtkWidget *w);
void vala_panel_applet_update_context_menu(ValaPanelApplet *self, GMenu *parent_menu);
/**
 * vala_panel_applet_get_settings: (get-property settings)
 * @self: a #ValaPanelApplet
 * 
 * Returns: (transfer none): #GSettings instance of this applet
 */
GSettings *vala_panel_applet_get_settings(ValaPanelApplet *self);
const char *vala_panel_applet_get_uuid(ValaPanelApplet *self);
/**
 * vala_panel_applet_get_action_group: (get-property action-group)
 * @self: a #ValaPanelApplet
 * 
 * Returns: (transfer none): #GActionMap instance of this applet
 */
GActionMap *vala_panel_applet_get_action_group(ValaPanelApplet *self);

G_END_DECLS

#endif // APPLETWIDGET_H
