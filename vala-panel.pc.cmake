prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

datadir=@CMAKE_INSTALL_FULL_DATADIR@/@CMAKE_PROJECT_NAME@
datarootdir=@CMAKE_INSTALL_FULL_DATAROOTDIR@
applets_dir=@PLUGINS_DIRECTORY@
applets_data=@PLUGINS_DATA@

Name: vala-panel
Description: A GTK3 desktop panel
Requires: glib-2.0 >= 2.56.0 gtk+-3.0 >= 3.22
Version: @VERSION@
Cflags: -I${includedir}/vala-panel
Libs: -L${libdir} -lvalapanel
