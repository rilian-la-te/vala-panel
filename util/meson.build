util_headers = files(
    'boxed-wrapper.h',
    'glistmodel-filter.h',
    'constants.h',
    'misc.h',
    'info-data.h',
    'launcher.h',
    'util.h'
)
util_sources = files(
    'boxed-wrapper.c',
    'glistmodel-filter.c',
    'misc.c',
    'info-data.c',
    'launcher.c',
)

util_inc = include_directories('.')

util_dep = static_library(
							'util', util_sources, util_headers,
							dependencies : [giounix,glib,gio],
						 )
install_headers(util_headers, subdir : meson.project_name())

subdir('gtk')
