INCLUDE(GResource)
add_definitions(-DG_SETTINGS_ENABLE_BACKEND)
glib_compile_resources(GLIB_RESOURCES_APP
    source
        vala-panel.gresource.xml
)

#####
# Executable
#####
set(APP_SOURCES
        ${GLIB_RESOURCES_APP}
	vala-panel-platform-standalone-x11.c
	vala-panel-platform-standalone-x11.h
        application.c application.h)
add_executable(${APPNAME}  ${APP_SOURCES} )
target_link_libraries (${APPNAME} ${LIBNAME} ${CORE_LIBRARIES})
target_include_directories(${APPNAME} PRIVATE
    ${CORE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/util/gtk
    ${CMAKE_BINARY_DIR}/ui)

install (TARGETS ${APPNAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin)
