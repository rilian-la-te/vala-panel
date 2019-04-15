sources = files(
  'tasklist.c',
  'tasklist.h',
  'tasklist-widget.c',
  'tasklist-widget.h',
  'xfce-arrow-button.c',
  'xfce-arrow-button.h',
  )
tasklist = shared_module( 'tasklist',
                        sources, 
                        dependencies: [core_deps,vala_panel_dep, wnck],
                        install: true,
                        c_args: [applet_cflags, wnck_cflags],
                        include_directories: applet_inc,
                        install_dir: applets_libdir
                     )
                     
i18n.merge_file(
  input: 'org.xfce.tasklist.desktop.plugin.in',
  output: 'org.xfce.tasklist.plugin',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(),'po'),
  install: true,
  install_dir: applets_datadir
)