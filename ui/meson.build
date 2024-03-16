ui_enum_headers = files(
    'panel-platform.h',
)
ui_headers = ui_enum_headers + files (
    'client.h',
    'server.h',
    'vala-panel.h',
    'definitions.h',
    'applet-widget-api.h',
    'applet-widget.h',
    'settings-manager.h',
    'toplevel.h',
    'applet-info.h',
    'panel-layout.h'
)
ui_sources = files (
    'toplevel-config.c',
    'toplevel-config.h',
    'settings-manager.c',
    'panel-platform.c',
    'applet-widget.c',
    'toplevel.c',
    'applet-info.c',
    'applet-manager.c',
    'applet-manager.h',
    'panel-layout.c'
)
enum = 'vala-panel-enums'
ui_enums_gen = gnome.mkenums(
  enum,
  sources: ui_enum_headers,
  c_template: enum + '.c.template',
  h_template: enum + '.h.template',
  install_header: true,
  install_dir: join_paths(includedir, meson.project_name()),
)
ui_res = gnome.compile_resources(
    'ui_res', 'libvalapanel.gresource.xml',
    source_dir: 'data',
    c_name: 'ui_res'
)
ui_inc = include_directories('.')

ui_lib = library(library_name,
             ui_headers, ui_sources, ui_res, ui_enums_gen, version,
             dependencies: util_gtk,
             include_directories: ui_inc,
             version: meson.project_version(),
             c_args: ['-DG_SETTINGS_ENABLE_BACKEND'],
             soversion: 0,
             install: true)

applets_vapi_dir = meson.current_source_dir()
vala_panel_dep = meson.get_compiler('vala').find_library('vala-panel', dirs: applets_vapi_dir)

libvalapanel = declare_dependency(
	link_with: ui_lib,
	dependencies: [util_gtk,vala_panel_dep],
	include_directories: [core_inc, ui_inc],
)

install_headers(ui_headers, subdir : meson.project_name())
vapi_dir = join_paths(prefix, datadir, 'vala', 'vapi')
install_data(
    meson.project_name()+'.deps',
    install_dir: vapi_dir,
)
install_data(
    meson.project_name()+'.vapi',
    install_dir: vapi_dir,
)

pkgconfig.generate(ui_lib,
             name: meson.project_name(),
             description: 'A GTK3 desktop panel',
             requires: [glib, gio, giounix, gmodule, gtk],
             variables: [
                 'datadir='+join_paths('${prefix}',datadir,meson.project_name()),
                 'datarootdir='+join_paths('${prefix}',datadir),
                 'applets_dir='+join_paths('${libdir}',meson.project_name(),'applets'),
                 'applets_data='+join_paths('${datadir}','applets'),
                ],
             extra_cflags: ['-I${includedir}/vala-panel']
            )

ui_gir = gnome.generate_gir(ui_lib,
                    sources: [ui_headers, util_headers, util_gtk_headers],
                    includes: ['GObject-2.0', 'Gio-2.0', 'Gtk-3.0', 'GModule-2.0'],
                    header: 'vala-panel.h',
                    namespace: 'ValaPanel',
                    identifier_prefix: 'ValaPanel',
                    symbol_prefix: 'vala_panel',
                    nsversion: meson.project_version(),
                    install: true
)

vala_panel_vapi = gnome.generate_vapi('vala-panel',
    sources: ui_gir[0],
    packages: ['gio-2.0', 'gtk+-3.0', 'gio-unix-2.0'],
    install: true,
)