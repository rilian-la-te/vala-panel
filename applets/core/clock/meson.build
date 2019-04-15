sources = files('clock.vala')
clock = shared_module( 'clock',
                        sources, 
                        dependencies: [core_deps,vala_panel_dep],
                        install: true,
                        c_args: applet_cflags,
                        include_directories: applet_inc,
                        install_dir: applets_libdir
                     )
                     
i18n.merge_file(
  input: 'org.valapanel.clock.desktop.plugin.in',
  output: 'org.valapanel.clock.plugin',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(),'po'),
  install: true,
  install_dir: applets_datadir
)