#####
# DBusMenu
#####
add_subdirectory(dbusmenu)
add_definitions(-fPIC)
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
add_library(sntray-core STATIC ${VALA_C_CORE})
target_link_libraries (sntray-core ${CORE_LIBRARIES} m vala-dbusmenu)
add_custom_target(sntray-core-ide SOURCES ${VALA_FILES_DBUSMENU} ${VALA_FILES_CORE})
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
        ${VALA_C_XFCE}
        ${RESOURCES}
    )
    include_directories(${GTK+_INCLUDE_DIRS} ${XFCE_INCLUDE_DIRS})
    target_link_libraries(sntray-xfce ${GTK+_LIBRARIES} ${XFCE_LIBRARIES} -lm sntray-core)
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
        libpeas-1.0
        vala-panel
    OPTIONS
        --vapidir=${CMAKE_CURRENT_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
	--target-glib=2.44
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}.gresource.xml
        --thread
        ${NEW_DEFINE}
    )
    #####
    # Plugin Library
    #####
    add_library (sntray-valapanel SHARED
        ${VALA_C_VALAPANEL}
        ${RESOURCES}
    )
    include_directories(${GTK+_INCLUDE_DIRS} ${VALAPANEL_INCLUDE_DIRS})
    target_link_libraries(sntray-valapanel ${GTK+_LIBRARIES} ${VALAPANEL_LIBRARIES} -lm sntray-core)
    add_custom_target(sntray-valapanel-ide SOURCES ${VALA_FILES_VALAPANEL})

    # Install plugin stuffs
    install(TARGETS sntray-valapanel DESTINATION ${CMAKE_INSTALL_LIBDIR}/vala-panel/applets)
endif()

#################
# Budgie Panel part
#################
set(VALA_FILES_BUDGIE sntray-applet-budgie.vala)
if (ENABLE_BUDGIE)
    vala_precompile(VALA_C_BUDGIE sntray-budgie
	    ${VALA_FILES_BUDGIE}
    PACKAGES
        gtk+-3.0
        vala-panel-sntray
        budgie-1.0
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
    add_library (sntray-budgie SHARED
        ${VALA_C_BUDGIE}
        ${RESOURCES}
    )
    include_directories(${GTK+_INCLUDE_DIRS} ${BUDGIE_INCLUDE_DIRS})
    target_link_libraries(sntray-budgie ${GTK+_LIBRARIES} ${BUDGIE_LIBRARIES} -lm sntray-core)
    add_custom_target(sntray-budgie-ide SOURCES ${VALA_FILES_BUDGIE})

    # Install plugin stuffs
    install(TARGETS sntray-budgie DESTINATION ${CMAKE_INSTALL_LIBDIR}/budgie-desktop/plugins)
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
    add_executable (sntray-mate
	${VALA_C_MATE}
	${RESOURCES}
    )
    add_dependencies(sntray-mate sntray-core)
    include_directories(${GTK+_INCLUDE_DIRS} ${MATE_INCLUDE_DIRS})
    target_link_libraries(sntray-mate ${GTK+_LIBRARIES} ${MATE_LIBRARIES} -lm sntray-core)
    add_custom_target(sntray-mate-ide SOURCES ${VALA_FILES_MATE})

    # Install plugin stuffs
    install(TARGETS sntray-mate DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/mate-panel)
endif()
