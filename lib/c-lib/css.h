#ifndef __VALA_PANEL_CSS_H__
#define __VALA_PANEL_CSS_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

inline void css_apply_with_class (GtkWidget* widget,const gchar* css, gchar* klass ,gboolean remove);
inline gchar* css_generate_background(const char *filename, GdkRGBA color,gboolean no_image);
inline gchar* css_generate_font_color(GdkRGBA color);
inline gchar* css_generate_font_size(gint size);
inline gchar* css_generate_font_label(gfloat size, gboolean is_bold);
inline gchar* css_apply_from_file (GtkWidget* widget, gchar* file);
inline gchar* css_apply_from_resource (GtkWidget* widget, gchar* file);
inline gchar* css_apply_from_file_to_app (gchar* file);
//inline gchar* css_generate_flat_button(GtkWidget* widget,ValaPanel* panel);

#endif
