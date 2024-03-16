plugin_list = [
    'kbled',
    'separator',
    'menumodel',
    'clock',
    'launchbar',
    'dirmenu'
]
drawing_list = [
    'cpu',
    'monitors',
    'netmon'
]

if get_option('drawing')
    plugin_list += drawing_list
endif

posix_dep = meson.get_compiler('vala').find_library('posix')
foreach plugin : plugin_list
    subdir(plugin)
    if res_exists
        res = gnome.compile_resources(
            plugin+'res', join_paths(plugin,'@0@.gresource.xml'.format(plugin)),
            source_dir: join_paths(meson.current_source_dir(),plugin),
            c_name: 'res'
        )
    endif
    plugin_module = shared_module(plugin,
                    sources, res, config, version,
                    dependencies: [libvalapanel, posix_dep],
                    install: true,
                    install_dir: applets_libdir
                 )
    i18n.merge_file(
        input: '@0@/org.valapanel.@1@.desktop.in'.format(plugin, plugin),
        output: 'org.valapanel.@0@.plugin'.format(plugin),
        kwargs: plugin_conf_kwargs
    )
endforeach

importer = dependency('appmenu-glib-translator', required: false)
subdir('sntray', if_found : importer)