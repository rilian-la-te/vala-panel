set(PLUGIN_NAME deskno)
vala_precompile(VALA_C ${PLUGIN_NAME}
    ${PLUGIN_NAME}.vala
PACKAGES
    ${CORE_PACKAGES}
    vala-panel
    libwnck-3.0
    gdk-x11-3.0
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/lib
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.40
    --thread
)
#####
# Plugin Library
#####

# Build library for plugin
set (LIBS ${CORE_LIBRARIES} ${WNCK_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS} ${WNCK_LIBRARY_DIRS})
set (LIB_CFLAGS ${CORE_CFLAGS} ${WNCK_CFLAGS} -DWNCK_I_KNOW_THIS_IS_UNSTABLE)

add_library (${PLUGIN_NAME} MODULE
    ${VALA_C}
    ${RESOURCES}
)

add_definitions (${LIB_CFLAGS})
target_link_libraries (${PLUGIN_NAME} ${LIBS})
target_include_directories (${PLUGIN_NAME} PRIVATE ${LIB_PATHS})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGINS_DIRECTORY})
install(FILES ${PLUGIN_NAME}.plugin DESTINATION ${PLUGINS_DIRECTORY})
