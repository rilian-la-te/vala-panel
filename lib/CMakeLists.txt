add_subdirectory(applets-new)
add_subdirectory(c-lib)
include(GLibHelpers)
set(ENUM_FILES
        panel-platform.h
        generic-config-dialog.h
        settings-manager.c
        )
add_glib_enumtypes(ENUMSC ENUMSH vala-panel-enums ${ENUM_FILES})
option(ENABLE_NEW_INTERFACE "Enable new interface function" OFF)
if (ENABLE_NEW_INTERFACE)
    set(NEW_DEFINE -D NEW)
endif()
set(LIBVALAPANEL_HEADERS
    applet-widget-api.h
    applet-widget.h
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    constants.h
    misc.h
    menu-maker.h
    css.h
    launcher.h
    generic-config-dialog.h
    settings-manager.h
    panel-platform.h
    ${ENUMSH})
set(LIBVALAPANEL_C_SOURCES
    misc.c
    menu-maker.c
    css.c
    launcher.c
    generic-config-dialog.c
    settings-manager.c
    panel-platform.c
    applet-widget.c
    ${ENUMSC})
INCLUDE(GResource)
if(GLIB_OLD_FOUND)
    list(APPEND LIBVALAPANEL_C_SOURCES
        guuid.c
        guuid.h)
endif()
set(VALA_FILES
	misc-old.vala
	configurator.vala
	dbusmenu/dbusmenu.vala
	applet-holder.vala
	panel-toplevel.vala
	)
vala_precompile(VALA_C ${LIBNAME}
    ${VALA_FILES}
PACKAGES
    ${CORE_PACKAGES}
    cvalapanel
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.50
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libvalapanel.gresource.xml
    --thread
    ${NEW_DEFINE}
GENERATE_HEADER
    ${APPNAME}-compat
)

add_definitions(-DG_SETTINGS_ENABLE_BACKEND)
glib_compile_resources(GLIB_RESOURCES_LIB
    source
        libvalapanel.gresource.xml
)

#####
# Core Library
#####

# Build library for plugins and application
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)
link_directories (${LIB_PATHS})

set (LIB_FILES ${VALA_C})
add_custom_target(vala-ide-lib SOURCES ${VALA_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/${APPNAME}.vapi)
add_library (${LIBNAME} SHARED
    ${LIB_FILES}
    ${LIBVALAPANEL_C_SOURCES}
    ${LIBVALAPANEL_HEADERS}
    ${GLIB_RESOURCES_LIB}
    definitions.h
)

target_link_libraries (${LIBNAME} ${LIBS})
target_include_directories(${LIBNAME} PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set_target_properties (${LIBNAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})

install (TARGETS ${LIBNAME} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT bin)

# Install lib stuffs
list(APPEND LIBVALAPANEL_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/${APPNAME}-compat.h)

install (FILES ${CMAKE_BINARY_DIR}/${APPNAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/ COMPONENT dev)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${APPNAME}.vapi DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vala/vapi/ COMPONENT dev)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${APPNAME}.deps DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vala/vapi/ COMPONENT dev)
install (FILES ${LIBVALAPANEL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vala-panel/ COMPONENT dev)
