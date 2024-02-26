mans = files(
    join_paths('man','vala-panel.1'),
    join_paths('man','vala-panel-runner.1')
)
install_man(mans)

default_profile = configure_file(input : join_paths('profiles','default.in'),
               output : 'default',
			   configuration : conf_data)

additional_profiles = files(
    join_paths('profiles','appmenu'),
    join_paths('profiles','two_panels')
)
install_data(
    default_profile,
    install_dir: profiles_dir,
)
install_data(
    additional_profiles,
    install_dir: profiles_dir,
)

install_data(
    join_paths('images','panel.svg'),
    rename: 'vala-panel.svg',
    install_dir: join_paths(datadir, 'icons','hicolor','scalable','apps')
)

install_data(
    join_paths('images','panel.png'),
    rename: 'vala-panel.png',
    install_dir: join_paths(datadir, 'icons','hicolor','96x96','apps')
)

install_data(
    join_paths('images','background.png'),
    install_dir: join_paths(project_datadir, 'images')
)

schema_dir = join_paths(datadir, 'glib-2.0', 'schemas')
install_data([
    join_paths('gschemas','org.valapanel.builtin.gschema.xml'),
    join_paths('gschemas','org.valapanel.gschema.xml'),
    join_paths('gschemas','org.valapanel.plugins.gschema.xml'),
    join_paths('gschemas','org.valapanel.toplevel.gschema.xml'),
    join_paths('gschemas','org.valapanel.X.gschema.xml'),
], install_dir: schema_dir)

i18n.merge_file(
  input: join_paths('desktop','org.valapanel.application.desktop.in'),
  output: 'org.valapanel.application.desktop',
  type: 'desktop',
  po_dir: join_paths(meson.project_source_root(),'po'),
  install: true,
  install_dir: join_paths(datadir, 'applications')
)

i18n.merge_file(
  input: join_paths('desktop','org.valapanel.application.appdata.xml.in'),
  output: 'org.valapanel.application.appdata.xml',
  type: 'xml',
  po_dir: join_paths(meson.project_source_root(),'po'),
  install: true,
  install_dir: join_paths(datadir, 'appdata')
)
