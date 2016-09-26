set(PLUGIN_NAME monitors)
set(VALA_FILES ${PLUGIN_NAME}.vala)
vala_precompile(VALA_C ${PLUGIN_NAME}
    ${VALA_FILES}
PACKAGES
    ${CORE_PACKAGES}
    vala-panel
    posix
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/lib
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.40
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}.gresource.xml
    --thread
)
#####
# Plugin Library
#####

# Build library for plugin
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
set (LIB_CFLAGS ${CORE_CFLAGS})
link_directories (${LIB_PATHS})
include_directories(${CMAKE_SOURCE_DIR})

add_library (${PLUGIN_NAME} MODULE
    ${VALA_C}
    ${RESOURCES}
)

target_link_libraries (${PLUGIN_NAME} ${LIBS} valapanel)
target_include_directories (${PLUGIN_NAME} PRIVATE ${LIB_PATHS} ${CMAKE_BINARY_DIR})
add_custom_target(vala-ide-${PLUGIN_NAME} SOURCES ${VALA_FILES})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-base)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.plugin DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-base)

