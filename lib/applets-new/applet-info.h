#ifndef APPLETINFO_H
#define APPLETINFO_H

#include <glib-object.h>
#include <glib.h>

#include "applet-widget.h"

G_BEGIN_DECLS

#define DEFAULT_PLUGIN_SETTINGS_ID "org.valapanel.toplevel.applet"
#define DEFAULT_PLUGIN_PATH "/org/vala-panel/objects/"
#define DEFAULT_PLUGIN_GROUP "position"

#define DEFAULT_PLUGIN_NAME_KEY "plugin-type"
#define DEFAULT_PLUGIN_KEY_EXPAND "is-expanded"
#define DEFAULT_PLUGIN_KEY_POSITION "position"
#define DEFAULT_PLUGIN_KEY_ALIGNMENT "alignment"
#define DEFAULT_PLUGIN_KEY_ORIENTATION "orientation"

G_DECLARE_FINAL_TYPE(ValaPanelAppletInfo, vala_panel_applet_info, VALA_PANEL, APPLET_INFO, GObject)

G_END_DECLS

#endif // APPLETINFO_H
