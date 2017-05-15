cmake_minimum_required (VERSION 2.8)
cmake_policy (VERSION 2.8)
project (vala-dbusmenu C)
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Vala
find_package (Vala REQUIRED)
include (ValaVersion)
ensure_vala_version ("0.24" MINIMUM)
include (ValaPrecompile)

add_definitions(-DVALA_VERSION="${VALA_SHORTVER}")
add_definitions(-Wall -Wno-unused # Vala loves to create variables
#				-pedantic
                -Wno-ignored-qualifiers
                -Wno-incompatible-pointer-types
                -Wno-missing-field-initializers
                -Wno-discarded-qualifiers
                -Wno-sign-compare
                -Wextra) #Vala often forgot cast to GtkWidget or gpointer

find_package (PkgConfig)

pkg_check_modules(GTK3 REQUIRED
    gtk+-3.0>=3.10)

set(VALA_FILES_DBUSMENU dbusmenu.vala)
vala_precompile(VALA_C_DBUSMENU vala-dbusmenu
    ${VALA_FILES_DBUSMENU}
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

add_library(vala-dbusmenu STATIC ${VALA_C_DBUSMENU})
target_link_libraries (vala-dbusmenu ${GTK3_LIBRARIES})
target_include_directories (vala-dbusmenu PRIVATE ${GTK3_INCLUDE_DIRS})
add_definitions(-fPIC)
