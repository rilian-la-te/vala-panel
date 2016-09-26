/*
 * vala-panel
 * Copyright (C) 2015-2016 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#define gtk_widget_destroy0(x)                                                                     \
	{                                                                                          \
		if (x)                                                                             \
		{                                                                                  \
			gtk_widget_destroy(GTK_WIDGET(x));                                         \
			x = NULL;                                                                  \
		}                                                                                  \
	}

#define g_object_unref0(x)                                                                         \
	{                                                                                          \
		if (x)                                                                             \
		{                                                                                  \
			g_object_unref(x);                                                         \
			x = NULL;                                                                  \
		}                                                                                  \
	}

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
	                (GBindingFlags)(G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET |                \
	                                G_SETTINGS_BIND_DEFAULT));

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

#define vala_panel_dup_array(DST, SRC, LEN)                                                        \
	{                                                                                          \
		size_t TMPSZ = sizeof(*(SRC)) * (LEN);                                             \
		if (((DST) = malloc(TMPSZ)) != NULL)                                               \
			memcpy((DST), (SRC), TMPSZ);                                               \
	}

#endif // DEFINITIONS_H
