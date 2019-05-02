project(xfce4-sntray-plugin C)
cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
cmake_policy (VERSION 3.6)

include(GNUInstallDirs)

set (GETTEXT_PACKAGE ${CMAKE_PROJECT_NAME})
if(EXISTS ${CMAKE_SOURCE_DIR}/cmake/VCMSubmodule.cmake)
    include(${CMAKE_SOURCE_DIR}/cmake/VCMSubmodule.cmake)
    use_vcm(${CMAKE_SOURCE_DIR}/cmake)
else()
    find_package(VCM REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH ${VCM_MODULE_PATH})
endif()
include(GitVersion)
determine_fallback_version(${CMAKE_SOURCE_DIR}/cmake)
include(${CMAKE_SOURCE_DIR}/cmake/FallbackVersion.cmake)
include(Format)

find_package (PkgConfig)
find_package(GLIB2 2.50 REQUIRED COMPONENTS GOBJECT GIO GIO_UNIX GMODULE GTHREAD CODEGEN MKENUMS GENMARSHAL COMPILE_SCHEMAS COMPILE_RESOURCES)
include(GLibProgramHandlers)
find_package(GTK 3.22 REQUIRED COMPONENTS GDK X11 WAYLAND)
find_package(Vala 0.36 REQUIRED)
include(ValaPrecompile)


add_definitions(-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\")
add_definitions(-Wall -Wextra
                -Wno-unused
                -Wno-unused-parameter# Vala loves to create variables
#				-pedantic
                -Wno-ignored-qualifiers
                -Wno-incompatible-pointer-types
                -Wno-missing-field-initializers
                -Wno-sign-compare
                ) #Vala often forgot cast to GtkWidget or gpointer
include(CMakeDependentOption)
include(FeatureSummary)

#set build options
pkg_check_modules(XFCE libxfce4panel-2.0 libxfconf-0 IMPORTED_TARGET)
cmake_dependent_option(ENABLE_XFCE "Enable Xfce Panel Integration" ON
    "XFCE_FOUND" OFF)
add_feature_info(Xfce ENABLE_XFCE "Xfce Panel Integration")

pkg_check_modules(MATE libmatepanelapplet-4.0>=1.20 IMPORTED_TARGET)
cmake_dependent_option(ENABLE_MATE "Enable mate-panel Integration" ON
    "MATE_FOUND" OFF)
add_feature_info(MATE ENABLE_MATE "MATE Panel Integration")

find_package(PEAS 1.14 COMPONENTS PEAS)
pkg_check_modules(VALAPANEL vala-panel>=0.4.89 IMPORTED_TARGET)
pkg_check_modules(BUDGIE budgie-1.0>=2 IMPORTED_TARGET)

cmake_dependent_option(ENABLE_VALAPANEL "Enable Vala Panel Integration" ON
    "VALAPANEL_FOUND" OFF)
if (VALAPANEL_FOUND)
    add_feature_info(ValaPanel ENABLE_VALAPANEL "Vala Panel Integration - 0.5.x")
endif()

cmake_dependent_option(ENABLE_BUDGIE "Enable budgie-desktop plugin" ON
    "BUDGIE_FOUND; PEAS_FOUND" OFF)
add_feature_info(Budgie ENABLE_BUDGIE "Budgie Desktop Integration")

# Configuration
configure_file (${CMAKE_SOURCE_DIR}/src/config.vala.in ${CMAKE_BINARY_DIR}/src/config.vala)
# Common include dir
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(data)
add_subdirectory(po)
add_subdirectory(src)
#add_subdirectory(xembedsniproxy)

# Add 'make dist' command for creating release tarball
set (CPACK_PACKAGE_VERSION ${VERSION})
set (CPACK_PACKAGE_RELEASE "1")
set (CPACK_SOURCE_GENERATOR "TXZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.git;/.gitignore;/.gitmodules;~$;${CPACK_SOURCE_IGNORE_FILES}")

include (CPack)
add_custom_target (dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
#Add RPM build support
include (UseRPMTools)
if (RPMTools_FOUND)
   RPMTools_ADD_RPM_TARGETS(package ${CMAKE_SOURCE_DIR}/rpm/xfce4-sntray-plugin.spec.in)
endif(RPMTools_FOUND)

feature_summary(WHAT ALL)
