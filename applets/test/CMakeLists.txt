vala_precompile(VALA_C
	test.vala
    test.vala
PACKAGES
    ${CORE_PACKAGES}
    vala-panel
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/lib
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.40
    --gresources=${CMAKE_SOURCE_DIR}/lib/libvalapanel.gresource.xml
    --thread
)
#####
# Plugin Library
#####

# Bild library for plugins and application
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})

add_library (testapplet MODULE
    ${VALA_C}
)

target_link_libraries (testapplet ${LIBS})

# Install plugin stuffs
install(TARGETS testapplet DESTINATION ${PLUGINS_DIRECTORY})
install(FILES test.plugin DESTINATION ${PLUGINS_DIRECTORY})
