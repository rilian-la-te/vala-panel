INCLUDE(GResource)
set(VALA_FILES  application.vala runner.vala)
vala_precompile(VALA_C ${APPNAME}
    ${VALA_FILES}
PACKAGES
    ${CORE_PACKAGES}
    vala-panel
    posix
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/lib
    --target-glib=2.44
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/vala-panel.gresource.xml
    --thread
    -D GLIB242
)

add_definitions(-DG_SETTINGS_ENABLE_BACKEND)
glib_compile_resources(GLIB_RESOURCES_APP
    source
        vala-panel.gresource.xml
)

#####
# Executable
#####
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_PROJECT_NAME}/")
add_custom_target(vala-ide-app SOURCES ${VALA_FILES})
add_executable (${APPNAME} ${VALA_C} ${GLIB_RESOURCES_APP})
add_executable(${APPNAME}-new  runner-new.c runner-new.h application-new.c application-new.h)
target_link_libraries (${APPNAME} ${LIBNAME} ${CORE_LIBRARIES})
target_link_libraries (${APPNAME}-new ${CORE_LIBRARIES} cvalapanel applet-api-new)
target_include_directories(${APPNAME} PRIVATE ${CORE_INCLUDE_DIRS})
target_include_directories(${APPNAME}-new PRIVATE ${CORE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/lib/c-lib ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/lib)

install (TARGETS ${APPNAME} ${APPNAME}-new RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin)
