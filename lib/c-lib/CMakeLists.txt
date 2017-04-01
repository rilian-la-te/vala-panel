include(GResource)
set(ENUM_FILES
	panel-platform.h
        generic-config-dialog.h
        toplevel.h
        panel-layout.h
        )
add_glib_enumtypes(ENUMSC ENUMSH vala-panel-enums ${ENUM_FILES})
add_library(cvalapanel SHARED
	misc.c
	misc.h
    menu-maker.h
    menu-maker.c
	css.c
	css.h
    launcher.c
    launcher.h
	generic-config-dialog.c
	generic-config-dialog.h
	panel-platform.c
	panel-platform.h
    panel-layout.h
    panel-layout.c
    toplevel.c
    toplevel.h
    applet-manager.c
    applet-engine-module.c
    private.h
#    conf-gsettings.c
#    conf-gsettings.h
	${ENUMSC}
	${ENUMSH}
	)
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})
target_link_libraries (cvalapanel ${LIBS} applet-api-new)
target_include_directories(cvalapanel PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties (cvalapanel PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
install (TARGETS cvalapanel DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/ COMPONENT bin)
