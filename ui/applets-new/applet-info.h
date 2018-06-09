#ifndef APPLETINFO_H
#define APPLETINFO_H

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <stdbool.h>

G_BEGIN_DECLS

typedef struct _ValaPanelAppeltInfo ValaPanelAppletInfo;

ValaPanelAppletInfo *vala_panel_applet_info_load(const char *extension_name);
ValaPanelAppletInfo *vala_panel_applet_info_duplicate(void *info);
void vala_panel_applet_info_free(void *info);

const char *vala_panel_applet_info_get_module_name(ValaPanelAppletInfo *info);
const char *vala_panel_applet_info_get_name(ValaPanelAppletInfo *info);
const char *vala_panel_applet_info_get_description(ValaPanelAppletInfo *info);
const char *vala_panel_applet_info_get_icon_name(ValaPanelAppletInfo *info);
const char *const *vala_panel_applet_info_get_authors(ValaPanelAppletInfo *info);
const char *vala_panel_applet_info_get_website(ValaPanelAppletInfo *info);
const char *vala_panel_applet_info_get_help_uri(ValaPanelAppletInfo *info);
GtkLicense vala_panel_applet_info_get_license(ValaPanelAppletInfo *info);
const char *vala_panel_applet_info_get_version(ValaPanelAppletInfo *info);
bool vala_panel_applet_info_get_one_per_system(ValaPanelAppletInfo *info);
bool vala_panel_applet_info_get_expandable(ValaPanelAppletInfo *info);

G_END_DECLS

#endif // APPLETINFO_H
