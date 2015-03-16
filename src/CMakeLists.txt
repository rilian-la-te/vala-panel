set(PLUGIN_NAME sntray)
vala_precompile(VALA_C ${PLUGIN_NAME}
    snwatcher.vala
    snitem.vala
    snhost.vala
    snitembox.vala
    snconfig.vala
    dbusmenu.vala
    snitemproxy.vala
    qrichtextparser.vala
    xfce4-sntray-applet.vala
PACKAGES
    gtk+-3.0
CUSTOM_VAPIS
    vapi/libxfce4panel-2.0.vapi
    vapi/libxfconf-0.vapi
OPTIONS
    --vapidir=${CMAKE_CURRENT_SOURCE_DIR}/vapi
    --target-glib=2.40
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}.gresource.xml
    --thread
)
#####
# Plugin Library
#####
INCLUDE(GResource)
glib_compile_resources(RESOURCES
    source
        ${PLUGIN_NAME}.gresource.xml
)

add_library (${PLUGIN_NAME} SHARED
    ${VALA_C}
    ${RESOURCES}
)

include_directories(${GTK+_INCLUDE_DIRS} ${LIBXFCE4PANEL_INCLUDE_DIRS} ${XFCONF_INCLUDE_DIRS})
target_link_libraries(${PLUGIN_NAME} ${GTK+_LIBRARIES} ${LIBXFCE4PANEL_LIBRARIES} ${XFCONF_LIBRARIES})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION lib/xfce4/panel/plugins)
