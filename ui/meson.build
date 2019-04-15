ui_enum_headers = files(
    'panel-platform.h',
)
ui_headers = ui_enum_headers + files (
    'client.h',
    'server.h',
    'definitions.h',
    'applet-widget-api.h',
    'applet-widget.h',
    'settings-manager.h',
    'panel-platform.h',
    'toplevel.h',
    'toplevel-config.h',
    'applet-plugin.h',
    'applet-info.h',
    'panel-layout.h'
)
ui_sources = files (
    'toplevel-config.c',
    'settings-manager.c',
    'panel-platform.c',
    'applet-widget.c',
    'toplevel.c',
    'applet-plugin.c',
    'applet-info.c',
    'applet-manager.c',
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
    'ui', 'libvalapanel.gresource.xml',
    source_dir: 'data',
    c_name: 'ui'
)
ui_inc = include_directories('.')

ui = library(library_name,
             ui_headers, ui_sources, ui_res, ui_enums_gen,
             dependencies: core_deps,
             link_with: [util, util_gtk],
             include_directories: [core_inc,util_inc, util_gtk_inc, ui_inc],
             version: meson.project_version(),
             c_args: ['-DG_SETTINGS_ENABLE_BACKEND'],
             soversion: 0,
             install: true)

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

pkg = import('pkgconfig')
pkg.generate(ui,
             name: meson.project_name(),
             description: 'A GTK3 desktop panel',
             requires: [glib, gio, giounix, gmodule, gtk],
             variables: [
                 'datadir='+join_paths('${prefix}',datadir,meson.project_name()),
                 'datarootdir='+join_paths('${prefix}',datadir),
                 'pluginsdir='+join_paths('${libdir}',meson.project_name(),'applets'),
                ],
             extra_cflags: ['-I${includedir}/vala-panel']
            )