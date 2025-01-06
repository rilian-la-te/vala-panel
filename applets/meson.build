wnck = dependency('libwnck-3.0',version: '>=3.4.8', required : platforms['x11'])

subdir('core')
subdir('wnck', if_found: [wnck, gdk_x11])
subdir('flowtasks', if_found: [wnck, gdk_x11])
