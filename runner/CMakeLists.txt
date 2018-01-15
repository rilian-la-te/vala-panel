INCLUDE(GResource)
set(RUNNER_SOURCES
	runner.c
	runner.h
        runner-app.c
        runner-app.h
	)
add_definitions(-DG_SETTINGS_ENABLE_BACKEND)
glib_compile_resources(GLIB_RESOURCES_APP
    source
        runner.gresource.xml
)

#####
# Executable
#####
add_executable(${APPNAME}-runner
	${GLIB_RESOURCES_APP}
	${RUNNER_SOURCES})
add_dependencies(${APPNAME}-runner util-gtk)
target_link_libraries(${APPNAME}-runner util-gtk)
target_link_libraries (${APPNAME}-runner ${GTK3_LIBRARIES} ${GLIB_LIBRARIES} ${GLIB_GIO_LIBRARIES} ${GLIB_GOBJECT_LIBRARIES} ${GLIB_GMODULE_LIBRARIES} ${GLIB_GTHREAD_LIBRARIES} m)
target_include_directories(${APPNAME}-runner PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/util ${CMAKE_BINARY_DIR})
install (TARGETS ${APPNAME}-runner RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin)
