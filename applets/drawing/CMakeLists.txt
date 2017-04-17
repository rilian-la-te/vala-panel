option(ENABLE_DRAWING "Enables drawing plugins, like CPU Graph plot" ON)

if (ENABLE_DRAWING)
    set(VALA_FILES
        monitors.vala
        cpu.vala
        engine-drawing.vala)
    add_custom_target(vala-ide-plugins-drawing SOURCES ${VALA_FILES})
    vala_precompile(VALA_C drawing
        ${VALA_FILES}
    PACKAGES
        ${CORE_PACKAGES}
        vala-panel
        cvalapanel
    OPTIONS
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_BINARY_DIR}/lib
        --vapidir=${CMAKE_BINARY_DIR}/vapi
        --target-glib=2.40
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/drawing.gresource.xml
        --thread
    )
    set (LIBS ${CORE_LIBRARIES} -lm)
    set (LIB_PATHS ${CORE_LIBRARY_DIRS})
    configure_file(data/plugins/drawing-plugins.gresource.xml.in ${CMAKE_CURRENT_BINARY_DIR}/data/plugins/drawing-plugins.gresource.xml)
    link_directories (${LIB_PATHS})
    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
    glib_compile_resources_fullpath(APPLETS
        source
            ${CMAKE_CURRENT_BINARY_DIR}/data/plugins/drawing-plugins.gresource.xml
    )
    add_library (drawing MODULE
        ${VALA_C}
        ${APPLETS}
    )
    target_link_libraries (drawing ${LIBS} valapanel)
    install(TARGETS drawing DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-base)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/data/plugins/drawing.plugin DESTINATION ${PLUGINS_DIRECTORY} COMPONENT plugins-base)
endif()
