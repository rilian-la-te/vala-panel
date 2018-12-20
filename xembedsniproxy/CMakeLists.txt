pkg_check_modules(XCB
    xcb-atom
    xcb-damage
    xcb-icccm
    xcb-ewmh
    xcb
    xcb-proto
    xcb-randr
    xcb-util
    xcb-xtest
    IMPORTED_TARGET)
pkg_check_modules(X11_XCB x11-xcb IMPORTED_TARGET)

pkg_check_modules(GDK_PIXBUF gdk-pixbuf-2.0 IMPORTED_TARGET)
pkg_check_modules(CAIRO cairo IMPORTED_TARGET)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(ENUM_FILES
    sn.c
    sn.h
        )
add_glib_enumtypes(ENUMSC ENUMSH sni-enums ${ENUM_FILES})

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
    sn.c
    sn.h
    ${MARSHALC}
    ${ENUMSC}
    ${ENUMSH}
)

if(XCB_FOUND AND X11_XCB_FOUND)
    file(STRINGS org.kde.StatusNotifierWatcher.xml XML_RAW)
    string(CONCAT WATCHER_XML ${XML_RAW})
    file(STRINGS org.kde.StatusNotifierItem.xml PRIVATE_RAW)
    string(CONCAT ITEM_XML ${PRIVATE_RAW})
    configure_file(interfaces.h.in interfaces.h ESCAPE_QUOTES)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)

    add_executable(xembedsniproxy ${XEMBED_SNI_PROXY_SOURCES})

    set(CMAKE_INCLUDE_CURRENT_DIR ON)

    target_link_libraries(xembedsniproxy
        PkgConfig::XCB
        PkgConfig::X11_XCB
        PkgConfig::GDK_PIXBUF
        PkgConfig::CAIRO
        GLIB2::GOBJECT
        GLIB2::GIO_UNIX
    )

   # add_executable(xcb-tray xcb-tray.c gwater-xcb.c gwater-xcb.h)


#    target_link_libraries(xcb-tray
#        PkgConfig::XCB
#        GLIB2::GIO_UNIX
#    )


    install(TARGETS xembedsniproxy RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/vala-panel )
    #install(FILES xembedsniproxy.desktop DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()
