sources = files('dirmenu.vala')
posix_dep = meson.get_compiler('vala').find_library('posix')
plugin_name = meson.current_source_dir().split('/').get(-1)
plugin = shared_module(  plugin_name,
                        sources,
                        dependencies: [libvalapanel, posix_dep],
                        install: true,
                        install_dir: applets_libdir
                     )
