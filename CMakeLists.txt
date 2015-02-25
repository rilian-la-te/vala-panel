project(xfce4-sntray-plugin)
cmake_minimum_required(VERSION 2.8)
cmake_policy (VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/vala)

include(ValaPrecompile)
include(GResource)

find_package(Vala)
find_package(PkgConfig)

add_definitions(-w) # Disable gcc warnings
pkg_check_modules(GTK+ REQUIRED gtk+-3.0)
pkg_check_modules(LIBXFCE4PANEL REQUIRED libxfce4panel-2.0)

add_subdirectory(data)
add_subdirectory(src)

set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PROJECT_NAME}-0.1)
set(CPACK_SOURCE_GENERATOR TXZ)
set(CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
INCLUDE(CPack)
