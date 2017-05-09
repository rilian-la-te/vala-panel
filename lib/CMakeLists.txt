add_subdirectory(applets-new)
add_subdirectory(c-lib)
include(GLibHelpers)
set(ENUM_FILES
        panel-platform.h
        generic-config-dialog.h
        )
add_glib_enumtypes(ENUMSC ENUMSH vala-panel-enums ${ENUM_FILES})
set(LIBVALAPANEL_C_SOURCES
    misc.c
    misc.h
    menu-maker.h
    menu-maker.c
    css.c
    css.h
    launcher.c
    launcher.h
    generic-config-dialog.c
    generic-config-dialog.h
    settings-manager.c
    settings-manager.h
    panel-platform.c
    panel-platform.h
    ${ENUMSC}
    ${ENUMSH})
INCLUDE(GResource)
if(GLIB_OLD_FOUND)
    list(APPEND LIBVALAPANEL_C_SOURCES
        guuid.c
        guuid.h)
endif()
set(VALA_FILES
        misc-old.vala
	configurator.vala
	dbusmenu/dbusmenu.vala
	panel-plugin.vala
	panel-toplevel.vala
	plugin-settings.vala
	)
vala_precompile(VALA_C ${LIBNAME}
    ${VALA_FILES}
    ${CMAKE_BINARY_DIR}/vapi/config.vala
PACKAGES
    ${CORE_PACKAGES}
    cvalapanel
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.40
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libvalapanel.gresource.xml
    --thread
GENERATE_VAPI
    ${APPNAME}
GENERATE_HEADER
    ${APPNAME}-compat
)

add_definitions(-DG_SETTINGS_ENABLE_BACKEND)
glib_compile_resources(GLIB_RESOURCES_LIB
    source
        libvalapanel.gresource.xml
)

#####
# Core Library
#####

# Build library for plugins and application
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)
link_directories (${LIB_PATHS})

set (LIB_FILES ${VALA_C})
add_custom_target(vala-ide-lib SOURCES ${VALA_FILES})
add_library (${LIBNAME} SHARED
    ${LIB_FILES}
    ${LIBVALAPANEL_C_SOURCES}
    ${GLIB_RESOURCES_LIB}
    definitions.h
)

target_link_libraries (${LIBNAME} ${LIBS})
target_include_directories(${LIBNAME} PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set_target_properties (${LIBNAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})

install (TARGETS ${LIBNAME} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT bin)

# Install lib stuffs
install (FILES ${CMAKE_BINARY_DIR}/${APPNAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/ COMPONENT dev)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${APPNAME}.vapi DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vala/vapi/ COMPONENT dev)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${APPNAME}.deps DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vala/vapi/ COMPONENT dev)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPNAME}-compat.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vala-panel/ COMPONENT dev RENAME ${APPNAME}.h)
