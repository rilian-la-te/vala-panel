sources = files('separator.vala')
clock = shared_module( 'separator',
                        sources,
                        dependencies: libvalapanel,
                        install: true,
                        install_dir: applets_libdir
                     )
                     
i18n.merge_file(
  input: 'org.valapanel.separator.desktop.plugin.in',
  output: 'org.valapanel.separator.plugin',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(),'po'),
  install: true,
  install_dir: applets_datadir
)
