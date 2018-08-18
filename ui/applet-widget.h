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
GtkWidget *vala_panel_applet_get_background_widget(ValaPanelApplet *self);
void vala_panel_applet_set_background_widget(ValaPanelApplet *self, GtkWidget *w);
void vala_panel_applet_update_context_menu(ValaPanelApplet *self, GMenu *parent_menu);
GSettings *vala_panel_applet_get_settings(ValaPanelApplet *self);
const char *vala_panel_applet_get_uuid(ValaPanelApplet *self);
const char *vala_panel_applet_get_action_group(ValaPanelApplet *self);

G_END_DECLS

#endif // APPLETWIDGET_H
