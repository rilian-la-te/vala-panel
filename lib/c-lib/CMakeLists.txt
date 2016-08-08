include(GResource)
add_library(cvalapanel SHARED
	misc.c
	misc.h
	css.c
	css.h
	)
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})
target_link_libraries (cvalapanel ${LIBS})
target_include_directories(cvalapanel PRIVATE ${CORE_INCLUDE_DIRS})
set_target_properties (cvalapanel PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${SOVERSION})
install (TARGETS cvalapanel DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME} COMPONENT bin)

