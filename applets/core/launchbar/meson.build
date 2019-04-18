sources = files(
  'launchbar.vala',
  'launchbar-button.vala',
  'launchbar-config.vala',
  )

res = gnome.compile_resources(
    'barres', 'launchbar.gresource.xml',
    source_dir: meson.current_source_dir(),
    c_name: 'launchbar'
)
clock = shared_module( 'launchbar',
                        sources, res, config,
                        dependencies: libvalapanel,
                        install: true,
                        install_dir: applets_libdir
                     )
                     
i18n.merge_file(
  input: 'org.valapanel.launchbar.desktop.plugin.in',
  output: 'org.valapanel.launchbar.plugin',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(),'po'),
  install: true,
  install_dir: applets_datadir
)
