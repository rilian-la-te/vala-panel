sources = files('dirmenu.vala')
posix_dep = meson.get_compiler('vala').find_library('posix')
clock = shared_module( 'dirmenu',
                        sources,
                        dependencies: [core_deps,vala_panel_dep, posix_dep],
                        install: true,
                        c_args: applet_cflags,
                        include_directories: applet_inc,
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