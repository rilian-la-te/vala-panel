#ifndef APPLETENGINEMODULE_H
#define APPLETENGINEMODULE_H

#include <glib-object.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelAppletEngineModule, vala_panel_applet_engine_module, VALA_PANEL,
		     APPLET_ENGINE_MODULE, GTypeModule)

G_END_DECLS

#endif // APPLETENGINEMODULE_H
