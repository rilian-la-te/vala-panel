#####
# DBusMenu
#####
add_subdirectory(dbusmenu)
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
        utils.vala
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
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}/dbusmenu
	--target-glib=2.44
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/sntray.gresource.xml
        --thread
    GENERATE_VAPI
        vala-panel-sntray
    )
add_library(sntray-core STATIC ${VALA_C_CORE} dbus-utils.c $<TARGET_OBJECTS:vala-dbusmenu-o>)
target_include_directories(sntray-core PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/dbusmenu)
target_link_libraries (sntray-core GLIB2::GIO_UNIX GTK3::GDK GTK3::GTK m)
set_target_properties(sntray-core PROPERTIES
    POSITION_INDEPENDENT_CODE ON)	# Compile this object code position independent.
add_custom_target(sntray-core-ide SOURCES ${VALA_FILES_DBUSMENU} ${VALA_FILES_CORE})
add_subdirectory(panels)

