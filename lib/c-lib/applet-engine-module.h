#ifndef APPLETENGINEMODULE_H
#define APPLETENGINEMODULE_H

#include "lib/definitions.h"
#include <glib-object.h>
#include <stdbool.h>

G_BEGIN_DECLS

VALA_PANEL_DECLARE_MODULE_TYPE(ValaPanelAppletEngineModule, vala_panel_applet_engine_module,
                               VALA_PANEL, APPLET_ENGINE_MODULE, GTypeModule)

G_END_DECLS

#endif // APPLETENGINEMODULE_H
