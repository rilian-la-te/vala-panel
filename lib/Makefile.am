-include $(top_srcdir)/common.mk

EXTRA_DIST = 

panel_resources = $(shell glib-compile-resources --generate-dependencies libvalapanel.gresource.xml)

BUILT_SOURCES = \
	libvalapanel-resources.h \
	libvalapanel-resources.c

# resources
libvalapanel-resources.h: libvalapanel.gresource.xml
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header libvalapanel.gresource.xml

libvalapanel-resources.c: libvalapanel.gresource.xml $(panel_resources)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source libvalapanel.gresource.xml


pkglib_LTLIBRARIES = libvalapanel.la

nodist_libvalapanel_la_SOURCES = libvalapanel-resources.c
libvalapanel_la_SOURCES = \
	menu-maker.vala \
	plugin-settings.vala \
	css.vala \
	panel-plugin.vala \
	panel-toplevel.vala \
	misc.vala \
	icon-grid.vala \
	configurator.vala 

libvalapanel_la_VALAFLAGS = \
	--library=libvalapanel \
	-H vala-panel.h \
	--target-glib=2.40 \
	--vapi=$(top_srcdir)/vapi/vala-panel.vapi \
	--gresources=$(srcdir)/libvalapanel.gresource.xml \
	--vapidir=../vapi \
	--vapidir=. \
	--pkg config \
	--pkg gtk+-3.0 \
	--pkg gio-2.0 \
	--pkg gio-unix-2.0 \
	--pkg gio-settingsbackend-2.0 \
	--pkg libpeas-1.0 \
	--pkg PeasGtk-1.0 \
	$(VALAFLAGS) \
	--pkg gee-0.8

libvalapanel_la_CPPFLAGS = \
	$(PACKAGE_CFLAGS) \
	$(LIBPEAS_CFLAGS) \
	-DG_SETTINGS_ENABLE_BACKEND \
	-DGETTEXT_PACKAGE=\""vala-panel"\" \
	-DPACKAGE_LIB_DIR=\""$(libdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/vala-panel"\" \
	-DPACKAGE_UI_DIR=\""$(datadir)/vala-panel/ui"\" \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

libvalapanel_la_LIBADD = \
	$(PACKAGE_LIBS) \
	$(LIBPEAS_LIBS) \
	-lm

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(libvalapanel_la_SOURCES:.vala=.c)

dist-hook:
	cd $(distdir) && \
	rm $(libvalapanel_la_SOURCES:.vala=.c) libvalapanel_la_vala.stamp
