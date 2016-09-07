#ifndef APPLETENGINEMODULE_H
#define APPLETENGINEMODULE_H

#include "lib/config.h"
#include "lib/definitions.h"
#include <glib-object.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define MODULE_GROUP_NAME "Vala Panel Engine"
#define MODULE_NAME_STRING "ModuleName"

VALA_PANEL_DECLARE_MODULE_TYPE(ValaPanelAppletEngineModule, vala_panel_applet_engine_module,
                               VALA_PANEL, APPLET_ENGINE_MODULE, GTypeModule)

ValaPanelAppletEngineModule *vala_panel_applet_engine_module_new_from_ini(const char *filename);

typedef GType (*ValaPanelAppletEngineInitFunc)(GTypeModule *module, bool *make_resident);

G_END_DECLS

#endif // APPLETENGINEMODULE_H
