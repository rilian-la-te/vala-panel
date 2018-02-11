set(PLUGIN_NAME deskno)
set(VALA_FILES ${PLUGIN_NAME}.vala)
vala_precompile(VALA_C ${PLUGIN_NAME}
    ${VALA_FILES}
PACKAGES
    ${CORE_PACKAGES}
    vala-panel
    libwnck-3.0
    gdk-x11-3.0
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_SOURCE_DIR}/ui
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.50
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}.gresource.xml
    --thread
)
#####
# Plugin Library
#####

# Build library for plugin
add_library (${PLUGIN_NAME} MODULE
    ${VALA_C}
    ${RESOURCES}
)

target_link_libraries (${PLUGIN_NAME} WNCK::WNCK valapanel)
add_custom_target(vala-ide-${PLUGIN_NAME} SOURCES ${VALA_FILES})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.plugin DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
