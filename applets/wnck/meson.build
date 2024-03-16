wnck_cflags = ['-DWNCK_I_KNOW_THIS_IS_UNSTABLE']
plugin_list = [
    'pager',
    'deskno',
    'wincmd',
    'buttons',
]

wnck_kwargs = {
    'install': true,
    'c_args': wnck_cflags,
    'install_dir': applets_libdir
}

foreach plugin : plugin_list
    add_dep = dependency('', required : false)
    subdir(plugin)
    plugin_module = shared_module(plugin,
                    sources, config,
                    dependencies: [libvalapanel, wnck, gdk_x11, add_dep],
                    kwargs: wnck_kwargs,
                 )
    i18n.merge_file(
        input: '@0@/org.valapanel.@1@.desktop.in'.format(plugin, plugin),
        output: 'org.valapanel.@0@.plugin'.format(plugin),
        kwargs: plugin_conf_kwargs
    )
endforeach


subdir('tasklist')
