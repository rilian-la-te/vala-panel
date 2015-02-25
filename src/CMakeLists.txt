set(PLUGIN_NAME sntray)
vala_precompile(VALA_C ${PLUGIN_NAME}
    snwatcher.vala
    snitem.vala
    snhost.vala
    snitembox.vala
    dbusmenu.vala
    snitemproxy.vala
    qrichtextparser.vala
    xfce4-sntray-applet.vala
PACKAGES
    gtk+-3.0
CUSTOM_VAPIS
    vapi/libxfce4panel-2.0.vapi
    vapi/gio-addons-2.0.vapi
OPTIONS
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

include_directories(${GTK+_INCLUDE_DIRS} ${LIBXFCE4PANEL_INCLUDE_DIRS})
target_link_libraries(${PLUGIN_NAME} ${GTK+_LIBRARIES} ${LIBXFCE4PANEL_LIBRARIES})

# Install plugin stuffs
install(TARGETS ${PLUGIN_NAME} DESTINATION lib/xfce4/panel/plugins)
