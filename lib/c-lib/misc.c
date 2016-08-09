#include "misc.h"


void vala_panel_apply_window_icon(GtkWindow *win)
{
    g_autoptr(GdkPixbuf) icon;
    icon = gdk_pixbuf_new_from_resource("/org/vala-panel/lib/panel.png",NULL);
    gtk_window_set_icon(win,icon);
}

void vala_panel_scale_button_set_range (GtkScaleButton* b, gint lower, gint upper)
{
    gtk_adjustment_set_lower(gtk_scale_button_get_adjustment(b),lower);
    gtk_adjustment_set_upper(gtk_scale_button_get_adjustment(b),upper);
    gtk_adjustment_set_step_increment(gtk_scale_button_get_adjustment(b),1);
    gtk_adjustment_set_page_increment(gtk_scale_button_get_adjustment(b),1);
}

void vala_panel_scale_button_set_value_labeled (GtkScaleButton* b, gint value)
{
    gtk_scale_button_set_value(b,value);
    gchar* str = g_strdup_printf("%d",value);
    gtk_button_set_label(GTK_BUTTON(b),str);
    g_free(str);
}

void vala_panel_add_prop_as_action(GActionMap* map,const char* prop)
{
    GAction* action;
    action = G_ACTION(g_property_action_new(prop,map,prop));
    g_action_map_add_action(map,action);
    g_object_unref(action);
}

void vala_panel_add_gsettings_as_action(GActionMap* map, GSettings* settings,const char* prop)
{
    GAction* action;
    g_settings_bind(settings,prop,G_OBJECT(map),prop,G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET |G_SETTINGS_BIND_DEFAULT);
    action = G_ACTION(g_settings_create_action(settings,prop));
    g_action_map_add_action(map,action);
    g_object_unref(action);
}

void vala_panel_bind_gsettings(GObject* obj, GSettings* settings, const gchar* prop)
{
    g_settings_bind(settings,prop,obj,prop,G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET | G_SETTINGS_BIND_DEFAULT);
}
