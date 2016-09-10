INCLUDE(GResource)
add_definitions(-DG_SETTINGS_ENABLE_BACKEND)
glib_compile_resources(GLIB_RESOURCES_LIB
    source
        runner.gresource.xml
)
add_executable(${APPNAME}-runner ${GLIB_RESOURCES_LIB} runner.c runner.h runner-app.c runner-app.h ${CMAKE_SOURCE_DIR}/lib/c-lib/css.c ${CMAKE_SOURCE_DIR}/lib/c-lib/launcher.c)
target_link_libraries (${APPNAME}-runner ${CORE_LIBRARIES} m)
target_include_directories(${APPNAME}-runner PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})
install (TARGETS ${APPNAME}-runner RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin)
