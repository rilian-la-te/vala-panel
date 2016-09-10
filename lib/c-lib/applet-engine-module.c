#include "applet-engine-module.h"
#include <gmodule.h>

struct _ValaPanelAppletEngineModule
{
	GTypeModule __parent__;
	char *filename;
	ValaPanelAppletEngine *engine;
	GModule *library;
	GType plugin_type;
};

G_DEFINE_TYPE(ValaPanelAppletEngineModule, vala_panel_applet_engine_module, G_TYPE_TYPE_MODULE)

ValaPanelAppletEngineModule *vala_panel_applet_engine_module_new_from_ini(const char *filename)
{
	ValaPanelAppletEngineModule *module = NULL;
	g_autoptr(GKeyFile) rc              = g_key_file_new();
	g_autoptr(GError) err               = NULL;
	g_key_file_load_from_file(rc, filename, G_KEY_FILE_KEEP_COMMENTS, &err);
	if (G_UNLIKELY(rc == NULL))
	{
		g_critical("Unable to read plugin engine from desktop file \"%s\"", filename);
		return NULL;
	}

	if (!g_key_file_has_group(rc, MODULE_GROUP_NAME))
	{
		g_critical(
		    "Desktop file \"%s\" has no "
		    "\"" MODULE_GROUP_NAME "\" group",
		    filename);
		return NULL;
	}

	/* read module location from the desktop file */
	g_autofree char *module_name =
	    g_key_file_get_string(rc, MODULE_GROUP_NAME, MODULE_NAME_STRING, NULL);
	if (G_LIKELY(module_name != NULL))
	{
		g_autofree char *path = g_module_build_path(CONFIG_PLUGINS_DIRECTORY, module_name);
		bool found            = g_file_test(path, G_FILE_TEST_EXISTS);

		if (G_LIKELY(found))
		{
			/* create new module */
			module = g_object_new(vala_panel_applet_engine_module_get_type(), NULL);
			module->filename = path;
		}
		else
			g_critical("There was no module found at \"%s\"", path);
	}
	if (G_LIKELY(module != NULL))
		g_type_module_set_name(G_TYPE_MODULE(module), module_name);

	return module;
}

ValaPanelAppletEngine *vala_panel_applet_engine_module_get_engine(ValaPanelAppletEngineModule *self)
{
	if (g_type_module_use(G_TYPE_MODULE(self)))
	{
		if (self->plugin_type != G_TYPE_NONE && !self->engine)
		{
			/* plugin is build as an object, to use its gtype */
			self->engine = g_object_new(self->plugin_type, NULL);
		}
	}
	else
		g_type_module_unuse(G_TYPE_MODULE(self));

	return self->engine;
}

void vala_panel_applet_engine_module_free_engine(ValaPanelAppletEngineModule *self)
{
	g_object_unref(self->engine);
	self->engine = NULL;
	g_type_module_unuse(G_TYPE_MODULE(self));
}

static void vala_panel_applet_engine_module_unload(GTypeModule *type_module)
{
	ValaPanelAppletEngineModule *module = VALA_PANEL_APPLET_ENGINE_MODULE(type_module);
	vala_panel_applet_engine_module_free_engine(module);
	g_module_close(module->library);

	/* reset plugin state */
	module->library     = NULL;
	module->plugin_type = G_TYPE_NONE;
}

static bool vala_panel_applet_engine_module_load(GTypeModule *type_module)
{
	ValaPanelAppletEngineModule *module = VALA_PANEL_APPLET_ENGINE_MODULE(type_module);
	ValaPanelAppletEngineInitFunc init_func;
	bool make_resident = true;

	/* open the module */
	module->library = g_module_open(module->filename, G_MODULE_BIND_LOCAL);
	if (G_UNLIKELY(module->library == NULL))
	{
		g_critical("Failed to load module \"%s\": %s.", module->filename, g_module_error());
		return FALSE;
	}

	/* try to link the contruct function */
	if (g_module_symbol(module->library, "vala_panel_module_init", (void **)&init_func))
	{
		/* initialize the plugin */
		module->plugin_type = init_func(type_module, &make_resident);

		/* whether to make this plugin resident or not */
		if (make_resident)
			g_module_make_resident(module->library);
	}
	else
	{
		g_critical("Module \"%s\" lacks a plugin register function.", module->filename);

		vala_panel_applet_engine_module_unload(type_module);

		return FALSE;
	}

	return TRUE;
}
static void vala_panel_applet_engine_module_dispose(GObject *object)
{
	/* Do nothing to avoid problems with dispose in GTypeModule when
	 * types are registered.
	 *
	 * For us this is not a problem since the modules are released when
	 * everything is destroyed. So we really want that last unref before
	 * closing the application. */
}
static void vala_panel_applet_engine_module_finalize(GTypeModule *type_module)
{
	ValaPanelAppletEngineModule *module = VALA_PANEL_APPLET_ENGINE_MODULE(type_module);

	g_free(module->filename);

	(*G_OBJECT_CLASS(vala_panel_applet_engine_module_parent_class)->finalize)(type_module);
}
static void vala_panel_applet_engine_module_init(ValaPanelAppletEngineModule *self)
{
	self->filename    = NULL;
	self->engine      = NULL;
	self->library     = NULL;
	self->plugin_type = G_TYPE_NONE;
}
static void vala_panel_applet_engine_module_class_init(ValaPanelAppletEngineModuleClass *klass)
{
	GObjectClass *gobject_class;
	GTypeModuleClass *gtype_module_class;

	gobject_class           = G_OBJECT_CLASS(klass);
	gobject_class->dispose  = vala_panel_applet_engine_module_dispose;
	gobject_class->finalize = vala_panel_applet_engine_module_finalize;

	gtype_module_class         = G_TYPE_MODULE_CLASS(klass);
	gtype_module_class->load   = vala_panel_applet_engine_module_load;
	gtype_module_class->unload = vala_panel_applet_engine_module_unload;
}
