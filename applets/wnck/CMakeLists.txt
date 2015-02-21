option(ENABLE_WNCK "Enables wnck-related plugins, like taaskbar and pager" ON)

if (ENABLE_WNCK)
	pkg_check_modules(WNCK libwnck-3.0)
	if (NOT(WNCK_FOUND))
		set(ENABLE_WNCK OFF)
	endif()
endif()

if (ENABLE_WNCK)
	add_subdirectory(tasklist)
endif()
