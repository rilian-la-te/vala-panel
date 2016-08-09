#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <glib.h>

#define g_free0(x)                                                             \
  {                                                                            \
    if (x) {                                                                   \
      g_free(x);                                                               \
      x = NULL;                                                                \
    }                                                                          \
  }

#define g_value_replace_string(string, value)                                  \
                                                                               \
  {                                                                            \
    g_free0(string);                                                           \
    string = g_value_dup_string(value);                                        \
  }

#define _user_config_file_name(name1, cprofile, name2)                         \
  g_build_filename(g_get_user_config_dir(), "simple-panel", cprofile, name1,   \
                   name2, NULL)

#define g_ascii_inplace_tolower(string) \
{ \
    for(int i = 0; string[i]!='\0'; i++)\
        g_ascii_tolower(string[i]); \
}

#define vala_panel_bind_gsettings(obj,settings,prop)\
    g_settings_bind(settings,prop,G_OBJECT(obj),prop,G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET | G_SETTINGS_BIND_DEFAULT);

#endif // DEFINITIONS_H
