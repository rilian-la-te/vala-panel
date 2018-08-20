include(CMakeDependentOption)
find_package(WNCK 3.4.8)
find_package(X11 COMPONENTS X11 Xcomposite Xdamage Xrender Xfixes)
cmake_dependent_option(ENABLE_X11 "Enables X11 system tray" ON
    "X11_FOUND" OFF)
cmake_dependent_option(ENABLE_WNCK "Enables wnck-related plugins, like taskbar and pager" ON
    "WNCK_FOUND;ENABLE_X11" OFF)

if (ENABLE_WNCK)
    add_subdirectory(tasklist)
    add_subdirectory(tasklist-xfce)
    add_subdirectory(pager)
    add_subdirectory(icontasks)
    add_subdirectory(deskno)
    add_subdirectory(wincmd)
    add_subdirectory(buttons)
endif()

if (ENABLE_X11)
    add_subdirectory(xembed)
endif()

add_feature_info(WnckApplets ENABLE_WNCK "ValaPanel libwnck applets (tasklists,pager)")
add_feature_info(Xembed ENABLE_X11 "XEmbed-based system tray (deprecated)")
