INCLUDE(GResource)
vala_precompile(VALA_C ${APPNAME}
    application.vala
    runner.vala
    ${CMAKE_BINARY_DIR}/vapi/config.vala
PACKAGES
    ${CORE_PACKAGES}
    vala-panel
    posix
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/lib
    --target-glib=2.40
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/vala-panel.gresource.xml
    --thread
)

add_definitions(-DG_SETTINGS_ENABLE_BACKEND)
glib_compile_resources(GLIB_RESOURCES_APP
	source
    	vala-panel.gresource.xml
)

#####
# Executable
#####

add_executable (${APPNAME} ${VALA_C} ${GLIB_RESOURCES_APP})
target_link_libraries (${APPNAME} ${LIBNAME})

install (TARGETS ${APPNAME} RUNTIME DESTINATION bin)
