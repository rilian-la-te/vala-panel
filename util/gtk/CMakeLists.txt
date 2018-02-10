set(ENUM_FILES
        generic-config-dialog.h
        )
add_glib_enumtypes(ENUMSC ENUMSH vala-panel-util-enums ${ENUM_FILES})
set(UTIL_GTK_HEADERS
    css.h
    util-gtk.h
	misc-gtk.h
    generic-config-dialog.h
    menu-maker.h
    launcher-gtk.h
	${ENUMSH}
    )
set(UTIL_GTK_SOURCES
    generic-config-dialog.c
    menu-maker.c
	misc-gtk.c
    css.c
    launcher-gtk.c
	${ENUMSC}
	)
add_library (util-gtk STATIC
    ${UTIL_GTK_SOURCES}
    ${UTIL_GTK_HEADERS}
    ${GLIB_RESOURCES_LIB}
)
set_property(TARGET util-gtk PROPERTY POSITION_INDEPENDENT_CODE ON)
add_dependencies(util-gtk util)
target_link_libraries (util-gtk util GTK3::GTK GLIB2::GIO_UNIX)
target_include_directories(util-gtk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/util ${CMAKE_SOURCE_DIR}/util)

set_target_properties (util-gtk PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION}
    POSITION_INDEPENDENT_CODE ON)

install (FILES ${UTIL_GTK_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vala-panel/ COMPONENT dev)

