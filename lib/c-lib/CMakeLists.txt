include(GResource)
set(ENUM_FILES
        panel-manager.h
        generic-config-dialog.h
        )
add_glib_enumtypes(ENUMSC ENUMSH vala-panel-enums ${ENUM_FILES})
add_library(cvalapanel SHARED
	misc.c
	misc.h
	css.c
	css.h
        launcher.c
        launcher.h
	generic-config-dialog.c
	generic-config-dialog.h
        panel-manager.c
        panel-manager.h
        toplevel.c
        toplevel.h
	${ENUMSC}
	${ENUMSH}
	)
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})
target_link_libraries (cvalapanel ${LIBS} applet-api-new)
target_include_directories(cvalapanel PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties (cvalapanel PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
install (TARGETS cvalapanel DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME} COMPONENT bin)

