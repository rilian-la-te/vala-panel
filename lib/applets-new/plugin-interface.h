#ifndef APPLETWIDGET_H
#define APPLETWIDGET_H

#include "applet-widget.h"
#include <glib-object.h>
#include <glib.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS
G_DECLARE_INTERFACE(ValaPanelPluginInterface, vala_panel_plugin_interface,
                    VALA_PANEL, PUGIN_INTERFACE, PeasExtensionBase);

struct _ValaPanelPluginInterface
{
  GTypeInterface parent_iface;
  ValaPanelAppletWidget* (*get_applet_widget)(ValaPanelPluginInterface* self,
                                              const gchar* uuid,
                                              const gchar* name,
                                              const gchar* path,
                                              const gchar* filename);
};
G_END_DECLS

#endif // APPLETWIDGET_H
