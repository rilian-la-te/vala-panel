util_headers = files(
    'boxed-wrapper.h',
    'glistmodel-filter.h',
    'constants.h',
    'misc.h',
    'util.h'
)
util_sources = files(
    'boxed-wrapper.c',
    'glistmodel-filter.c',
    'misc.c',
)

util_inc = include_directories('.')

util_lib = static_library(
    'util', util_sources, util_headers,
    dependencies : [giounix,glib,gio],
    pic: true,
)

util = declare_dependency(
	link_whole: util_lib,
	dependencies : [giounix,glib,gio],
	include_directories: [core_inc, util_inc],
)
install_headers(util_headers, subdir : meson.project_name())

subdir('gtk')
