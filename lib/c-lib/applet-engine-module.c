#include "applet-engine-module.h"

struct _ValaPanelAppletEngineModule
{
     GTypeModule __parent__;
     char* filename;
     uint use_count;
};

G_DEFINE_TYPE(ValaPanelAppletEngineModule, vala_panel_applet_engine_module, G_TYPE_TYPE_MODULE)

static bool vala_panel_applet_engine_module_load(GTypeModule* type_module)
{
    ValaPanelAppletEngineModule *module = VALA_PANEL_APPLET_ENGINE_MODULE(type_module);

    g_free (module->filename);

    (*G_OBJECT_CLASS (vala_panel_applet_engine_module_parent_class)->finalize) (object);
  }



  static gboolean
  panel_module_load (GTypeModule *type_module)
  {
    PanelModule    *module = PANEL_MODULE (type_module);
    PluginInitFunc  init_func;
    gboolean        make_resident = TRUE;
    gpointer        foo;

    panel_return_val_if_fail (PANEL_IS_MODULE (module), FALSE);
    panel_return_val_if_fail (G_IS_TYPE_MODULE (module), FALSE);
    panel_return_val_if_fail (module->mode == INTERNAL, FALSE);
    panel_return_val_if_fail (module->library == NULL, FALSE);
    panel_return_val_if_fail (module->plugin_type == G_TYPE_NONE, FALSE);
    panel_return_val_if_fail (module->construct_func == NULL, FALSE);

    /* open the module */
    module->library = g_module_open (module->filename, G_MODULE_BIND_LOCAL);
    if (G_UNLIKELY (module->library == NULL))
      {
        g_critical ("Failed to load module \"%s\": %s.",
                    module->filename,
                    g_module_error ());
        return FALSE;
      }

      /* check if there is a preinit function */
    if (g_module_symbol (module->library, "xfce_panel_module_preinit", &foo))
      {
        /* large message, but technically never shown to normal users */
        g_warning ("The plugin \"%s\" is marked as internal in the desktop file, "
                   "but the developer has defined an pre-init function, which is "
                   "not supported for internal plugins. " PACKAGE_NAME " will force "
                   "the plugin to run external.", module->filename);

        panel_module_unload (type_module);

        /* from now on, run this plugin in a wrapper */
        module->mode = WRAPPER;
        g_free (module->api);
        module->api = g_strdup (LIBXFCE4PANEL_VERSION_API);

        return FALSE;
      }

    /* try to link the contruct function */
    if (g_module_symbol (module->library, "xfce_panel_module_init", (gpointer) &init_func))
      {
        /* initialize the plugin */
        module->plugin_type = init_func (type_module, &make_resident);

        /* whether to make this plugin resident or not */
        if (make_resident)
          g_module_make_resident (module->library);
      }
    else if (!g_module_symbol (module->library, "xfce_panel_module_construct",
                               (gpointer) &module->construct_func))
      {
        g_critical ("Module \"%s\" lacks a plugin register function.",
                    module->filename);

        panel_module_unload (type_module);

        return FALSE;
      }

  return TRUE;
}

void vala_panel_applet_engine_module_init(ValaPanelAppletEngineModule *self)
{
}
void vala_panel_applet_engine_module_class_init(ValaPanelAppletEngineModuleClass *klass)
{
}
