set(UTIL_HEADERS
    boxed-wrapper.h
    glistmodel-filter.h
    ${CMAKE_BINARY_DIR}/config.h
    constants.h
    misc.h
    info-data.h
    launcher.h
    util.h
    )
set(UTIL_SOURCES
    boxed-wrapper.c
    glistmodel-filter.c
    misc.c
    info-data.c
    launcher.c)

set(UTIL_GTK_HEADERS
    css.h
    util-gtk.h
    launcher-gtk.h
    )

set(UTIL_GTK_SOURCES
    css.c
    launcher-gtk.c)

INCLUDE(GResource)
if(GLIB_OLD_FOUND)
    list(APPEND LIBVALAPANEL_C_SOURCES
        guuid.c
        guuid.h)
endif()
#glib_compile_resources(GLIB_RESOURCES_LIB
#    source
#        libvalapanel.gresource.xml
#)

#####
# Core Library
#####

add_library (util STATIC
    ${UTIL_SOURCES}
    ${UTIL_HEADERS}
    ${GLIB_RESOURCES_LIB}
)

target_link_libraries (util ${GLIB_LIBRARIES} ${GLIB_GIO_LIBRARIES} ${GLIB_GOBJECT_LIBRARIES} ${GLIB_GMODULE_LIBRARIES} ${GLIB_GTHREAD_LIBRARIES})
target_include_directories(util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/util)

set_target_properties (util PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION}
    POSITION_INDEPENDENT_CODE ON)

add_library (util-gtk STATIC
    ${UTIL_GTK_SOURCES}
    ${UTIL_GTK_HEADERS}
    ${GLIB_RESOURCES_LIB}
)
set_property(TARGET util-gtk PROPERTY POSITION_INDEPENDENT_CODE ON)
add_dependencies(util-gtk util)
target_link_libraries (util-gtk util ${GTK3_LIBRARIES} ${GLIB_LIBRARIES} ${GLIB_GIO_LIBRARIES} ${GLIB_GOBJECT_LIBRARIES} ${GLIB_GMODULE_LIBRARIES} ${GLIB_GTHREAD_LIBRARIES})
target_include_directories(util-gtk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/util)

set_target_properties (util-gtk PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})

install (FILES ${UTIL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vala-panel/ COMPONENT dev)
install (FILES ${UTIL_GTK_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vala-panel/ COMPONENT dev)

