#ifndef APPLETINFO_H
#define APPLETINFO_H

#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _ValaPanelAppeltInfo ValaPanelAppletInfo;

ValaPanelAppletInfo *vala_panel_applet_info_load();
ValaPanelAppletInfo *vala_panel_applet_info_duplicate(void *info);
void vala_panel_applet_info_free(void *info);

G_END_DECLS

#endif // APPLETINFO_H
