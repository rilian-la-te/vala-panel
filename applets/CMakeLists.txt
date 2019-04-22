INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/util ${CMAKE_SOURCE_DIR}/ui ${CMAKE_BINARY_DIR}/ui ${CMAKE_SOURCE_DIR}/util/gtk ${CMAKE_BINARY_DIR})
add_subdirectory(core)
add_feature_info(CoreApplets ON "ValaPanel core applets (clock,menumodel,launchbar,separator)")
add_subdirectory(wnck)


