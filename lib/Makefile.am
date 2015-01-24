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
	css.vala 
#	Misc.vala 
#	configurator.vala 
#	IconGrid.vala 
#	Panel.vala 
#	Plugin.vala 

libvalapanel_la_VALAFLAGS = \
	--library=libvalapanel \
	-H libvalapanel.h \
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
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
	$(GOBJECT_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(LIBPEAS_CFLAGS) \
	-DG_SETTINGS_ENABLE_BACKEND \
	-DGETTEXT_PACKAGE=\""vala-panel"\" \
	-DPACKAGE_LIB_DIR=\""$(libdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/simple-panel"\" \
	-DPACKAGE_UI_DIR=\""$(datadir)/simple-panel/ui"\" \
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
