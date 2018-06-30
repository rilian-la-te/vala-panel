prefix=@CMAKE_INSTALL_PREFIX@
pkglibdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@
pluginsdir=@CMAKE_INSTALL_FULL_LIBDIR@/vala-panel/plugins
datadir=@CMAKE_INSTALL_FULL_DATADIR@/vala-panel
datarootdir=@CMAKE_INSTALL_FULL_DATAROOTDIR@

Name: vala-panel
Description: A GTK3 desktop panel
Requires: glib-2.0 >= 2.56.0 gtk+-3.0 >= 3.22
Version: @VERSION@
Cflags: -I@DOLLAR@{includedir}/vala-panel
Libs: -L@DOLLAR@{pkglibdir} -lvalapanel
