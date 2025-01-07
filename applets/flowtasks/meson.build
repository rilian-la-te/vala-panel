x11_sources = files(
    'flowtasks-backend-wnck.c',
    'flowtasks-backend-wnck.h',
    'libwnck-aux.c',
    'libwnck-aux.h'
)

headers = files(
    'task.h',
    'task-model.h',
    'matcher.h',
    'flowtasks-widget.h',
    'flowtasks-applet.h'
)

sources = files(
    'task.c',
    'task-model.c',
    'matcher.c',
    'flowtasks-widget.c',
    'flowtasks-applet.c',
  )
  
enum = 'flowtasks-enums'
flowtasks_enums_gen = gnome.mkenums_simple(
  enum,
  sources: headers,
  install_header: false,
)

flowtasks_res = gnome.compile_resources(
    'flowtasks_res', 'flowtasks.gresource.xml',
    source_dir: '.',
    c_name: 'flowtasks_res'
)

flowtasks = shared_module( 'flowtasks',
                        headers, sources, x11_sources, flowtasks_enums_gen, flowtasks_res,
                        dependencies: [libvalapanel, wnck],
                        install: true,
                        c_args: ['-DWNCK_I_KNOW_THIS_IS_UNSTABLE'],
                        install_dir: applets_libdir
                     )

i18n.merge_file(
  input: 'org.valapanel.flowtasks.desktop.plugin.in',
  output: 'org.valapanel.flowtasks.plugin',
  kwargs: plugin_conf_kwargs
)

install_data([
	'org.valapanel.flowtasks.gschema.xml'
], install_dir: schema_dir)
