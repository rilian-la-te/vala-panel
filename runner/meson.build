runner_sources = files(
    'runner.c',
    'runner.h',
    'runner-app.c',
    'runner-app.h'
)

runner_res = gnome.compile_resources(
    'runner', 'runner.gresource.xml',
    source_dir: 'runner',
    c_name: 'runner'
)

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

runner = executable(
    meson.project_name()+'-runner',
    runner_sources, runner_res,
    dependencies: [core_deps, m_dep],
    include_directories: [core_inc, util_inc, util_gtk_inc],
    link_with: [util, util_gtk],
    install : true
)