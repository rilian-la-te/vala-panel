subdir('dbusmenu')

backend_sources = files(
    'icon-pixmap.c',
    'icon-pixmap.h',
    'rtparser.c',
    'rtparser.h',
    'snproxy.c',
    'snproxy.h',
    )
backend_sources_vala = files(
    'snconfig.vala',
    'snchild.vala',
    'snhost.vala',
    'snitembox.vala',
    'snwatcher.vala',
    'sntray-backend.vapi'
)
backend_headers = files('../include/sn-common.h')
enum = 'sni-enums'
backend_enums_gen = gnome.mkenums(
  enum,
  sources: backend_headers,
  c_template: enum + '.c.template',
  h_template: enum + '.h.template',
)

res = gnome.compile_resources(
    'sntrayres', 'sntray.gresource.xml',
    source_dir: '.',
    c_name: 'sntrayres'
)

backend_inc = include_directories('../include')
this_inc = include_directories('.')
backend_lib = static_library('sn-backend', backend_sources, backend_enums_gen, backend_sources_vala, res,
    include_directories : backend_inc,
    dependencies: [vala_dbusmenu],
    pic : true
)
backend_dep = declare_dependency(
        dependencies : [gtk, giounix],
        include_directories: backend_inc,
        link_whole: backend_lib
)
backend_vapi = declare_dependency(
    include_directories: [backend_inc, this_inc],
    dependencies: [backend_dep, meson.get_compiler('vala').find_library('sntray-backend', dirs: meson.current_source_dir())]
)

vp_sources = files('vala-panel-sntray-applet.vala')
vpp = shared_module('sntray', vp_sources,
                dependencies: [backend_vapi, libvalapanel],
                install: true,
                install_dir: applets_libdir
                )