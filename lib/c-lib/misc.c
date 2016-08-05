#include "misc.h"


void vala_panel_apply_window_icon(GtkWindow *win)
{
    g_autoptr(GdkPixbuf) icon;
    icon = gdk_pixbuf_new_from_resource("/org/vala-panel/lib/panel.png",NULL);
    gtk_window_set_icon(win,icon);
}
