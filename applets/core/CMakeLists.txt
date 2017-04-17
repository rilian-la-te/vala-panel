set(VALA_FILES
    kbled.vala
    clock.vala
    dirmenu.vala
    separator.vala
    launchbar.vala
    launchbar-button.vala
    launchbar-config.vala
    menu-maker.vala
    menumodel.vala
    engine-core.vala)
add_custom_target(vala-ide-plugins-core SOURCES ${VALA_FILES})
vala_precompile(VALA_C core
    ${VALA_FILES}
PACKAGES
    ${CORE_PACKAGES}
    vala-panel
    cvalapanel
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/lib
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.40
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/core.gresource.xml
    --thread
)
configure_file(data/plugins/core-plugins.gresource.xml.in ${CMAKE_CURRENT_BINARY_DIR}/data/plugins/core-plugins.gresource.xml)
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
INCLUDE(GResource)
glib_compile_resources(RESOURCES
    source
        core.gresource.xml
)
glib_compile_resources_fullpath(APPLETS
    source
        ${CMAKE_CURRENT_BINARY_DIR}/data/plugins/core-plugins.gresource.xml
)
add_library (core MODULE
    ${VALA_C}
    ${RESOURCES}
    ${APPLETS}
)
target_link_libraries (core ${LIBS} valapanel)
install(TARGETS core DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-base)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/data/plugins/core.plugin DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-base)
