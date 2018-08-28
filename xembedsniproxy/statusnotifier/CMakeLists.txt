include(GLibProgramHandlers)
add_glib_marshal(MARSHALC MARSHALH "closures" g_cclosure_user_marshal)

file(STRINGS org.kde.StatusNotifierWatcher.xml XML_RAW)
string(CONCAT WATCHER_XML ${XML_RAW})
file(STRINGS org.kde.StatusNotifierItem.xml PRIVATE_RAW)
string(CONCAT ITEM_XML ${PRIVATE_RAW})
configure_file(interfaces.h.in interfaces.h ESCAPE_QUOTES)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ENUM_FILES
    sn.c
    sn.h
        )
add_glib_enumtypes(ENUMSC ENUMSH sni-enums ${ENUM_FILES})


add_library(sni STATIC
    sn.c
    sn.h
    ${MARSHALC}
    ${ENUMSC}
    ${ENUMSH})

target_link_libraries(sni GLIB2::GIO_UNIX GTK3::GDK)
