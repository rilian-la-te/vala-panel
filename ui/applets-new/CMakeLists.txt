include(GLibProgramHandlers)
add_glib_marshal(MARSHALS MARSHALH "applet-api-marshal" g_cclosure_user_marshal)
add_library(applet-api-new SHARED
        applet-info.c
        applet-engine.c
        applet-api.h
        ${MARSHALS}
        ${ENUMSC}
        ${ENUMSH}
        ${MARSHALH}
	)
target_link_libraries (applet-api-new GLIB2::GOBJECT GLIB2::GIO_UNIX GTK3::GDK GTK3::GTK)
target_include_directories(applet-api-new PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/ui)
set_target_properties (applet-api-new PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
#install (TARGETS applet-api-new DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/ COMPONENT bin)

