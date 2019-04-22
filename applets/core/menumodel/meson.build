sources = files(
  'menu-maker.c',
  'menu.c',
  'menu.h',
  )

plugin_name = meson.current_source_dir().split('/').get(-1)
res = gnome.compile_resources(
    plugin_name+'res', plugin_name+'.gresource.xml',
    source_dir: meson.current_source_dir(),
    c_name: 'res'
)

plugin = shared_module(  plugin_name,
                        sources, res, config,
                        dependencies: libvalapanel,
                        install: true,
                        install_dir: applets_libdir
                     )
