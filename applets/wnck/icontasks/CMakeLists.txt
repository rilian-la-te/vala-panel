set(PLUGIN_NAME icontasks)
set(VALA_FILES ${PLUGIN_NAME}.vala appsystem.vala buttons.vala)
vala_precompile(VALA_C ${PLUGIN_NAME}
    ${VALA_FILES}
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
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}.gresource.xml
    --thread
)
#####
# Plugin Library
#####

# Build library for plugin
set (LIBS ${CORE_LIBRARIES} ${WNCK_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS} ${WNCK_LIBRARY_DIRS})
set (LIB_CFLAGS ${CORE_CFLAGS} ${WNCK_CFLAGS})
link_directories (${LIB_PATHS})
add_definitions(-DWNCK_I_KNOW_THIS_IS_UNSTABLE)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})


add_library (${PLUGIN_NAME} MODULE
    ${VALA_C}
    ${RESOURCES}
)

target_link_libraries (${PLUGIN_NAME} ${LIBS} valapanel)
target_include_directories (${PLUGIN_NAME} PRIVATE ${CORE_INCLUDE_DIRS} ${WNCK_INCLUDE_DIRS})
add_custom_target(vala-ide-${PLUGIN_NAME} SOURCES ${VALA_FILES})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.plugin DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
