INCLUDE(GResource)
set(VALA_FILES  application.vala)
vala_precompile(VALA_C ${APPNAME}
    ${VALA_FILES}
PACKAGES
    ${CORE_PACKAGES}
    cvalapanel
    vala-panel
    posix
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/lib
    --target-glib=2.50
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/vala-panel.gresource.xml
    --thread
    -D GLIB242
)

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
        vala-panel.gresource.xml
        runner.gresource.xml
)

#####
# Executable
#####
set(APP_SOURCES
	${GLIB_RESOURCES_APP}
	vala-panel-platform-standalone-x11.c
	vala-panel-platform-standalone-x11.h)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_PROJECT_NAME}/")
add_custom_target(vala-ide-app SOURCES ${VALA_FILES})
add_executable (${APPNAME} ${VALA_C} ${APP_SOURCES})
add_executable(${APPNAME}-new  ${APP_SOURCES} application-new.c application-new.h)
target_link_libraries (${APPNAME} ${LIBNAME} ${CORE_LIBRARIES})
target_link_libraries (${APPNAME}-new ${CORE_LIBRARIES} ${LIBNAME})
target_include_directories(${APPNAME} PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/lib/c-lib ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/lib)
target_include_directories(${APPNAME}-new PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/lib/c-lib ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/lib)

install (TARGETS ${APPNAME} ${APPNAME}-new RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin)

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
