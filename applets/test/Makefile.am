-include $(top_srcdir)/common.mk

EXTRA_DIST =
pkglib_LTLIBRARIES =
pkglibdir = $(libdir)/vala-panel/plugins
plugindir = $(datadir)/vala-panel/plugins
plugin_DATA =



plugin_DATA += \
	TestApplet.plugin

EXTRA_DIST += \
	$(plugin_DATA)

pkglib_LTLIBRARIES += libtestapplet.la

libtestapplet_la_SOURCES = \
	test.vala

libtestapplet_la_CFLAGS = \
	$(PACKAGE_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(GEE_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(LIBPEAS_CFLAGS) \
	-DMODULE_DIR=\"$(pkglibdir)/plugins\" \
	-DMODULE_DATA_DIR=\"$(datadir)/vala-panel/plugins\" \
	-DDATADIR=\"$(datadir)/vala-panel\"


libtestapplet_la_LIBADD = \
	${top_builddir}/lib/libvalapanel.la \
	$(PACKAGE_LIBS) \
	$(LIBPEAS_LIBS)

libtestapplet_la_LDFLAGS = \
	-module \
	-avoid-version \
	$(AM_LDFLAGS)

libtestapplet_la_VALAFLAGS = \
	--vapidir=${top_builddir}/. \
	--vapidir=${top_builddir}/vapi \
	--pkg gtk+-3.0 \
	--pkg config \
	--pkg gio-unix-2.0 \
	--pkg libpeas-1.0 \
	--pkg PeasGtk-1.0 \
	--pkg vala-panel

dist-hook:
	cd $(distdir) && \
	rm $(libtestapplet_la_SOURCES:.vala=.c) libtestapplet_la_vala.stamp
