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

#include <gdk/gdk.h>
#include <glib-object.h>
#include <glib.h>

#include <math.h>
#include <string.h>

#include "css.h"

void css_apply_with_class(GtkWidget *widget, const gchar *css, const gchar *klass, bool remove)
{
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	gtk_widget_reset_style(widget);
	if (remove)
	{
		gtk_style_context_remove_class(context, klass);
	}
	else
	{
		g_autoptr(GtkCssProvider) provider = gtk_css_provider_new();
		gtk_css_provider_load_from_data(provider, css, -1, NULL);
		gtk_style_context_add_class(context, klass);
		gtk_style_context_add_provider(context,
		                               GTK_STYLE_PROVIDER(provider),
		                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	}
}

gchar *css_apply_from_file(GtkWidget *widget, const gchar *file)
{
	g_autoptr(GError) error  = NULL;
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	gtk_widget_reset_style(widget);
	g_autoptr(GtkCssProvider) provider = gtk_css_provider_new();
	gtk_css_provider_load_from_path(provider, file, &error);
	if (error)
	{
		gchar *returnie = g_strdup(error->message);
		return returnie;
	}
	gtk_style_context_add_provider(context,
	                               GTK_STYLE_PROVIDER(provider),
	                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	return NULL;
}

gchar *css_apply_from_file_to_app(const gchar *file)
{
	g_autoptr(GError) error            = NULL;
	g_autoptr(GtkCssProvider) provider = gtk_css_provider_new();
	gtk_css_provider_load_from_path(provider, file, &error);
	if (error)
	{
		gchar *returnie = g_strdup(error->message);
		return returnie;
	}
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
	                                          GTK_STYLE_PROVIDER(provider),
	                                          GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	return NULL;
}

inline gchar *css_generate_background(const char *filename, GdkRGBA *color)
{
	gchar *returnie;
	g_autofree gchar *str = gdk_rgba_to_string(color);
	if (!filename)
		returnie = g_strdup_printf(
		    ".-vala-panel-background{\n"
		    " background-color: %s;\n"
		    " background-image: none;\n"
		    "}",
		    str);
	else
		returnie = g_strdup_printf(
		    ".-vala-panel-background{\n"
		    " background-color: transparent;\n"
		    " background-image: url('%s');\n"
		    "}",
		    filename);
	return returnie;
}

inline gchar *css_generate_font_color(GdkRGBA color)
{
	g_autofree gchar *color_str = gdk_rgba_to_string(&color);
	gchar *ret;
	ret = g_strdup_printf(
	    ".-vala-panel-font-color{\n"
	    "color: %s;\n"
	    "}",
	    color_str);
	return ret;
}
inline gchar *css_generate_font_size(gint size)
{
	return g_strdup_printf(
	    ".-vala-panel-font-size{\n"
	    " font-size: %dpx;\n"
	    "}",
	    size);
}
inline gchar *css_generate_font_label(gfloat size, bool is_bold)
{
	gint size_factor = (gint)round(size * 100);
	return g_strdup_printf(
	    ".-vala-panel-font-label{\n"
	    " font-size: %d%%;\n"
	    " font-weight: %s;\n"
	    "}",
	    size_factor,
	    is_bold ? "bold" : "normal");
}

inline gchar *css_generate_flat_button(GtkWidget *widget, GtkPositionType direction)
{
	gchar *returnie;
	GdkRGBA color, active_color;
	gtk_style_context_get_color(gtk_widget_get_style_context(widget),
	                            gtk_widget_get_state_flags(widget),
	                            &color);
	color.alpha              = 0.8;
	active_color.red         = color.red;
	active_color.green       = color.green;
	active_color.blue        = color.blue;
	active_color.alpha       = 0.5;
	g_autofree char *c_str   = gdk_rgba_to_string(&color);
	g_autofree char *act_str = gdk_rgba_to_string(&active_color);
	const char *edge;
	if (direction == GTK_POS_BOTTOM)
		edge = "0px 0px 2px 0px";
	if (direction == GTK_POS_TOP)
		edge = "2px 0px 0px 0px";
	if (direction == GTK_POS_RIGHT)
		edge = "0px 2px 0px 0px";
	if (direction == GTK_POS_LEFT)
		edge = "0px 0px 0px 2px";
	returnie     = g_strdup_printf(
	    ".-panel-flat-button {\n"
	    "padding: 0px;\n"
	    " -GtkWidget-focus-line-width: 0px;\n"
	    " -GtkWidget-focus-padding: 0px;\n"
	    "border-style: solid;"
	    "border-color: transparent;"
	    "border-width: %s;"
	    "}\n"
	    ".-panel-flat-button:checked,"
	    ".-panel-flat-button:active {\n"
	    "border-style: solid;"
	    "border-width: %s;"
	    "border-color: %s;"
	    "}\n"
	    ".-panel-flat-button:hover,"
	    ".-panel-flat-button.highlight,"
	    ".-panel-flat-button:active:hover {\n"
	    "border-style: solid;"
	    "border-width: %s;"
	    "border-color: %s;"
	    "}\n",
	    edge,
	    edge,
	    act_str,
	    edge,
	    c_str);
	return returnie;
}

gchar *css_apply_from_resource(GtkWidget *widget, const char *file, const char *klass)
{
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	gtk_widget_reset_style(widget);
	g_autoptr(GtkCssProvider) provider = gtk_css_provider_new();
	gtk_css_provider_load_from_resource(provider, file);
	gtk_style_context_add_provider(context,
	                               GTK_STYLE_PROVIDER(provider),
	                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_style_context_add_class(context, klass);
	return NULL;
}
void css_toggle_class(GtkWidget *widget, const char *klass, bool apply)
{
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	if (apply)
		gtk_style_context_add_class(context, klass);
	else
		gtk_style_context_remove_class(context, klass);
}

GtkCssProvider *css_add_css_to_widget(GtkWidget *widget, const char *css)
{
	g_autoptr(GError) err    = NULL;
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	gtk_widget_reset_style(widget);
	GtkCssProvider *provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(provider, css, strlen(css), &err);
	gtk_style_context_add_provider(context,
	                               GTK_STYLE_PROVIDER(provider),
	                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	return err ? NULL : provider;
}

GtkCssProvider *css_apply_from_file_to_app_with_provider(const char *file)
{
	GtkCssProvider *provider = gtk_css_provider_new();
	g_autoptr(GError) err    = NULL;
	gtk_css_provider_load_from_path(provider, file, &err);
	GdkScreen *scr = gdk_screen_get_default();
	gtk_style_context_add_provider_for_screen(scr,
	                                          GTK_STYLE_PROVIDER(provider),
	                                          GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	return err ? NULL : provider;
}
