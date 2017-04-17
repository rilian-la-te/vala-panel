option(ENABLE_WNCK "Enables wnck-related plugins, like taskbar and pager" ON)

set(VALA_FILES
    buttons.vala
    deskno.vala
    pager.vala
    tasklist.vala
    wincmd.vala
    appsystem.vala
    buttons-icontasks.vala
    icontasks.vala
    engine-wnck.vala)
add_custom_target(vala-ide-plugins-wnck SOURCES ${VALA_FILES})
if (ENABLE_WNCK)
    pkg_check_modules(WNCK libwnck-3.0)
    if (NOT(WNCK_FOUND))
        set(ENABLE_WNCK OFF)
    endif()
endif()

if (ENABLE_WNCK)
    vala_precompile(VALA_C wnck
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
    add_definitions(-DWNCK_I_KNOW_THIS_IS_UNSTABLE)
    link_directories (${LIB_PATHS})
    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
    configure_file(data/plugins/wnck-plugins.gresource.xml.in ${CMAKE_CURRENT_BINARY_DIR}/data/plugins/wnck-plugins.gresource.xml)
    glib_compile_resources_fullpath(APPLETS
        source
            ${CMAKE_CURRENT_BINARY_DIR}/data/plugins/wnck-plugins.gresource.xml
    )
    add_library (wnck MODULE
        ${VALA_C}
        ${APPLETS}
    )

    target_link_libraries (wnck ${LIBS} valapanel)
    target_include_directories (wnck PRIVATE ${CORE_INCLUDE_DIRS} ${WNCK_INCLUDE_DIRS} )

    # Install plugin stuffs
    install(TARGETS wnck DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/wnck.plugin DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-wnck)
endif()

option(ENABLE_X11 "Enables X11 system tray" ON)

if (ENABLE_X11)
    pkg_check_modules(X11 x11 gdk-x11-3.0)
    if (NOT(X11_FOUND))
        set(ENABLE_X11 OFF)
    endif()
endif()

if (ENABLE_X11)
    add_subdirectory(xembed)
endif()
