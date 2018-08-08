find_package(XCB
    COMPONENTS
        AUX
        XCB
        XFIXES
        DAMAGE
        COMPOSITE
        RANDR
        SHM
        UTIL
        IMAGE
        XTEST
        ICCCM
)

set(XCB_LIBS
    XCB::AUX
    XCB::XCB
    XCB::XFIXES
    XCB::DAMAGE
    XCB::COMPOSITE
    XCB::RANDR
    XCB::SHM
    XCB::UTIL
    XCB::IMAGE
    XCB::XTEST
    XCB::ICCCM
)

find_package(X11_XCB)
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

add_executable(xembedsniproxy ${XEMBED_SNI_PROXY_SOURCES})

set_package_properties(XCB PROPERTIES TYPE REQUIRED)

target_include_directories(xembedsniproxy PRIVATE
    ${XCB_INCLUDE_DIRS}
    ${X11_XCB_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(xembedsniproxy
    ${XCB_LIBRARIES}
    ${X11_XCB_LIBRARIES}
    GLIB2::GOBJECT
    GLIB2::GIO_UNIX
    GTK3::GDK
    GTK3::GTK
)

add_executable(xcb-tray xcb-tray.c gwater-xcb.c gwater-xcb.h)

target_include_directories(xcb-tray PRIVATE
    ${XCB_INCLUDE_DIRS}
    ${X11_XCB_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(xcb-tray
    ${XCB_LIBRARIES}
    GLIB2::GIO_UNIX
)


install(TARGETS xembedsniproxy RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXEC_DIR}/vala-panel )
install(FILES xembedsniproxy.desktop DESTINATION ${CMAKE_INSTALL_BINDIR})
