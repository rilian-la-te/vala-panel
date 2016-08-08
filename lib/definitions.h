#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <glib.h>

#define g_free0(x)\
      { \
            if(x) {\
                g_free(x); \
                x = NULL; \
        } \
    }

#define g_value_replace_string(string, value) \                                                              \
        {                    \                                                                      \
                g_free0(string);   \                                                                 \
                string = g_value_dup_string(value); \                                            \
}\

#endif // DEFINITIONS_H
