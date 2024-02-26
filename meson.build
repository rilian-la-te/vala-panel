project('vala-panel-sntray', 'c', 'vala',
        version : '0.4.13',
        meson_version: '>=0.61.0',
        license: ['LGPL-3.0'],
    default_options: [
        'c_std=gnu11',
        'buildtype=debugoptimized',
        'warning_level=1'
        ])

am_cflags = [
    '-fstack-protector',
    '-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
]
add_project_arguments(am_cflags, language: 'c')

##################
# Module imports #
##################

gnome = import('gnome')
i18n = import('i18n')

###############
# Directories #
###############
prefix = get_option('prefix')
datadir = get_option('datadir')
localedir = get_option('localedir')
schema_dir = join_paths(get_option('datadir'), 'glib-2.0', 'schemas')

################
# Dependencies #
################
glib_ver = '>=2.52.0'
giounix = dependency('gio-unix-2.0', version: glib_ver)

gtk_ver = '>=3.24.0'
gtk = dependency('gtk+-3.0', version: gtk_ver)

vp_ver = '>=0.4.89'
vp = dependency('vala-panel', version:  vp_ver, required: true)
vala_panel_found = vp.found()


#################
# Configuration #
#################
conf_data = configuration_data()
conf_data.set('GETTEXT_PACKAGE', meson.project_name())
conf_data.set('LOCALE_DIR', join_paths(prefix,localedir))
conf_data.set('CMAKE_INSTALL_PREFIX', prefix)

config = configure_file(input : join_paths('src','config.vala.in'),
               output : 'config.vala',
               configuration : conf_data)

desktop_kwargs = {
      'type': 'desktop',
      'args' : ['--keyword=Name','--keyword=Description','--keyword=Comment', '--keyword=Help'],
      'po_dir': join_paths(meson.current_source_dir(),'po'),
      'install': true,
}

subdir('po')
subdir('src')
subdir('data')

readmes = [
    'README.md',
    'LICENSE',
]
install_data(readmes, install_dir : join_paths(get_option('datadir'), meson.project_name(), 'doc'))
