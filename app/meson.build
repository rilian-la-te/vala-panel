app_sources = files(
    'application.c',
    'application.h',
    'vala-panel-platform-standalone-x11.c',
    'vala-panel-platform-standalone-x11.h'
)

app_res = gnome.compile_resources(
    'app', 'vala-panel.gresource.xml',
    source_dir: 'app',
    c_name: 'app'
)

app = executable(
    meson.project_name(),
    app_sources, app_res,
    dependencies: libvalapanel,
    c_args: ['-DG_SETTINGS_ENABLE_BACKEND'],
    install : true
)
