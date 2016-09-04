configure_file(${CMAKE_SOURCE_DIR}/data/sntray-mate.desktop.plugin.in ${CMAKE_SOURCE_DIR}/data/sntray-mate.desktop.in)
include(Translations)
add_translations_directory(${GETTEXT_PACKAGE})
add_translations_catalog(${GETTEXT_PACKAGE}
    ../src
    ../data
)
