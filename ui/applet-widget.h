#ifndef APPLETWIDGET_H
#define APPLETWIDGET_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define VP_APPLET_ACTION_REMOTE "remote"
#define VP_APPLET_ACTION_CONFIGURE "configure"
#define VP_APPLET_ACTION_REMOVE "remove"
#define VP_APPLET_ACTION_ABOUT "about"

G_DECLARE_DERIVABLE_TYPE(ValaPanelApplet, vp_applet, VALA_PANEL, APPLET, GtkBin)
#define VALA_PANEL_TYPE_APPLET vp_applet_get_type()

struct _ValaPanelAppletClass
{
	GtkBinClass parent_class;
	bool (*remote_command)(ValaPanelApplet *self, const char *command);
	GtkWidget *(*get_settings_ui)(ValaPanelApplet *self);
	void (*update_context_menu)(ValaPanelApplet *self, GMenu *parent_menu);
	gpointer padding[12];
};

bool vp_applet_remote_command(ValaPanelApplet *self, const char *command);
bool vp_applet_is_configurable(ValaPanelApplet *self);
void vp_applet_init_background(ValaPanelApplet *self);
void vp_applet_show_config_dialog(ValaPanelApplet *self);
GtkWidget *vp_applet_get_settings_ui(ValaPanelApplet *self);
GtkWidget *vp_applet_get_background_widget(ValaPanelApplet *self);
void vp_applet_set_background_widget(ValaPanelApplet *self, GtkWidget *w);
void vp_applet_update_context_menu(ValaPanelApplet *self, GMenu *parent_menu);
GSettings *vp_applet_get_settings(ValaPanelApplet *self);
const char *vp_applet_get_uuid(ValaPanelApplet *self);
GActionMap *vp_applet_get_action_group(ValaPanelApplet *self);

G_END_DECLS

#endif // APPLETWIDGET_H
