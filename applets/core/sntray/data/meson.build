cdata = configuration_data()
cdata.set('CMAKE_INSTALL_FULL_LIBDIR', join_paths(prefix,get_option('libdir')))
cdata.set('CMAKE_INSTALL_FULL_DATAROOTDIR', join_paths(prefix,datadir))

i18n.merge_file(
  input: 'org.valapanel.sntray.desktop.in',
  output: 'org.valapanel.sntray.plugin',
  kwargs: plugin_conf_kwargs,
)

install_data([
  'org.valapanel.toplevel.sntray.gschema.xml'
], install_dir: schema_dir)
