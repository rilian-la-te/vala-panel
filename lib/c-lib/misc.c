#include <string.h>

#include "lib/definitions.h"
#include "misc.h"
#include "css.h"


static void set_widget_align(GtkWidget* user_data, gpointer data)
{
    if(GTK_IS_WIDGET(user_data))
    {
        gtk_widget_set_halign(GTK_WIDGET(user_data),GTK_ALIGN_FILL);
        gtk_widget_set_valign(GTK_WIDGET(user_data),GTK_ALIGN_FILL);
    }
}

/* Children hierarhy: button => alignment => box => (label,image) */
static void setup_button_notify_connect(GObject* _sender, GParamSpec* b, gpointer self)
{
    GtkButton* a = GTK_BUTTON(_sender);
    if (!strcmp(b->name,"label") || !strcmp(b->name, "image"))
    {
        GtkWidget* w = gtk_bin_get_child(GTK_BIN(a));
        if (GTK_IS_CONTAINER(w))
        {
            GtkWidget* ch;
            if (GTK_IS_BIN(w))
                ch = gtk_bin_get_child(GTK_BIN(w));
            else
                ch = w;
            if (GTK_IS_CONTAINER(ch))
                gtk_container_forall(GTK_CONTAINER(ch),set_widget_align,NULL);
            gtk_widget_set_halign(ch,GTK_ALIGN_FILL);
            gtk_widget_set_valign(ch,GTK_ALIGN_FILL);
        }
    }
}

void vala_panel_setup_button(GtkButton* b, GtkImage* img, char* label)
{
    css_apply_from_resource(GTK_WIDGET(b),"/org/vala-panel/lib/style.css","-panel-button");
    g_signal_connect(b,"notify",setup_button_notify_connect,NULL);
    if (img != NULL)
    {
        gtk_button_set_image(b,GTK_WIDGET(img));
        gtk_button_set_always_show_image(b,TRUE);
    }
    if (label != NULL)
        gtk_button_set_label(b,label);
    gtk_button_set_relief(b,GTK_RELIEF_NONE);
}

inline void vala_panel_apply_window_icon(GtkWindow *win)
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
    g_autofree gchar* str = g_strdup_printf("%d",value);
    gtk_button_set_label(GTK_BUTTON(b),str);
}

void vala_panel_add_prop_as_action(GActionMap* map,const char* prop)
{
    g_autoptr(GAction) action = G_ACTION(g_property_action_new(prop,map,prop));
    g_action_map_add_action(map,action);
}

void vala_panel_add_gsettings_as_action(GActionMap* map, GSettings* settings,const char* prop)
{
    vala_panel_bind_gsettings(map,settings,prop);
    g_autoptr(GAction) action = G_ACTION(g_settings_create_action(settings,prop));
    g_action_map_add_action(map,action);
}
