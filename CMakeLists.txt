cmake_minimum_required (VERSION 2.8)
cmake_policy (VERSION 2.8)
project (vala-panel)
enable_testing ()
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (DATADIR "${CMAKE_INSTALL_PREFIX}/share/vala-panel")
set (PKGDATADIR "${DATADIR}")
set (GETTEXT_PACKAGE "vala-panel")
set (RELEASE_NAME "Simple Vala panel")
set (VERSION "0.1.99")
set (VERSION_INFO "Development")
set (LIBNAME "valapanel")
set (APPNAME "vala-panel")
set (LIBVERSION "0")
set (SOVERSION 0)
set (PLUGINS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/lib/vala-panel/applets/")
set (PLUGINS_DATA "${DATADIR}/applets/")
set (PREFIX ${CMAKE_INSTALL_PREFIX})
set (DOLLAR "$")
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Add 'make dist' command for creating release tarball
set (CPACK_PACKAGE_VERSION ${VERSION})
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;/.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

include (CPack)
add_custom_target (dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# Configuration
configure_file (${CMAKE_SOURCE_DIR}/vapi/config.vala.in ${CMAKE_BINARY_DIR}/vapi/config.vala)
configure_file (${CMAKE_SOURCE_DIR}/${APPNAME}.pc.cmake ${CMAKE_BINARY_DIR}/${APPNAME}.pc)

# Some definitions
add_definitions(-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\")

# Vala
find_package (Vala REQUIRED)
include (ValaVersion)
ensure_vala_version ("0.24" MINIMUM)
include (ValaPrecompile)

add_definitions(-DVALA_VERSION="${VALA_SHORTVER}")
add_definitions(-w) # Disable gcc warnings

find_package (PkgConfig)
pkg_check_modules (GTK+ REQUIRED
  gtk+-3.0>=3.12)

pkg_check_modules (GLIB REQUIRED
  glib-2.0>=2.40)
  
pkg_check_modules (GIO REQUIRED
  gio-2.0>=2.40
  gio-unix-2.0>=2.40
  gthread-2.0)

pkg_check_modules (PEAS REQUIRED
  libpeas-1.0>=1.2
  libpeas-gtk-1.0>=1.2
  gmodule-2.0)

pkg_check_modules (CORE REQUIRED
  gtk+-3.0
  glib-2.0
  gio-2.0
  gio-unix-2.0
  gthread-2.0
  libpeas-1.0
  libpeas-gtk-1.0
  gmodule-2.0)

set(CORE_PACKAGES
  gtk+-3.0
  glib-2.0
  gio-2.0
  gio-unix-2.0
  libpeas-1.0
  libpeas-gtk-1.0
  gmodule-2.0
  gio-addons-2.0)

set(PACKGAGE_CFLAGS ${CORE_CFLAGS})
set(PACKAGE_LINK_DIRS ${CORE_LIBRARY_DIRS})
set(PACKAGE_LIBRARIES ${CORE_LIBRARIES})

add_definitions (${CORE_CFLAGS})
link_libraries (${CORE_LIBRARIES})
link_directories (${CORE_LIBRARY_DIRS})


# Some include paths
include_directories(${CMAKE_BINARY_DIR}/lib)

#set requred variables
set(TARGET_GLIB ${GLIB_VERSION_MAJOR}.${GLIB_VERSION_MINOR})

ADD_SUBDIRECTORY(data)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(app)
ADD_SUBDIRECTORY(applets)
ADD_SUBDIRECTORY(po)

#Additional install
#install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/vala-panel.desktop DESTINATION share/applications)

