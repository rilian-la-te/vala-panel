include(GResource)
set(ENUM_FILES
	panel-platform.h
        generic-config-dialog.h
        toplevel.h
        panel-layout.h
        )
add_glib_enumtypes(ENUMSC ENUMSH vala-panel-enums ${ENUM_FILES})
add_library(cvalapanel SHARED

    panel-layout.h
    panel-layout.c
    toplevel.c
    toplevel.h
    applet-manager.c
    applet-engine-module.c
    private.h
#    conf-gsettings.c
#    conf-gsettings.h

	)
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})
target_link_libraries (cvalapanel ${LIBS} ${LIBNAME})
target_include_directories(cvalapanel PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/lib)
set_target_properties (cvalapanel PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
install (TARGETS cvalapanel DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/ COMPONENT bin)
