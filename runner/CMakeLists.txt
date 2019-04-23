INCLUDE(GLibProgramHandlers)
set(RUNNER_SOURCES
    info-data.c
    info-data.h
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

add_feature_info(Runner ON "Simple run dialog based on ValaPanel library")

add_executable(${APPNAME}-runner
	${GLIB_RESOURCES_APP}
	${RUNNER_SOURCES})
add_dependencies(${APPNAME}-runner util-gtk)
target_link_libraries(${APPNAME}-runner util-gtk GTK3::GTK GLIB2::GIO_UNIX GLIB2::GOBJECT m)
target_include_directories(${APPNAME}-runner PRIVATE ${CMAKE_SOURCE_DIR}/util ${CMAKE_SOURCE_DIR}/util/gtk ${CMAKE_BINARY_DIR})
install (TARGETS ${APPNAME}-runner RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runner)
