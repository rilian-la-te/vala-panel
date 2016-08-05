cmake_minimum_required (VERSION 2.8)
cmake_policy (VERSION 2.8)
project (vala-panel C)
enable_testing ()
include(GNUInstallDirs)
set (DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/vala-panel")
set (DATADIR_INSTALL "${CMAKE_INSTALL_DATADIR}/vala-panel")
set (PKGDATADIR "${DATADIR}")
set (GETTEXT_PACKAGE "vala-panel")
set (RELEASE_NAME "Simple Vala panel")
set (LIBNAME "valapanel")
set (APPNAME "vala-panel")
set (SOVERSION 0)
set (PLUGINS_DIRECTORY "${CMAKE_INSTALL_FULL_LIBDIR}/vala-panel/applets/")
set (PLUGINS_DATA "${DATADIR}/applets/")
set (PLUGINS_DATA_INSTALL "${DATADIR_INSTALL}/applets")
set (PROFILES_DIR "${CMAKE_INSTALL_SYSCONFDIR}/xdg/${APPNAME}")
set (PREFIX ${CMAKE_INSTALL_PREFIX})
set (DOLLAR "$")
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(GitVersion)
determine_fallback_version(${CMAKE_SOURCE_DIR}/cmake)
include(FallbackVersion)

# Add 'make dist' command for creating release tarball
set (CPACK_PACKAGE_VERSION ${VERSION})
set (CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.git;/.gitignore;/.gitmodules;~$;${CPACK_SOURCE_IGNORE_FILES}")

# Variables for CPack binary packages
set (CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set (CPACK_PACKAGE_RELEASE "1")
set (CPACK_PACKAGE_CONTACT "Konstantin P. <ria.freelander@gmail.com>")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Vala Panel is GTK+ 3 desktop panel. It is based on Vala and some LXPanel ideas.")
set (CPACK_PACKAGE_DESCRIPTION "Vala Panel is GTK+ 3 desktop panel. It is based on Vala and LXPanel ideas.\nIn fact, it was be a fork of LXPanel in version 0.1.0, but in 0.2.0 it completery rewritten in Vala.\nIt offers same functionality as LXPanel, but:\n1. It has slightly more memory usage\n2. X11 dependency is stripped from panel core (but it is not tested on Wayland right now)\n3. Some of former LXPanel plugins is separate binaries now, and lives in another packages.")
set (CPACK_COMPONENTS_ALL "bin;lib;dev;plugins-base;plugins-wnck")
set (CPACK_COMPONENT_bin_DEPENDS "lib")
set (CPACK_COMPONENT_dev_DEPENDS "lib")
set (CPACK_COMPONENT_plugins-base_DEPENDS "lib")
set (CPACK_COMPONENT_plugins-wnck_DEPENDS "lib")

# Variables and functions for building deb packages
set (CPACK_DEB_COMPONENT_INSTALL ON)
set (CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set (CPACK_DEBIAN_PACKAGE_SECTION "x11")
set (CPACK_DEBIAN_PACKAGE_REVISION ${CPACK_PACKAGE_RELEASE})
string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
if(DPKG_PROGRAM)
   execute_process(COMMAND ${DPKG_PROGRAM} --print-architecture
      OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
      OUTPUT_STRIP_TRAILING_WHITESPACE)
   set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}-${VERSION}-${CPACK_DEBIAN_PACKAGE_REVISION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
else(DPKG_PROGRAM)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}-${VERSION}-${CPACK_DEBIAN_PACKAGE_REVISION}_${CMAKE_SYSTEM_NAME}")
endif(DPKG_PROGRAM)

# Add 'make packages' command for building Debian and RPM packages
set (CPACK_GENERATOR "DEB")
include (CPack)

#Add RPM build support
include (UseRPMTools)
if (RPMTools_FOUND)
   RPMTools_ADD_RPM_TARGETS(package ${CMAKE_SOURCE_DIR}/rpm/vala-panel.spec.in)
endif(RPMTools_FOUND)

add_custom_target (dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
# Configuration
configure_file (${CMAKE_SOURCE_DIR}/vapi/config.vala.in ${CMAKE_BINARY_DIR}/vapi/config.vala)
configure_file (${CMAKE_SOURCE_DIR}/${APPNAME}.pc.cmake ${CMAKE_BINARY_DIR}/${APPNAME}.pc)

# Some definitions
add_definitions(-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\")

# Vala
find_package (Vala REQUIRED)
include (ValaVersion)
ensure_vala_version ("0.28" MINIMUM)
include (ValaPrecompile)

add_definitions(-DVALA_VERSION="${VALA_SHORTVER}")
#add_definitions(-w) #Suppress all GCC warnings
add_definitions(-Wall -Wno-unused # Vala loves to create variables
		-Wno-ignored-qualifiers
		-Wno-deprecated-declarations #Vala warns us already
		-Wno-incompatible-pointer-types
		-Wextra) #Vala often forgot cast to GtkWidget or gpointer
find_package (PkgConfig)

pkg_check_modules (CORE REQUIRED
  gtk+-3.0>=3.16
  glib-2.0>=2.44
  gio-2.0>=2.44
  gio-unix-2.0>=2.44
  gthread-2.0>=2.44
  libpeas-1.0>=1.2
  libpeas-gtk-1.0>=1.2
  gmodule-2.0>=2.44
  libuuid)

set(CORE_PACKAGES
  gtk+-3.0
  glib-2.0
  gio-2.0
  gio-unix-2.0
  libpeas-1.0
  libpeas-gtk-1.0
  gmodule-2.0
  gio-addons-2.0
  posix
  libuuid)

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

ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(app)
ADD_SUBDIRECTORY(applets)
ADD_SUBDIRECTORY(data)

#Additional install
#install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/vala-panel.desktop DESTINATION share/applications)
