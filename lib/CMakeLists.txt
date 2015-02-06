INCLUDE(GResource)
vala_precompile(VALA_C
	misc.vala
    misc.vala
    configurator.vala
    generic-config-dialog.vala
    css.vala
    icon-grid.vala
    menu-maker.vala
    panel-plugin.vala
    panel-toplevel.vala
    plugin-settings.vala
    ${CMAKE_BINARY_DIR}/vapi/config.vala
PACKAGES
    ${CORE_PACKAGES}
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.40
    --gresources=${CMAKE_SOURCE_DIR}/lib/libvalapanel.gresource.xml
    --thread
GENERATE_VAPI
    ${APPNAME}
GENERATE_HEADER
    ${APPNAME}
)

add_definitions(-DG_SETTINGS_ENABLE_BACKEND)
glib_compile_resources(GLIB_RESOURCES_LIB
	source
    	libvalapanel.gresource.xml
)

#####
# Core Library
#####

# Bild library for plugins and application
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})

set (LIB_FILES ${VALA_C})

add_library (${LIBNAME} SHARED
    ${LIB_FILES}
    ${GLIB_RESOURCES_LIB}
)

target_link_libraries (${LIBNAME} ${LIBS})

set_target_properties (${LIBNAME} PROPERTIES
    VERSION ${LIBVERSION}.0
    SOVERSION ${SOVERSION})

install (TARGETS ${LIBNAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Install lib stuffs
install (FILES ${CMAKE_BINARY_DIR}/${APPNAME}.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPNAME}.vapi DESTINATION share/vala/vapi/)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${APPNAME}.deps DESTINATION share/vala/vapi/)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPNAME}.h DESTINATION include/vala-panel/)
