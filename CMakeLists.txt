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

option(ENABLE_XFCE "Enable xfce4-panel plugin" ON)
if (ENABLE_XFCE)
    pkg_check_modules(XFCE libxfce4panel-2.0 libxfconf-0)
    if (NOT XFCE_FOUND)
        set(ENABLE_XFCE OFF)
    endif()
endif()

option(ENABLE_MATE "Enable mate-panel plugin" ON)
if (ENABLE_MATE)
    pkg_check_modules(MATE libmatepanelapplet-4.0)
    if (NOT MATE_FOUND)
	set(ENABLE_MATE OFF)
    endif()
endif()

option(ENABLE_VALAPANEL "Enable vala-panel plugin" ON)
if (ENABLE_VALAPANEL)
    pkg_check_modules(VALAPANEL vala-panel libpeas-1.0)
    if (NOT VALAPANEL_FOUND)
        set(ENABLE_VALAPANEL OFF)
    endif()
    pkg_check_modules(VP_NEW vala-panel>=0.3.50)
    if (VP_NEW_FOUND)
        set(NEW_DEFINE -D NEW)
        set(NEW_IFACE ON)
    endif()
endif()

option(ENABLE_BUDGIE "Enable budgie-desktop plugin" ON)
if (ENABLE_BUDGIE)
    pkg_check_modules(BUDGIE budgie-1.0 libpeas-1.0)
    if (NOT BUDGIE_FOUND)
        set(ENABLE_BUDGIE OFF)
    endif()
endif()

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
