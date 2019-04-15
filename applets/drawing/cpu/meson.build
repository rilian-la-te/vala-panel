sources = files(
  'cpu.c',
  'cpu.h'
  )
cpu = shared_module( 'cpu',
                        sources, 
                        dependencies: [core_deps,vala_panel_dep, m],
                        install: true,
                        c_args: applet_cflags,
                        include_directories: applet_inc,
                        install_dir: applets_libdir
                     )
                     
i18n.merge_file(
  input: 'org.valapanel.cpu.desktop.plugin.in',
  output: 'org.valapanel.cpu.plugin',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(),'po'),
  install: true,
  install_dir: applets_datadir
)