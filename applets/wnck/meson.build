wnck_cflags = ['-DWNCK_I_KNOW_THIS_IS_UNSTABLE']
plugin_list = [
    'pager',
    'deskno',
    'wincmd',
    'buttons',
]

foreach plugin : plugin_list
    subdir(plugin)
    plugin_module = shared_module(plugin,
                    sources, config,
                    dependencies: [libvalapanel, wnck, gdk_x11],
                    install: true,
                    c_args: wnck_cflags,
                    install_dir: applets_libdir
                 )
    i18n.merge_file(
        input: '@0@/org.valapanel.@1@.desktop.plugin.in'.format(plugin, plugin),
        output: 'org.valapanel.@0@.plugin'.format(plugin),
        type: 'desktop',
        po_dir: join_paths(meson.source_root(),'po'),
        install: true,
        install_dir: applets_datadir
    )
endforeach


subdir('tasklist')
