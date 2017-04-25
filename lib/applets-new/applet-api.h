#ifndef APPLETAPI_H
#define APPLETAPI_H

#define VALA_PANEL_DECLARE_MODULE_TYPE(                                                            \
    ModuleObjName, module_obj_name, MODULE, OBJ_NAME, ParentName)                                  \
	GType module_obj_name##_get_type(void);                                                    \
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS                                                           \
	typedef struct _##ModuleObjName ModuleObjName;                                             \
	typedef struct                                                                             \
	{                                                                                          \
		ParentName##Class parent_class;                                                    \
	} ModuleObjName##Class;                                                                    \
                                                                                                   \
	static inline ModuleObjName *MODULE##_##OBJ_NAME(gpointer ptr)                             \
	{                                                                                          \
		return G_TYPE_CHECK_INSTANCE_CAST(ptr,                                             \
		                                  module_obj_name##_get_type(),                    \
		                                  ModuleObjName);                                  \
	}                                                                                          \
	static inline gboolean MODULE##_IS_##OBJ_NAME(gpointer ptr)                                \
	{                                                                                          \
		return G_TYPE_CHECK_INSTANCE_TYPE(ptr, module_obj_name##_get_type());              \
	}                                                                                          \
	G_GNUC_END_IGNORE_DEPRECATIONS

#include "applet-engine.h"
#include "applet-info.h"
#include "applet-widget.h"

#endif // APPLETAPI_H
