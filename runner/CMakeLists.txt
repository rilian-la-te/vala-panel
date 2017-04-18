INCLUDE(GResource)
set(RUNNER_SOURCES
	runner.c
	runner.h
	info-data.c
	info-data.h
	boxed-wrapper.c
	boxed-wrapper.h
	glistmodel-filter.c
	glistmodel-filter.h
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
	${RUNNER_SOURCES}
	runner-app.c
	runner-app.h
        ${CMAKE_SOURCE_DIR}/lib/css.c
        ${CMAKE_SOURCE_DIR}/lib/launcher.c)
target_link_libraries (${APPNAME}-runner ${CORE_LIBRARIES} m)
target_include_directories(${APPNAME}-runner PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
install (TARGETS ${APPNAME}-runner RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin)
