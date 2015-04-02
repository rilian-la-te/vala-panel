prefix=@CMAKE_INSTALL_PREFIX@
pkglibdir=@CMAKE_INSTALL_PREFIX@/lib/vala-panel
includedir=@CMAKE_INSTALL_PREFIX@/include/vala-panel
pluginsdir=@CMAKE_INSTALL_PREFIX@/lib/vala-panel/plugins
datadir=@CMAKE_INSTALL_PREFIX@/share/vala-panel
datarootdir=@CMAKE_INSTALL_PREFIX@/share/vala-panel

Name: vala-panel
Description: A GTK3 desktop panel
Requires: glib-2.0 >= 2.40.0 gtk+-3.0 >= 3.12 libpeas-1.0
Version: @VERSION@
Libs: -L${pkglibdir} -lvalapanel
