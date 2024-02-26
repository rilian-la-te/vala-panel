cdata = configuration_data()
cdata.set('CMAKE_INSTALL_FULL_LIBDIR', join_paths(prefix,get_option('libdir')))
cdata.set('CMAKE_INSTALL_FULL_DATAROOTDIR', join_paths(prefix,datadir))

vp_applets_datadir = vp.get_variable(pkgconfig: 'applets_data')
desktop = vcs_tag(
  command : ['git','describe','--tags'],
  input: 'sntray.desktop.plugin.in.cmake',
  output: 'sntray.desktop.plugin.in',
  replace_string: '@VERSION@',
)
i18n.merge_file(
  input: desktop,
  output: 'org.valapanel.sntray.plugin',
  kwargs: desktop_kwargs,
  install_dir: vp_applets_datadir
)

install_data([
  'org.valapanel.toplevel.sntray.gschema.xml'
], install_dir: schema_dir)
