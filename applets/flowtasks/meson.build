x11_sources = files(
    'flowtasks-backend-wnck.c',
    'flowtasks-backend-wnck.h',
    'libwnck-aux.c',
    'libwnck-aux.h'
)

sources = files(
    'task.h',
    'task.c',
    'task-model.c',
    'task-model.h',
    'matcher.c',
    'matcher.h',
    'flowtasks-widget.c',
    'flowtasks-widget.h',
    'flowtasks-applet.c',
    'flowtasks-applet.h'
  )
  
enum = 'flowtasks-enums'
flowtasks_enums_gen = gnome.mkenums(
  enum,
  sources: sources,
  c_template: enum + '.c.template',
  h_template: enum + '.h.template',
  install_header: false,
)

flowtasks_res = gnome.compile_resources(
    'flowtasks_res', 'flowtasks.gresource.xml',
    source_dir: '.',
    c_name: 'flowtasks_res'
)

flowtasks = shared_module( 'flowtasks',
                        sources, x11_sources, flowtasks_enums_gen, flowtasks_res,
                        dependencies: [libvalapanel, wnck],
                        install: true,
                        c_args: [wnck_cflags],
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
