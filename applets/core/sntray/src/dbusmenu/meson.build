vala_dbusmenu_sources = files(
    'proxy.vala',
    'item.vala',
    'property-store.vala',
    'client.vala',
    'gtk/client.vala',
    'gtk/item-iface.vala',
    'gtk/main-item.vala',
    'gtk/scale-item.vala',
    'gtk/separator-item.vala'
)

vala_dbusmenu_deps = [giounix, gtk]
vala_dbusmenu_inc = include_directories('.')

vala_dbusmenu_lib = static_library('libvaladbusmenu',
    vala_dbusmenu_sources, config,
    dependencies: vala_dbusmenu_deps,
    pic: true
)

vala_dbusmenu = declare_dependency(
    include_directories: vala_dbusmenu_inc,
    dependencies: vala_dbusmenu_deps,
    link_with: vala_dbusmenu_lib
)