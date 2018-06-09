include(GLibProgramHandlers)
add_library(applet-api-new SHARED
        applet-plugin.h
        applet-plugin.c
        applet-info.c
        applet-info.h
	)
target_link_libraries (applet-api-new GLIB2::GOBJECT GLIB2::GIO_UNIX GTK3::GDK GTK3::GTK)
target_include_directories(applet-api-new PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/ui ${CMAKE_SOURCE_DIR}/util ${CMAKE_BINARY_DIR})
set_target_properties (applet-api-new PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
#install (TARGETS applet-api-new DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/ COMPONENT bin)

