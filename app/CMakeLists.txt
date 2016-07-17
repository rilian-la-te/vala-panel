if(GLIB242_FOUND)
    set(GLIB242_DEFINE -D GLIB242)
endif()
INCLUDE(GResource)
set(VALA_FILES  application.vala runner.vala)
vala_precompile(VALA_C ${APPNAME}
    ${VALA_FILES}
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
    ${GLIB242_DEFINE}
)

add_definitions(-DG_SETTINGS_ENABLE_BACKEND)
glib_compile_resources(GLIB_RESOURCES_APP
    source
        vala-panel.gresource.xml
)

#####
# Executable
#####
add_custom_target(vala-ide-app SOURCES ${VALA_FILES})
add_executable (${APPNAME} ${VALA_C} ${GLIB_RESOURCES_APP})
target_link_libraries (${APPNAME} ${LIBNAME})

install (TARGETS ${APPNAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin)
