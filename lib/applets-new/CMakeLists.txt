include(GResource)
include(GLibHelpers)
add_glib_marshal(MARSHALS MARSHALH "applet-api" g_cclosure_user_marshal)
set(ENUM_FILES
        applet-widget.h)
add_glib_enumtypes(ENUMSC ENUMSH vala-applet-enums ${ENUM_FILES})
add_library(applet-api-new SHARED
	applet-widget.c
	applet-info.c
        popup-manager.c
        applet-engine.c
        applet-api.h
	${MARSHALS}
        ${ENUMSC}
        ${ENUMSH}
	${MARSHALH}
	)
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})
target_link_libraries (applet-api-new ${LIBS})
target_include_directories(applet-api-new PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})
set_target_properties (applet-api-new PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
install (TARGETS applet-api-new DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/ COMPONENT bin)

