vp_applets_libdir = vp.get_variable(pkgconfig: 'applets_dir')
vp_sources = files('vala-panel-sntray-applet.vala')
vpp = shared_module('sntray', vp_sources,
                dependencies: [backend_vapi, vp],
                install: true,
                install_dir: vp_applets_libdir
                )
