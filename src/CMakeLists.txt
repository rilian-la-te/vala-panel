#####
# DBusMenu
#####
set(VALA_FILES_DBUSMENU  dbusmenu/dbusmenu.vala)
macro(precompile_dbusmenu)
vala_precompile(VALA_C_DBUSMENU vala-dbusmenu
    ${VALA_FILES_DBUSMENU}
PACKAGES
    gtk+-3.0
    gio-2.0
    glib-2.0
    gobject-2.0
    gio-addons-2.0
OPTIONS
    --vapidir=${CMAKE_CURRENT_SOURCE_DIR}/vapi
    --target-glib=2.44
    --thread
GENERATE_VAPI
    vala-dbusmenu
GENERATE_HEADER
    vala-dbusmenu
)
endmacro(precompile_dbusmenu)
if(NOT EXISTS ${CMAKE_BINARY_DIR}/vapi/vala-dbusmenu.vapi
   OR ${CMAKE_CURRENT_BINARY_DIR}/vala-dbusmenu_valac.stamp IS_NEWER_THAN ${CMAKE_BINARY_DIR}/vapi/vala-dbusmenu.vapi)
   precompile_dbusmenu()
endif()
add_custom_target(vala-dbusmenu DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/vala-dbusmenu_valac.stamp)
#####
# Core Parts (GResource, definitions)
#####
set(VALA_FILES_CORE
	snwatcher.vala
	snitem.vala
	snhost.vala
	snitembox.vala
	snconfig.vala
	snitemproxy.vala
	qrichtextparser.vala
	${CMAKE_CURRENT_BINARY_DIR}/config.vala
	)
macro(precompile_core)
    vala_precompile(VALA_C_CORE sntray-core
	${VALA_FILES_CORE}
    PACKAGES
        gtk+-3.0
        vala-dbusmenu
    OPTIONS
        --vapidir=${CMAKE_CURRENT_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
	--target-glib=2.44
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/sntray.gresource.xml
        --thread
    GENERATE_VAPI
        vala-panel-sntray
    )
    INCLUDE(GResource)
    glib_compile_resources(RESOURCES
        source
            sntray.gresource.xml
    )
endmacro(precompile_core)
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/vala-panel-sntray.vapi
   OR ${CMAKE_CURRENT_BINARY_DIR}/sntray-core_valac.stamp IS_NEWER_THAN ${CMAKE_CURRENT_BINARY_DIR}/vala-panel-sntray.vapi)
   precompile_core()
endif()
add_custom_target(sntray-core DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sntray.gresource.c ${CMAKE_CURRENT_BINARY_DIR}/sntray-core_valac.stamp)
add_custom_target(sntray-core-ide SOURCES ${VALA_FILES_DBUSMENU} ${VALA_FILES_CORE})
add_dependencies(sntray-core vala-dbusmenu)
#################
# Xfce Panel part
#################
set(VALA_FILES_XFCE xfce4-sntray-applet.vala)
if (ENABLE_XFCE)
    vala_precompile(VALA_C_XFCE sntray-xfce
	${VALA_FILES_XFCE}
    PACKAGES
        gtk+-3.0
        vala-panel-sntray
    CUSTOM_VAPIS
        vapi/libxfce4panel-2.0.vapi
        vapi/libxfconf-0.vapi
    OPTIONS
        --vapidir=${CMAKE_CURRENT_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
	--target-glib=2.44
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}.gresource.xml
        --thread
    )
    #####
    # Plugin Library
    #####
    add_library (sntray-xfce SHARED
        ${VALA_C_DBUSMENU}
        ${VALA_C_CORE}
        ${VALA_C_XFCE}
        ${RESOURCES}
    )
    add_dependencies(sntray-xfce sntray-core)
    include_directories(${GTK+_INCLUDE_DIRS} ${XFCE_INCLUDE_DIRS})
    target_link_libraries(sntray-xfce ${GTK+_LIBRARIES} ${XFCE_LIBRARIES} -lm)
    add_custom_target(sntray-xfce-ide SOURCES ${VALA_FILES_XFCE})

    # Install plugin stuffs
    install(TARGETS sntray-xfce DESTINATION ${CMAKE_INSTALL_LIBDIR}/xfce4/panel/plugins)
endif()


#################
# Vala Panel part
#################
set(VALA_FILES_VALAPANEL vala-panel-sntray-applet.vala)
if (ENABLE_VALAPANEL)
    vala_precompile(VALA_C_VALAPANEL sntray-valapanel
	    ${VALA_FILES_VALAPANEL}
    PACKAGES
        gtk+-3.0
        vala-panel-sntray
        vala-panel
    OPTIONS
        --vapidir=${CMAKE_CURRENT_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
	--target-glib=2.44
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}.gresource.xml
        --thread
    )
    #####
    # Plugin Library
    #####
    add_library (sntray-valapanel SHARED
        ${VALA_C_CORE}
        ${VALA_C_VALAPANEL}
        ${RESOURCES}
    )
    add_dependencies(sntray-valapanel sntray-core)
    include_directories(${GTK+_INCLUDE_DIRS} ${VALAPANEL_INCLUDE_DIRS})
    target_link_libraries(sntray-valapanel ${GTK+_LIBRARIES} ${VALAPANEL_LIBRARIES} -lm)
    add_custom_target(sntray-valapanel-ide SOURCES ${VALA_FILES_VALAPANEL})

    # Install plugin stuffs
    install(TARGETS sntray-valapanel DESTINATION ${CMAKE_INSTALL_LIBDIR}/vala-panel/applets)
endif()
#################
# MATE Panel part
#################
set(VALA_FILES_MATE mate-sntray-applet.vala)
if (ENABLE_MATE)
    vala_precompile(VALA_C_MATE sntray-mate
	${VALA_FILES_MATE}
    PACKAGES
	gdk-3.0
	gtk+-3.0
	vala-panel-sntray
    CUSTOM_VAPIS
	vapi/libmatepanelapplet-4.0.vapi
    OPTIONS
	--vapidir=${CMAKE_CURRENT_SOURCE_DIR}/vapi
	--vapidir=${CMAKE_CURRENT_BINARY_DIR}
	--target-glib=2.44
	--gresources=${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}.gresource.xml
	--thread
    )
    #####
    # Plugin Library
    #####
    add_library (sntray-mate SHARED
	${VALA_C_DBUSMENU}
	${VALA_C_CORE}
	${VALA_C_MATE}
	${RESOURCES}
    )
    add_dependencies(sntray-mate sntray-core)
    include_directories(${GTK+_INCLUDE_DIRS} ${MATE_INCLUDE_DIRS})
    target_link_libraries(sntray-mate ${GTK+_LIBRARIES} ${MATE_LIBRARIES} -lm)
    add_custom_target(sntray-mate-ide SOURCES ${VALA_FILES_MATE})

    # Install plugin stuffs
    install(TARGETS sntray-mate DESTINATION ${CMAKE_INSTALL_LIBDIR}/mate-panel)
endif()
