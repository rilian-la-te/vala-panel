set(UTIL_HEADERS
    boxed-wrapper.h
    glistmodel-filter.h
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

#####
# Core Library
#####

add_library (util STATIC
    ${UTIL_SOURCES}
    ${UTIL_HEADERS}
    ${CMAKE_BINARY_DIR}/config.h
    ${GLIB_RESOURCES_LIB}
)

target_link_libraries (util GLIB2::GIO_UNIX GLIB2::GOBJECT)
target_include_directories(util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/util)

set_target_properties (util PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION}
    POSITION_INDEPENDENT_CODE ON)

add_subdirectory(gtk)

install (FILES ${UTIL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vala-panel/ COMPONENT dev)
install (FILES ${CMAKE_BINARY_DIR}/config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vala-panel/ RENAME vala-panel-configuration.h COMPONENT dev)

add_feature_info(Toplevel ON "ValaPanel utility library")
