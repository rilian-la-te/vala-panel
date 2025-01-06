wnck = dependency('libwnck-3.0',version: '>=3.4.8', required : platforms['x11'])

wind = dependency('libxfce4windowing-0',version: '>=4.20.0', required : true)

if(wind.found())
    gir_location = join_paths(girepository.get_variable('girdir'), 'Libxfce4windowing-0.0.gir')
    wind_vapi = gnome.generate_vapi('libxfce4windowing-0', sources: gir_location, packages: ['glib-2.0', 'gtk+-3.0'])
    xfcewind_dep = declare_dependency(dependencies: [wind_vapi, wind])
endif

subdir('core')
subdir('wnck', if_found: [wind, wnck, gdk_x11])
subdir('flowtasks', if_found: [wind, wnck, gdk_x11])
