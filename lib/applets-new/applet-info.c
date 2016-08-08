#include "lib/definitions.h"
#include "applet-info.h"
#include "applet-widget.h"

#include <gtk/gtk.h>

struct _ValaPanelAppletInfo
{
    ValaPanelAppletWidget* applet;
    GSettings* settings;
    gchar* icon;
    gchar* applet_type;
    gchar* name;
    gchar* description;
    gchar* uuid;
    GtkAlign alignment;
    gint position;
    gboolean expand;
};

G_DEFINE_TYPE(ValaPanelAppletInfo,vala_panel_applet_info,G_TYPE_OBJECT)

void vala_panel_applet_info_class_init(ValaPanelAppletInfoClass* klass)
{

}

void vala_panel_applet_info_init(ValaPanelAppletInfo* self)
{

}
