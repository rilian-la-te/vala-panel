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
 )

add_executable(xembedsniproxy ${XEMBED_SNI_PROXY_SOURCES})

set_package_properties(XCB PROPERTIES TYPE REQUIRED)

target_include_directories(xembedsniproxy PRIVATE
    ${XCB_INCLUDE_DIRS}
    ${X11_XCB_INCLUDE_DIRS}
    ${GTK3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(xembedsniproxy
    ${XCB_LIBRARIES}
    ${X11_XCB_LIBRARIES}
    ${GTK3_LIBRARIES}
)

add_executable(xcb-tray xcbtray.c)

target_include_directories(xcb-tray PRIVATE
    ${XCB_INCLUDE_DIRS}
    ${X11_XCB_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(xcb-tray
    ${XCB_LIBRARIES}
)


install(TARGETS xembedsniproxy RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXEC_DIR}/vala-panel )
install(FILES xembedsniproxy.desktop DESTINATION ${CMAKE_INSTALL_BINDIR})
