wnck = dependency('libwnck-3.0',version: '>=3.4.8', required : get_option('wnck'))

subdir('core')
subdir('wnck', if_found: [wnck, gdk_x11])

