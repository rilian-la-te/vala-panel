wnck_cflags = ['-DWNCK_I_KNOW_THIS_IS_UNSTABLE']
plugin_list = [
    'pager',
    'deskno',
    'wincmd',
    'buttons',
]

wnck_kwargs = {
    'dependencies': [libvalapanel, wnck, gdk_x11],
    'install': true,
    'c_args': wnck_cflags,
    'install_dir': applets_libdir
}

foreach plugin : plugin_list
    subdir(plugin)
    plugin_module = shared_module(plugin,
                    sources, config,
                    kwargs: wnck_kwargs,
                 )
    i18n.merge_file(
        input: '@0@/org.valapanel.@1@.desktop.in'.format(plugin, plugin),
        output: 'org.valapanel.@0@.plugin'.format(plugin),
        kwargs: plugin_conf_kwargs
    )
endforeach


subdir('tasklist')
