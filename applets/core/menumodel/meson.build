sources = files(
  'menu-maker.c',
  'menu.c',
  'menu.h',
  )

res = gnome.compile_resources(
    'menumodel', 'menumodel.gresource.xml',
    source_dir: meson.current_source_dir(),
    c_name: 'menumodel'
)
clock = shared_module( 'menumodel',
                        sources, res, config,
                        dependencies: libvalapanel,
                        install: true,
                        install_dir: applets_libdir
                     )
                     
i18n.merge_file(
  input: 'org.valapanel.menumodel.desktop.plugin.in',
  output: 'org.valapanel.menumodel.plugin',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(),'po'),
  install: true,
  install_dir: applets_datadir
)
