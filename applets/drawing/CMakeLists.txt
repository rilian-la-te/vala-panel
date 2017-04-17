option(ENABLE_DRAWING "Enables drawing plugins, like CPU Graph plot" ON)

if (ENABLE_DRAWING)
	add_subdirectory(cpu)
    add_subdirectory(monitors)
endif()
