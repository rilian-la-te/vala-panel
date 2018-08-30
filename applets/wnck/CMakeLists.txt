include(CMakeDependentOption)
find_package(WNCK 3.4.8)
find_package(X11 COMPONENTS X11 Xcomposite Xdamage Xrender Xfixes)
cmake_dependent_option(ENABLE_WNCK "Enables wnck-related plugins, like taskbar and pager" ON
    "WNCK_FOUND;X11_FOUND" OFF)

if (ENABLE_WNCK)
    add_subdirectory(tasklist)
    add_subdirectory(pager)
    add_subdirectory(deskno)
    add_subdirectory(wincmd)
    add_subdirectory(buttons)
endif()


add_feature_info(WnckApplets ENABLE_WNCK "ValaPanel libwnck applets (tasklists,pager)")
