-include $(top_srcdir)/common.mk

EXTRA_DIST = 

panel_resources = $(shell glib-compile-resources --sourcedir=$(top_srcdir)/data --generate-dependencies $(top_srcdir)/data/libvalapanel.gresource.xml)

BUILT_SOURCES = \
	libvalapanel-resources.h \
	libvalapanel-resources.c

# resources
libvalapanel-resources.h: $(top_srcdir)/data/libvalapanel.gresource.xml
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(top_srcdir)/data --generate-header $(top_srcdir)/data/libvalapanel.gresource.xml

libvalapanel-resources.c: $(top_srcdir)/data/libvalapanel.gresource.xml $(panel_resources)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(top_srcdir)/data --generate-source $(top_srcdir)/data/libvalapanel.gresource.xml


pkglib_LTLIBRARIES = libvalapanel.la

libvalapanel_la_CPPFLAGS = $(simple_panel_CPPFLAGS)
nodist_libvalapanel_la_SOURCES = libvalapanel-resources.c
libvalapanel_la_SOURCES = \
	menu-maker.vala \
	plugin-settings.vala \
	css.vala \
	panel-plugin.vala \
	panel-toplevel.vala \
	misc.vala \
	icon-grid.vala
#	Misc.vala 
#	configurator.vala 
#	Panel.vala 
#	Plugin.vala 

libvalapanel_la_VALAFLAGS = \
	--library=libvalapanel \
	-H vala-panel.h \
	--vapi=$(top_srcdir)/vapi/vala-panel.vapi \
	--vapidir=../vapi \
	--vapidir=. \
	--pkg gtk+-3.0 \
	--pkg gio-2.0 \
	--pkg gio-unix-2.0 \
	--pkg gio-settingsbackend-2.0 \
	--pkg libpeas-1.0 \
	--pkg PeasGtk-1.0 \
	$(VALAFLAGS) \
	--pkg gee-0.8

libvalapanel_la_CFLAGS = \
	$(GOBJECT_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(LIBPEAS_CFLAGS) \
	-DG_SETTINGS_ENABLE_BACKEND \
	-DGETTEXT_PACKAGE=\""vala-panel"\" \
	-DPACKAGE_LIB_DIR=\""$(libdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/vala-panel"\" \
	-DPACKAGE_UI_DIR=\""$(datadir)/vala-panel/ui"\" \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

libvalapanel_la_LIBADD = \
	$(GOBJECT_LIBS) \
	$(GTK3_LIBS) \
	$(LIBPEAS_LIBS) \
	-lm

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(libvalapanel_la_SOURCES:.vala=.c)

dist-hook:
	cd $(distdir) && \
	rm $(libvalapanel_la_SOURCES:.vala=.c) libvalapanel_la_vala.stamp
