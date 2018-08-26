pkg_check_modules(XCB
    xcb-atom
    xcb-aux
    xcb-composite
    xcb-damage
    xcb-dpms
    xcb-dri2
    xcb-dri3
    xcb-event
    xcb-ewmh
    xcb-glx
    xcb-icccm
    xcb-image
    xcb-keysyms
    xcb
    xcb-present
    xcb-proto
    xcb-randr
    xcb-record
    xcb-render
    xcb-renderutil
    xcb-res
    xcb-screensaver
    xcb-shape
    xcb-shm
    xcb-sync
    xcb-util
    xcb-xf86dri
    xcb-xfixes
    xcb-xinerama
    xcb-xinput
    xcb-xkb
    xcb-xtest
    xcb-xvmc
    xcb-xv
    IMPORTED_TARGET)
pkg_check_modules(X11_XCB x11-xcb IMPORTED_TARGET)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(XEMBED_SNI_PROXY_SOURCES
    application.c
    application.h
    xcb-utils.c
    xcb-utils.h
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    xtestsender.c
    xtestsender.h
    gwater-xcb.c
    gwater-xcb.h
 )

if(XCB_FOUND AND X11_XCB_FOUND)

    add_executable(xembedsniproxy ${XEMBED_SNI_PROXY_SOURCES})

    set(CMAKE_INCLUDE_CURRENT_DIR ON)

    target_link_libraries(xembedsniproxy
        PkgConfig::XCB
        PkgConfig::X11_XCB
        GLIB2::GOBJECT
        GLIB2::GIO_UNIX
        GTK3::GDK
        GTK3::GTK
    )

    add_executable(xcb-tray xcb-tray.c gwater-xcb.c gwater-xcb.h)


    target_link_libraries(xcb-tray
        PkgConfig::XCB
        GLIB2::GIO_UNIX
    )


    install(TARGETS xembedsniproxy RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/vala-panel )
    install(FILES xembedsniproxy.desktop DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()
