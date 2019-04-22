wnck = dependency('libwnck-3.0',version: '>=3.4.8', required : get_option('wnck'))
gdk_x11 = dependency('gdk-x11-3.0', version: gtk_ver, required : get_option('wnck'))


subdir('core')
subdir('wnck', if_found: [wnck, gdk_x11])

