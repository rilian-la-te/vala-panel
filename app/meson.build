app_sources = files(
    'application.c',
    'application.h'
)

platform_x11_sources = files(
    'vala-panel-platform-standalone-x11.c',
    'vala-panel-platform-standalone-x11.h'
)

platform_gtk_layer_sources = files(
    'vala-panel-platform-standalone-wayland.c',
    'vala-panel-platform-standalone-wayland.h'
)

app_res = gnome.compile_resources(
    'app', 'vala-panel.gresource.xml',
    source_dir: 'app',
    c_name: 'app'
)

app_all_sources = app_sources
app_dependencies = [libvalapanel]
if(platforms['x11'])
    app_all_sources += platform_x11_sources
endif

if(platforms['wayland'])
    app_all_sources += platform_gtk_layer_sources
    app_dependencies += gtk_layer_shell
endif

app = executable(
    meson.project_name(),
    app_all_sources, app_res, version,
    dependencies: app_dependencies,
    c_args: ['-DG_SETTINGS_ENABLE_BACKEND'],
    install : true
)
