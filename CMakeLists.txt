vala_precompile(VALA_C vala-dbusmenu
    dbusmenu.vala
PACKAGES
    gtk+-3.0
    gio-2.0
    glib-2.0
    gobject-2.0
OPTIONS
    --target-glib=2.40
    --thread
GENERATE_VAPI
    vala-dbusmenu
GENERATE_HEADER
    vala-dbusmenu
)

#####
# Core Library
#####

# Build library for plugins and application
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})

set (LIB_FILES ${VALA_C})

add_library (vala-dbusmenu STATIC
    ${LIB_FILES}
)

target_link_libraries (vala-dbusmenu ${LIBS})

set_target_properties (vala-dbusmenu PROPERTIES
    VERSION 0.0
    SOVERSION 0)
