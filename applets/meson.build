wnck = dependency('libwnck-3.0',version: '>=3.4.8', required : get_option('wnck'))
gdk_x11 = dependency('gdk-x11-3.0', version: gtk_ver, required : get_option('wnck'))
drawing = get_option('drawing')
applet_inc = [core_inc,util_inc,util_gtk_inc, ui_inc]
applet_cflags = ['-DGETTEXT_PACKAGE="'+meson.project_name()+'"']

subdir('core')
if(drawing)
    subdir('drawing')
endif
if(wnck.found() and gdk_x11.found())
    subdir('wnck')
endif