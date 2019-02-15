set(PLUGIN_NAME tasklist-xfce)

#####
# Plugin Library
#####

# Build library for plugin
add_library (${PLUGIN_NAME} MODULE
    tasklist.c
    tasklist.h
    tasklist-widget.c
    tasklist-widget.h
    xfce-arrow-button.c
    xfce-arrow-button.h
    ${RESOURCES}
)

target_link_libraries (${PLUGIN_NAME} WNCK::WNCK valapanel)
target_include_directories (${PLUGIN_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(vala-ide-${PLUGIN_NAME} SOURCES ${VALA_FILES})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.xfce.tasklist.plugin DESTINATION ${PLUGINS_DATA} COMPONENT plugins-wnck)
