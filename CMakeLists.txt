project(xfce4-sntray-plugin)
cmake_minimum_required(VERSION 2.8)
cmake_policy (VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/vala)

include(ValaPrecompile)
include(GResource)
include(GNUInstallDirs)

find_package(Vala)
find_package(PkgConfig)
set (GETTEXT_PACKAGE "xfce4-sntray-plugin")
set (VERSION "0.3.8")
add_definitions(-w) # Disable gcc warnings
add_definitions(-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\")
pkg_check_modules(GTK+ REQUIRED gtk+-3.0)

option(ENABLE_XFCE "Enable xfce4-panel plugin" ON)
if (ENABLE_XFCE)
    pkg_check_modules(XFCE libxfce4panel-2.0 libxfconf-0)
    if (NOT XFCE_FOUND)
        set(ENABLE_XFCE OFF)
    endif()
endif()

option(ENABLE_VALAPANEL "Enable vala-panel plugin" ON)
if (ENABLE_VALAPANEL)
    pkg_check_modules(VALAPANEL vala-panel libpeas-1.0)
    if (NOT XFCE_FOUND)
        set(ENABLE_VALAPANEL OFF)
    endif()
endif()

# Configuration
configure_file (${CMAKE_SOURCE_DIR}/src/config.vala.in ${CMAKE_BINARY_DIR}/src/config.vala)

add_subdirectory(po)
add_subdirectory(data)
add_subdirectory(src)

set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PROJECT_NAME}-0.1)
set(CPACK_SOURCE_GENERATOR TXZ)
set(CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.kdev4;temp)
set(CPACK_PACKAGE_VERSION ${VERSION})
INCLUDE(CPack)
