/*
 * vala-panel
 * Copyright (C) 2015-2016 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <stdarg.h>

#include "generic-config-dialog.h"
#include "lib/applets-new/applet-api.h"
#include "lib/misc.h"

typedef struct
{
	GSettings *settings;
	const char *key;
} SignalData;

static void set_file_response(GtkFileChooserButton *widget, gpointer user_data)
{
	SignalData *data       = (SignalData *)user_data;
	g_autofree char *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
	g_settings_set_string(data->settings, data->key, fname);
}

static GtkWidget *generic_config_widget_internal(GSettings *settings, va_list l)
{
	GtkBox *dlg_vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 4));
	while (true)
	{
		const char *name = va_arg(l, const char *);
		if (!name)
			break;
		GtkLabel *label        = GTK_LABEL(gtk_label_new(name));
		GtkWidget *entry       = NULL;
		void *arg              = va_arg(l, void *);
		const char *key        = NULL;
		GenericConfigType type = (GenericConfigType)va_arg(l, int);
		if (type == CONF_EXTERNAL)
			entry = GTK_WIDGET(arg);
		else
			key = (const char *)arg;
		if (type != CONF_TRIM && type != CONF_EXTERNAL && key == NULL)
			g_critical("NULL pointer for generic config dialog");
		else
			switch (type)
			{
			case CONF_STR:
				entry = gtk_entry_new();
				gtk_entry_set_width_chars(GTK_ENTRY(entry), 40);
				g_settings_bind(settings,
				                key,
				                entry,
				                "text",
				                G_SETTINGS_BIND_DEFAULT);
				break;
			case CONF_INT:
			{
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
			case CONF_DIRECTORY:
			{
				entry = gtk_file_chooser_button_new(
				    _("Select a file"),
				    type == CONF_FILE ? GTK_FILE_CHOOSER_ACTION_OPEN
				                      : GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
				g_autofree char *str = g_settings_get_string(settings, key);
				gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(entry), str);
				SignalData *data = (SignalData *)g_malloc0(sizeof(SignalData));
				data->key        = key;
				data->settings   = settings;
				g_signal_connect(entry,
				                 "file-set",
				                 G_CALLBACK(set_file_response),
				                 data);
				g_signal_connect_swapped(dlg_vbox,
				                         "destroy",
				                         G_CALLBACK(g_free),
				                         data);
				break;
			}
			case CONF_FILE_ENTRY:
			case CONF_DIRECTORY_ENTRY:
			{
				entry          = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
				GtkWidget *btn = gtk_file_chooser_button_new(
				    _("Select a file"),
				    type == CONF_FILE_ENTRY
				        ? GTK_FILE_CHOOSER_ACTION_OPEN
				        : GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
				GtkWidget *str_elem = gtk_entry_new();
				gtk_entry_set_width_chars(GTK_ENTRY(str_elem), 40);
				g_settings_bind(settings,
				                key,
				                str_elem,
				                "text",
				                G_SETTINGS_BIND_DEFAULT);
				g_autofree char *str = g_settings_get_string(settings, key);
				gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(btn), str);
				SignalData *data = (SignalData *)g_malloc0(sizeof(SignalData));
				data->key        = key;
				data->settings   = settings;
				g_signal_connect(btn,
				                 "file-set",
				                 G_CALLBACK(set_file_response),
				                 &data);
				g_signal_connect_swapped(dlg_vbox,
				                         "destroy",
				                         G_CALLBACK(g_free),
				                         data);
				gtk_box_pack_start(GTK_BOX(entry), str_elem, true, true, 0);
				gtk_box_pack_start(GTK_BOX(entry), btn, false, true, 0);
				break;
			}
			case CONF_TRIM:
			{
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
		if (entry)
		{
			if ((type == CONF_BOOL) || (type == CONF_TRIM))
				gtk_box_pack_start(dlg_vbox, entry, false, false, 2);
			else
			{
				GtkBox *hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
				gtk_box_pack_start(hbox, GTK_WIDGET(label), false, false, 2);
				gtk_box_pack_start(hbox, entry, true, true, 2);
				gtk_box_pack_start(dlg_vbox, GTK_WIDGET(hbox), false, false, 2);
			}
		}
	}
	return GTK_WIDGET(dlg_vbox);
}

GtkWidget *generic_config_widget(GSettings *settings, ...)
{
	va_list l;
	va_start(l, settings);
	GtkWidget *w = generic_config_widget_internal(settings, l);
	va_end(l);
	return w;
}

GtkDialog *generic_config_dlg(const char *title, GtkWindow *parent, GSettings *settings, ...)
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
	GtkWidget *settings_widget = generic_config_widget_internal(settings, l);
	gtk_container_add(GTK_CONTAINER(dlg_vbox), settings_widget);
	gtk_box_set_spacing(dlg_vbox, 4);
	g_signal_connect(dlg, "response", G_CALLBACK(gtk_widget_destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(dlg), 8);
	gtk_widget_show_all(GTK_WIDGET(dlg_vbox));
	va_end(l);
	return dlg;
}
