runner_sources = files(
    'runner.c',
    'runner.h',
    'runner-app.c',
    'runner-app.h'
)

runner_res = gnome.compile_resources(
    'runner', 'runner.gresource.xml',
    source_dir: ['runner', 'data'],
    c_name: 'runner'
)

runner = executable(
    meson.project_name()+'-runner',
    runner_sources, runner_res,
    dependencies: util_gtk,
    include_directories: core_inc,
    install : true
)
