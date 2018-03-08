cmake_minimum_required (VERSION 2.8.8)
cmake_policy (VERSION 2.8)
cmake_policy (SET CMP0063 NEW)
project (vala-dbusmenu C)
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Vala
find_package (Vala 0.24 REQUIRED)
include (ValaPrecompile)

add_definitions(-DVALA_VERSION="${VALA_SHORTVER}")
add_definitions(-Wall -Wextra
                -Wno-unused
                -Wno-unused-parameter# Vala loves to create variables
               #-pedantic
                -Wno-ignored-qualifiers
                -Wno-incompatible-pointer-types
                -Wno-missing-field-initializers
                -Wno-discarded-qualifiers
                -Wno-sign-compare
                ) #Vala often forgot cast to GtkWidget or gpointer

find_package (PkgConfig)

pkg_check_modules(GTK3 REQUIRED QUIET
    gtk+-3.0>=3.10)

set(VALA_FILES_DBUSMENU_CORE
    item.vala
    proxy.vala
    client.vala
    property-store.vala
    )
set(VALA_FILES_DBUSMENU_GTK
    gtk/client.vala
    gtk/main-item.vala
    gtk/separator-item.vala
    gtk/item-iface.vala
    gtk/scale-item.vala
    )
vala_precompile(VALA_C_DBUSMENU vala-dbusmenu
    ${VALA_FILES_DBUSMENU_CORE}
    ${VALA_FILES_DBUSMENU_GTK}
PACKAGES
    gtk+-3.0
    gio-2.0
    glib-2.0
    gio-addons-2.0
    gobject-2.0
OPTIONS
    --vapidir=${CMAKE_CURRENT_SOURCE_DIR}
    --target-glib=2.44
    --thread
GENERATE_VAPI
    vala-dbusmenu
GENERATE_HEADER
    vala-dbusmenu
)

add_library(vala-dbusmenu-o OBJECT ${VALA_C_DBUSMENU})
set_target_properties(vala-dbusmenu-o PROPERTIES
#        C_VISIBILITY_PRESET hidden	# Make global variables and functions HIDDEN by default.
        POSITION_INDEPENDENT_CODE ON	# Compile this object code position independent.
)
target_include_directories (vala-dbusmenu-o PRIVATE ${GTK3_INCLUDE_DIRS})
add_custom_target(vala-dbusmenu-ide SOURCES ${VALA_FILES_DBUSMENU_CORE} ${VALA_FILES_DBUSMENU_GTK})
