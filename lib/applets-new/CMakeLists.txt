include(GResource)
add_library(applet-api-new SHARED
	applet-widget.c
	plugin-interface.h
	applet-info.c
	popup-manager.h
	)
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})
target_link_libraries (applet-api-new ${LIBS})
target_include_directories(applet-api-new PRIVATE ${CORE_INCLUDE_DIRS})
set_target_properties (applet-api-new PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
install (TARGETS applet-api-new DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME} COMPONENT bin)

