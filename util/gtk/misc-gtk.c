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
#include <inttypes.h>
#include <string.h>

#include "css-private.h"
#include "misc-gtk.h"
#include "misc.h"

static void set_widget_align(GtkWidget *user_data, G_GNUC_UNUSED gpointer data)
{
	if (GTK_IS_WIDGET(user_data))
	{
		gtk_widget_set_halign(GTK_WIDGET(user_data), GTK_ALIGN_FILL);
		gtk_widget_set_valign(GTK_WIDGET(user_data), GTK_ALIGN_FILL);
	}
}

/* Draw text into a label, with the user preference color and optionally bold. */
void vala_panel_setup_label(GtkLabel *label, const char *text, bool bold, double factor)
{
	gtk_label_set_text(label, text);
	g_autofree char *css = css_generate_font_label(factor, bold);
	css_apply_with_class(GTK_WIDGET(label), css, "-vala-panel-font-label", false);
}

/* Children hierarhy: button => alignment => box => (label,image) */
static void setup_button_notify_connect(GObject *_sender, GParamSpec *b,
                                        G_GNUC_UNUSED gpointer self)
{
	GtkButton *a = GTK_BUTTON(_sender);
	if (!strcmp(b->name, "label") || !strcmp(b->name, "image"))
	{
		GtkWidget *w = gtk_bin_get_child(GTK_BIN(a));
		if (GTK_IS_CONTAINER(w))
		{
			GtkWidget *ch;
			if (GTK_IS_BIN(w))
				ch = gtk_bin_get_child(GTK_BIN(w));
			else
				ch = w;
			if (GTK_IS_CONTAINER(ch))
				gtk_container_forall(GTK_CONTAINER(ch), set_widget_align, NULL);
			gtk_widget_set_halign(ch, GTK_ALIGN_FILL);
			gtk_widget_set_valign(ch, GTK_ALIGN_FILL);
		}
	}
}

void vala_panel_setup_button(GtkButton *b, GtkImage *img, const char *label)
{
	css_apply_from_resource(GTK_WIDGET(b), "/org/vala-panel/lib/style.css", "-panel-button");
	g_signal_connect(G_OBJECT(b), "notify", G_CALLBACK(setup_button_notify_connect), NULL);
	if (img != NULL)
	{
		gtk_button_set_image(b, GTK_WIDGET(img));
		gtk_button_set_always_show_image(b, true);
	}
	if (label != NULL)
		gtk_button_set_label(b, label);
	gtk_button_set_relief(b, GTK_RELIEF_NONE);
}

void vala_panel_setup_icon(GtkImage *img, GIcon *icon, GObject *top, int size)
{
	gtk_image_set_from_gicon(img, icon, GTK_ICON_SIZE_INVALID);
	if (top != NULL)
		g_object_bind_property(top,
		                       "icon-size",
		                       img,
		                       "pixel-size",
		                       G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
	else if (size > 0)
		gtk_image_set_pixel_size(img, size);
}

void vala_panel_setup_icon_button(GtkButton *btn, GIcon *icon, const char *label, GObject *top)
{
	css_apply_from_resource(GTK_WIDGET(btn),
	                        "/org/vala-panel/lib/style.css",
	                        "-panel-icon-button");
	css_toggle_class(GTK_WIDGET(btn), GTK_STYLE_CLASS_BUTTON, true);
	GtkImage *img = NULL;
	if (icon != NULL)
	{
		img = GTK_IMAGE(gtk_image_new());
		vala_panel_setup_icon(img, icon, top, -1);
	}
	vala_panel_setup_button(btn, img, label);
	gtk_container_set_border_width(GTK_CONTAINER(btn), 0);
	gtk_widget_set_can_focus(GTK_WIDGET(btn), false);
	gtk_widget_set_has_window(GTK_WIDGET(btn), false);
}

void vala_panel_apply_window_icon(GtkWindow *win)
{
	g_autoptr(GdkPixbuf) icon =
	    gdk_pixbuf_new_from_resource("/org/vala-panel/lib/panel.png", NULL);
	gtk_window_set_icon(win, icon);
}

void vala_panel_scale_button_set_range(GtkScaleButton *b, gint lower, gint upper)
{
	gtk_adjustment_set_lower(gtk_scale_button_get_adjustment(b), lower);
	gtk_adjustment_set_upper(gtk_scale_button_get_adjustment(b), upper);
	gtk_adjustment_set_step_increment(gtk_scale_button_get_adjustment(b), 1);
	gtk_adjustment_set_page_increment(gtk_scale_button_get_adjustment(b), 1);
}

void vala_panel_scale_button_set_value_labeled(GtkScaleButton *b, gint value)
{
	gtk_scale_button_set_value(b, value);
	g_autofree char *str = g_strdup_printf("%d", value);
	gtk_button_set_label(GTK_BUTTON(b), str);
}

int vala_panel_monitor_num_from_mon(GdkDisplay *disp, GdkMonitor *mon)
{
	int mons = gdk_display_get_n_monitors(disp);
	for (int i = 0; i < mons; i++)
	{
		if (mon == gdk_display_get_monitor(disp, i))
			return i;
	}
	return -1;
}

void vala_panel_generate_error_dialog(GtkWindow *parent, const char *error)
{
	g_warning("%s", error);
	GtkWidget *dlg = gtk_message_dialog_new(parent,
	                                        GTK_DIALOG_DESTROY_WITH_PARENT,
	                                        GTK_MESSAGE_ERROR,
	                                        GTK_BUTTONS_CLOSE,
	                                        "%s",
	                                        error);
	vala_panel_apply_window_icon(GTK_IS_WINDOW(dlg) ? GTK_WINDOW(dlg) : NULL);
	gtk_window_set_title(GTK_WINDOW(dlg), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(GTK_WIDGET(dlg));
}

bool vala_panel_generate_confirmation_dialog(GtkWindow *parent, const char *error)
{
	GtkWidget *dlg = gtk_message_dialog_new(parent,
	                                        GTK_DIALOG_MODAL,
	                                        GTK_MESSAGE_QUESTION,
	                                        GTK_BUTTONS_OK_CANCEL,
	                                        "%s",
	                                        error);
	vala_panel_apply_window_icon(GTK_IS_WINDOW(dlg) ? GTK_WINDOW(dlg) : NULL);
	gtk_window_set_title(GTK_WINDOW(dlg), _("Confirm"));
	bool ret = (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK);
	gtk_widget_destroy(GTK_WIDGET(dlg));
	return ret;
}
