include(GResource)
add_library(appletwidget SHARED
	applet-widget.c
	plugin-interface.c
	applet-info.c
	popup-manager.c
	)
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})
target_link_libraries (appletwidget ${LIBS})
target_include_directories(appletwidget PRIVATE ${CORE_INCLUDE_DIRS})
set_target_properties (appletwidget PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
install (TARGETS appletwidget DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME} COMPONENT bin)

