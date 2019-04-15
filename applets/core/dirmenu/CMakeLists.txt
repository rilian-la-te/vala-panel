set(PLUGIN_NAME dirmenu)
set(VALA_FILES ${PLUGIN_NAME}.vala)
vala_precompile(VALA_C ${PLUGIN_NAME}
    ${VALA_FILES}
PACKAGES
    ${CORE_PACKAGES}
    posix
    vala-panel
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_SOURCE_DIR}/ui
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.50
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}.gresource.xml
    
)
#####
# Plugin Library
#####

# Build library for plugin
add_library (${PLUGIN_NAME} MODULE
    ${VALA_C}
    ${RESOURCES}
)

target_link_libraries (${PLUGIN_NAME} valapanel)
add_custom_target(vala-ide-${PLUGIN_NAME} SOURCES ${VALA_FILES})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-base)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.valapanel.${PLUGIN_NAME}.plugin DESTINATION ${PLUGINS_DATA} COMPONENT plugins-base)
