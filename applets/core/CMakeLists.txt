add_subdirectory(kbled)
add_subdirectory(separator)
add_subdirectory(menumodel)
add_subdirectory(clock)
add_subdirectory(dirmenu)
add_subdirectory(launchbar)

option(ENABLE_DRAWING "Enables drawing plugins, like CPU Graph plot" ON)

if (ENABLE_DRAWING)
    add_subdirectory(cpu)
    add_subdirectory(monitors)
    add_subdirectory(netmon)
endif()
