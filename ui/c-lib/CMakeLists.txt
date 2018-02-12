include(GLibProgramHandlers)
set(ENUM_FILES
        toplevel.h
        panel-layout.h
        )
add_glib_enumtypes(ENUMSC ENUMSH vala-panel-enums ${ENUM_FILES})
add_library(cvalapanel SHARED
    panel-layout.h
    panel-layout.c
    applet-manager.c
    applet-engine-module.c
    private.h)
target_link_libraries (cvalapanel GLIB2::GOBJECT GLIB2::GIO_UNIX GTK3::GDK GTK3::GTK)
target_include_directories(cvalapanel PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/ui/applets-new)
set_target_properties (cvalapanel PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
#install (TARGETS cvalapanel DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/ COMPONENT bin)
