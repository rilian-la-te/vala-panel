sources = files('dirmenu.vala')
posix_dep = meson.get_compiler('vala').find_library('posix')
clock = shared_module( 'dirmenu',
                        sources,
                        dependencies: [libvalapanel, posix_dep],
                        install: true,
                        install_dir: applets_libdir
                     )
                     
i18n.merge_file(
  input: 'org.valapanel.dirmenu.desktop.plugin.in',
  output: 'org.valapanel.dirmenu.plugin',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(),'po'),
  install: true,
  install_dir: applets_datadir
)
