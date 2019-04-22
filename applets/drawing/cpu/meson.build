sources = files(
  'cpu.c',
  'cpu.h'
  )
plugin_name = meson.current_source_dir().split('/').get(-1)
plugin = shared_module(  plugin_name,
                        sources,
                        dependencies: libvalapanel,
                        install: true,
                        install_dir: applets_libdir
                     )
