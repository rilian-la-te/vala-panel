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
  \                                                                            \
  {                                                                            \
    \ g_free0(string);                                                         \
    \ string = g_value_dup_string(value);                                      \
    \                                                                          \
  }

#define _user_config_file_name(name1, cprofile, name2)                         \
  g_build_filename(g_get_user_config_dir(), "simple-panel", cprofile, name1,   \
                   name2, NULL)

#endif // DEFINITIONS_H
