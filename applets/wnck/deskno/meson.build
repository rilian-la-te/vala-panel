sources = files('deskno.vala')
plugin_name = meson.current_source_dir().split('/').get(-1)
plugin = shared_module( plugin_name,
                        sources,
                        dependencies: [libvalapanel, wnck, gdk_x11],
                        install: true,
                        c_args: wnck_cflags,
                        install_dir: applets_libdir
                     )
