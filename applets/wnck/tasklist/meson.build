sources = files(
  'tasklist.c',
  'tasklist.h',
  'tasklist-widget.c',
  'tasklist-widget.h',
  'xfce-arrow-button.c',
  'xfce-arrow-button.h',
  )
tasklist = shared_module( 'tasklist-xfce',
                        sources,
                        dependencies: [libvalapanel, wnck, gdk_x11],
                        kwargs: wnck_kwargs
                     )

i18n.merge_file(
  input: 'org.xfce.tasklist.desktop.in',
  output: 'org.xfce.tasklist.plugin',
  kwargs: plugin_conf_kwargs
)
