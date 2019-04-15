sources = files('buttons.vala')
buttons = shared_module( 'buttons',
                        sources, 
                        dependencies: [core_deps,vala_panel_dep, wnck, gdk_x11],
                        install: true,
                        c_args: [applet_cflags, wnck_cflags],
                        include_directories: applet_inc,
                        install_dir: applets_libdir
                     )
                     
i18n.merge_file(
  input: 'org.valapanel.buttons.desktop.plugin.in',
  output: 'org.valapanel.buttons.plugin',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(),'po'),
  install: true,
  install_dir: applets_datadir
)