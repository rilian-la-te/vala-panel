sources = files('kbled.vala')
res = gnome.compile_resources(
    'kbledres', 'kbled.gresource.xml',
    source_dir: meson.current_source_dir(),
    c_name: 'res'
)
clock = shared_module( 'kbled',
                        sources, res, config,
                        dependencies: [core_deps,vala_panel_dep],
                        install: true,
                        c_args: applet_cflags,
                        include_directories: applet_inc,
                        install_dir: applets_libdir
                     )
                     
i18n.merge_file(
  input: 'org.valapanel.kbled.desktop.plugin.in',
  output: 'org.valapanel.kbled.plugin',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(),'po'),
  install: true,
  install_dir: applets_datadir
)