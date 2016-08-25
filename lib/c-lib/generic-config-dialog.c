#include <glib/gi18n.h>
#include <stdarg.h>

#include "generic-config-dialog.h"
#include "lib/c-lib/misc.h"

typedef struct {
        GSettings *settings;
        const char *key;
} SignalData;

static void set_file_response(GtkFileChooserButton *widget, gpointer user_data)
{
        SignalData *data = (SignalData *)user_data;
        g_autofree char *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
        g_settings_set_string(data->settings, data->key, fname);
}

GtkDialog *generic_config_dlg(const char *title, GtkWindow *parent, GSettings* settings,
                              ...)
{
        va_list l;
        va_start(l, settings);
        GtkDialog *dlg = GTK_DIALOG(gtk_dialog_new_with_buttons(title,
                                                                parent,
                                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                                _("_Close"),
                                                                GTK_RESPONSE_CLOSE,
                                                                NULL));
        GtkBox *dlg_vbox = GTK_BOX(gtk_dialog_get_content_area(dlg));
        vala_panel_apply_window_icon(GTK_WINDOW(dlg));
        gtk_box_set_spacing(dlg_vbox, 4);
        while (true) {
                const char *name = va_arg(l, const char *);
                if (!name)
                        break;
                GtkLabel *label = GTK_LABEL(gtk_label_new(name));
                GtkWidget *entry = NULL;
                void *arg = va_arg(l, void *);
                const char *key = NULL;
                GenericConfigType type = (GenericConfigType)va_arg(l, int);
                if (type == CONF_EXTERNAL)
                        entry = GTK_WIDGET(arg);
                else
                        key = (const char *)arg;
                if (type != CONF_TRIM && type != CONF_EXTERNAL && key == NULL)
                        g_critical("NULL pointer for generic config dialog");
                else
                        switch (type) {
                        case CONF_STR:
                                entry = gtk_entry_new();
                                gtk_entry_set_width_chars(GTK_ENTRY(entry), 40);
                                g_settings_bind(settings,
                                                key,
                                                entry,
                                                "text",
                                                G_SETTINGS_BIND_DEFAULT);
                                break;
                        case CONF_INT: {
                                /* FIXME: the range shouldn't be hardcoded */
                                entry = gtk_spin_button_new_with_range(0, 1000, 1);
                                g_settings_bind(settings,
                                                key,
                                                entry,
                                                "value",
                                                G_SETTINGS_BIND_DEFAULT);
                                break;
                        }
                        case CONF_BOOL:
                                entry = gtk_check_button_new();
                                gtk_container_add(GTK_CONTAINER(entry), GTK_WIDGET(label));
                                g_settings_bind(settings,
                                                key,
                                                entry,
                                                "active",
                                                G_SETTINGS_BIND_DEFAULT);
                                break;
                        case CONF_FILE:
                        case CONF_DIRECTORY: {
                                entry = gtk_file_chooser_button_new(
                                    _("Select a file"),
                                    CONF_FILE ? GTK_FILE_CHOOSER_ACTION_OPEN
                                              : GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
                                g_autofree char *str = g_settings_get_string(settings, key);
                                gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(entry), str);
                                g_autofree SignalData *data =
                                    (SignalData *)g_malloc(sizeof(SignalData));
                                g_signal_connect(entry,
                                                 "file-set",
                                                 G_CALLBACK(set_file_response),
                                                 data);
                                break;
                        }
                        case CONF_TRIM: {
                                entry = gtk_label_new(NULL);
                                g_autofree char *markup =
                                    g_markup_printf_escaped("<span style=\"italic\">%s</span>",
                                                            name);
                                gtk_label_set_markup(GTK_LABEL(entry), markup);
                                break;
                        }
                        case CONF_EXTERNAL:
                                if (GTK_IS_WIDGET(entry))
                                        gtk_box_pack_start(dlg_vbox, entry, false, false, 2);
                                else
                                        g_critical("value for CONF_EXTERNAL is not a GtkWidget");
                                break;
                        }
                if (entry) {
                        if ((type == CONF_BOOL) || (type == CONF_TRIM))
                                gtk_box_pack_start(dlg_vbox, entry, false, false, 2);
                        else {
                                GtkBox *hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
                                gtk_box_pack_start(hbox, GTK_WIDGET(label), false, false, 2);
                                gtk_box_pack_start(hbox, entry, true, true, 2);
                                gtk_box_pack_start(dlg_vbox, GTK_WIDGET(hbox), false, false, 2);
                        }
                }
        }
        g_signal_connect(dlg, "response", G_CALLBACK(gtk_widget_destroy), NULL);
        gtk_container_set_border_width(GTK_CONTAINER(dlg), 8);
        gtk_widget_show_all(GTK_WIDGET(dlg_vbox));
        return dlg;
}
