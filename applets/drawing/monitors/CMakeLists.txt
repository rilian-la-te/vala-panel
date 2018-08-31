set(PLUGIN_NAME monitors)
#####
# Plugin Library
#####

# Build library for plugin

add_library (${PLUGIN_NAME} MODULE
    monitors.h
    monitors.c
)

target_link_libraries (${PLUGIN_NAME} valapanel)

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-base)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.valapanel.${PLUGIN_NAME}.plugin DESTINATION ${PLUGINS_DATA} COMPONENT plugins-base)

