#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VALA_PANEL_SETTINGS_SCHEMA "org.valapanel.toplevel"
#define VALA_PANEL_SETTINGS_PATH "/org/vala-panel/toplevel/"
#define VALA_PANEL_TOPLEVEL_PATH_BASE "/org/vala-panel/objects/%s/%s/"

#define VALA_PANEL_KEY_EDGE "edge"
#define VALA_PANEL_KEY_ALIGNMENT "alignment"
#define VALA_PANEL_KEY_HEIGHT "height"
#define VALA_PANEL_KEY_WIDTH "width"
#define VALA_PANEL_KEY_DYNAMIC "is-dynamic"
#define VALA_PANEL_KEY_AUTOHIDE "autohide"
#define VALA_PANEL_KEY_SHOW_HIDDEN "show-hidden"
#define VALA_PANEL_KEY_STRUT "strut"
#define VALA_PANEL_KEY_DOCK "dock"
#define VALA_PANEL_KEY_MONITOR "monitor"
#define VALA_PANEL_KEY_MARGIN "panel-margin"
#define VALA_PANEL_KEY_ICON_SIZE "icon-size"
#define VALA_PANEL_KEY_BACKGROUND_COLOR "background-color"
#define VALA_PANEL_KEY_FOREGROUND_COLOR "foreground-color"
#define VALA_PANEL_KEY_BACKGROUND_FILE "background-file"
#define VALA_PANEL_KEY_FONT "font"
#define VALA_PANEL_KEY_CORNERS_SIZE "round-corners-size"
#define VALA_PANEL_KEY_USE_BACKGROUND_COLOR "use-background-color"
#define VALA_PANEL_KEY_USE_FOREGROUND_COLOR "use-foreground-color"
#define VALA_PANEL_KEY_USE_FONT "use-font"
#define VALA_PANEL_KEY_FONT_SIZE_ONLY "font-size-only"
#define VALA_PANEL_KEY_USE_BACKGROUND_FILE "use-background-file"

#define GAP 2

G_DECLARE_FINAL_TYPE(ValaPanelToplevelUnit, vala_panel_toplevel_unit, VALA_PANEL, TOPLEVEL_UNIT,
                     GtkApplicationWindow)

ValaPanelToplevelUnit *vala_panel_toplevel_unit_new_from_position(GtkApplication *app,
                                                                  const char *uid, int mon,
                                                                  GtkPositionType edge);
ValaPanelToplevelUnit *vala_panel_toplevel_unit_new_from_uid(GtkApplication *app, char *uid);

G_END_DECLS

#endif // TOPLEVEL_H
