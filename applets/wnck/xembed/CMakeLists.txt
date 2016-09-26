set(PLUGIN_NAME xembed)
set(VALA_FILES_SOCKET ${PLUGIN_NAME}-socket.vala)
vala_precompile(VALA_C_SOCKET ${PLUGIN_NAME}-internal
    ${VALA_FILES_SOCKET}
PACKAGES
    ${CORE_PACKAGES}
    vala-panel
    x11
    gdk-x11-3.0
GENERATE_HEADER
    xembed-internal
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/lib
    --vapidir=${CMAKE_CURRENT_SOURCE_DIR}
    --target-glib=2.40
    --thread
)
set(VALA_FILES ${PLUGIN_NAME}.vala)

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
include_directories(${CORE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/lib ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
link_directories (${LIB_PATHS})

add_library (${PLUGIN_NAME} MODULE
    ${VALA_C}
    ${VALA_C_SOCKET}
    xembed-ccode.c
    xembed-ccode.h
    xembed-internal.h
    ${RESOURCES}
)

add_definitions (${LIB_CFLAGS})
target_link_libraries (${PLUGIN_NAME} ${LIBS} valapanel)
add_custom_target(vala-ide-${PLUGIN_NAME} SOURCES ${VALA_FILES} ${VALA_FILES_SOCKET})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.plugin DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
