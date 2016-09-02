#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <glib.h>

#define g_free0(x)                                                                                 \
	{                                                                                          \
		if (x)                                                                             \
		{                                                                                  \
			g_free(x);                                                                 \
			x = NULL;                                                                  \
		}                                                                                  \
	}

#define g_value_replace_string(string, value)                                                      \
                                                                                                   \
	{                                                                                          \
		g_free0(string);                                                                   \
		string = g_value_dup_string(value);                                                \
	}

#define _user_config_file_name(name1, cprofile, name2)                                             \
	g_build_filename(g_get_user_config_dir(), GETTEXT_PACKAGE, cprofile, name1, name2, NULL)

#define g_ascii_inplace_tolower(string)                                                            \
	{                                                                                          \
		for (int i = 0; string[i] != '\0'; i++)                                            \
			g_ascii_tolower(string[i]);                                                \
	}

#define vala_panel_bind_gsettings(obj, settings, prop)                                             \
	g_settings_bind(settings,                                                                  \
	                prop,                                                                      \
	                G_OBJECT(obj),                                                             \
	                prop,                                                                      \
	                G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET | G_SETTINGS_BIND_DEFAULT);

#define VALA_PANEL_DECLARE_MODULE_TYPE(ModuleObjName, module_obj_name, MODULE, OBJ_NAME, ParentName) \
  GType module_obj_name##_get_type (void);                                                               \
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS                                                                       \
  typedef struct _##ModuleObjName ModuleObjName;                                                         \
  typedef struct { ParentName##Class parent_class; } ModuleObjName##Class;                               \
                                                                                                                                                                                                                  \
  static inline ModuleObjName * MODULE##_##OBJ_NAME (gpointer ptr) {                                     \
    return G_TYPE_CHECK_INSTANCE_CAST (ptr, module_obj_name##_get_type (), ModuleObjName); }             \
  static inline gboolean MODULE##_IS_##OBJ_NAME (gpointer ptr) {                                         \
    return G_TYPE_CHECK_INSTANCE_TYPE (ptr, module_obj_name##_get_type ()); }                            \
  G_GNUC_END_IGNORE_DEPRECATIONS

#endif // DEFINITIONS_H
