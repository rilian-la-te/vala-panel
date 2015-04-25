set(PLUGIN_NAME xembed)
vala_precompile(VALA_C ${PLUGIN_NAME}
    ${PLUGIN_NAME}.vala
PACKAGES
    ${CORE_PACKAGES}
    vala-panel
    x11
    xembed
    gdk-x11-3.0
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/lib
    --vapidir=${CMAKE_CURRENT_SOURCE_DIR}
    --target-glib=2.40
    --thread
)
#####
# Plugin Library
#####

# Build library for plugin
set (LIBS ${CORE_LIBRARIES} ${X11_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS} ${X11_LIBRARY_DIRS})
set (LIB_CFLAGS ${CORE_CFLAGS} ${X11_CFLAGS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/lib)

add_library (${PLUGIN_NAME} MODULE
    ${VALA_C}
    xembed-ccode.c
    ${RESOURCES}
)

add_definitions (${LIB_CFLAGS})
target_link_libraries (${PLUGIN_NAME} ${LIBS} valapanel)
target_include_directories (${PLUGIN_NAME} PRIVATE ${LIB_PATHS})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.plugin DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
