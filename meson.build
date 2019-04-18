project('vala-panel', 'vala', 'c',
	version : '0.5.0',
	meson_version: '>=0.47.0',
	license: ['LGPL-3.0'],
    default_options: [
        'c_std=gnu11',
        'buildtype=debugoptimized',
        'warning_level=1'
        ])

library_name = 'valapanel'

##################
# Module imports #
##################

gnome = import('gnome')
i18n = import('i18n')
pkgconfig = import('pkgconfig')

###############
# Directories #
###############

prefix = get_option('prefix')

bindir = get_option('bindir')
datadir = get_option('datadir')
desktopdir = join_paths(datadir, 'applications')
includedir = get_option('includedir')
libdir = get_option('libdir')
localedir = get_option('localedir')
project_datadir = join_paths(prefix,datadir, meson.project_name())
applets_datadir = join_paths(prefix,datadir, meson.project_name(),'applets')
applets_libdir = join_paths(prefix,libdir, meson.project_name(),'applets')
profiles_dir = join_paths(prefix,get_option('sysconfdir'),'xdg',meson.project_name())


#am_cflags = [
#    '-Wno-unused',
#    '-Wno-unused-parameter',
#    '-Wno-discarded-qualifiers',
#    '-Wno-incompatible-pointer-types',
#    '-Wno-missing-field-initializers',
#    '-fstack-protector',
#    '-Wstrict-prototypes',
#    '-Wundef',
#    '-Werror-implicit-function-declaration',
#    '-Wformat',
#    '-Wformat-security',
#    '-Werror=format-security',
#    '-Wconversion',
#    '-Wunreachable-code',
#    '-Wextra',
#    '-Wall',
#    '-W',
#]
am_cflags = [
    '-fstack-protector',
    '-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
]
add_project_arguments(am_cflags, language: 'c')

################
# Dependencies #
################

glib_ver = '>=2.52'
glib = dependency('glib-2.0', version: glib_ver)
gio = dependency('gio-2.0', version: glib_ver)
giounix = dependency('gio-unix-2.0', version: glib_ver)
gmodule = dependency('gmodule-2.0', version: glib_ver)
gtk_ver = '>=3.22'
gtk = dependency('gtk+-3.0', version: gtk_ver)
cc = meson.get_compiler('c')
m = cc.find_library('m', required : false)

core_deps = [glib, gio, giounix, gmodule, gtk, m]
core_inc = include_directories('.')

#################
# Configuration #
#################
conf_data = configuration_data()
conf_data.set('GETTEXT_PACKAGE', meson.project_name())
conf_data.set('LOCALE_DIR', join_paths(prefix,localedir))
conf_data.set('PLUGINS_DIRECTORY', applets_libdir)
conf_data.set('PLUGINS_DATA', applets_datadir)
conf_data.set('DATADIR', project_datadir)
conf_data.set('PROFILES_DIR', profiles_dir)

config = configure_file(input : join_paths('util','config.h.in'),
               output : 'config.h',
			   configuration : conf_data)

version = vcs_tag(
	command : ['git','describe','--tags'],
	input: 'util/version.h.in',
	output: 'version.h',
	replace_string: '@VERSION@',
)

readmes = [
    'README.md',
    'LICENSE'
]
install_data(readmes, install_dir : join_paths(datadir, meson.project_name(), 'doc'))

subdir('po')
subdir('data')
subdir('util')
subdir('runner')
subdir('ui')
subdir('app')
subdir('applets')
