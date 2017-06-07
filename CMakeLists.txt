project(xfce4-sntray-plugin C)
cmake_minimum_required(VERSION 2.8)
cmake_policy (VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/vala)

include(ValaPrecompile)
include(GResource)
include(GNUInstallDirs)

find_package(Vala)
find_package(PkgConfig)
set (GETTEXT_PACKAGE "xfce4-sntray-plugin")
include(GitVersion)
determine_fallback_version(${CMAKE_SOURCE_DIR}/cmake/vala)
include(FallbackVersion)

add_definitions(-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\")
add_definitions(-Wall -Wextra
                -Wno-unused
                -Wno-unused-parameter# Vala loves to create variables
#				-pedantic
                -Wno-ignored-qualifiers
                -Wno-incompatible-pointer-types
                -Wno-missing-field-initializers
                -Wno-discarded-qualifiers
                -Wno-sign-compare
                ) #Vala often forgot cast to GtkWidget or gpointer
pkg_check_modules(GTK+ REQUIRED gtk+-3.0>=3.22)

include(CMakeDependentOption)

#set build options
pkg_check_modules(XFCE libxfce4panel-2.0 libxfconf-0)
cmake_dependent_option(ENABLE_XFCE "Enable Xfce Panel Integration" ON
    "XFCE_FOUND" OFF)

pkg_check_modules(MATE libmatepanelapplet-4.0)
cmake_dependent_option(ENABLE_MATE "Enable mate-panel Integration" ON
    "MATE_FOUND" OFF)

pkg_check_modules(PEAS libpeas-1.0)
pkg_check_modules(VALAPANEL vala-panel)
pkg_check_modules(VP_NEW vala-panel>=0.3.50 QUIET)
pkg_check_modules(BUDGIE budgie-1.0)

cmake_dependent_option(ENABLE_VALAPANEL "Enable Vala Panel Integration" ON
    "VALAPANEL_FOUND; PEAS_FOUND" OFF)
if (VP_NEW_FOUND)
    message("Using 0.4 ValaPanelApplet api")
    set(NEW_DEFINE -D NEW)
    set(NEW_IFACE ON)
endif()

cmake_dependent_option(ENABLE_BUDGIE "Enable budgie-desktop plugin" ON
    "BUDGIE_FOUND; PEAS_FOUND" OFF)

# Configuration
configure_file (${CMAKE_SOURCE_DIR}/src/config.vala.in ${CMAKE_BINARY_DIR}/src/config.vala)

add_subdirectory(po)
add_subdirectory(data)
add_subdirectory(src)

# Add 'make dist' command for creating release tarball
set (CPACK_PACKAGE_VERSION ${VERSION})
set (CPACK_PACKAGE_RELEASE "1")
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.git;/.gitignore;/.gitmodules;~$;${CPACK_SOURCE_IGNORE_FILES}")

include (CPack)
add_custom_target (dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
#Add RPM build support
include (UseRPMTools)
if (RPMTools_FOUND)
   RPMTools_ADD_RPM_TARGETS(package ${CMAKE_SOURCE_DIR}/rpm/xfce4-sntray-plugin.spec.in)
endif(RPMTools_FOUND)
