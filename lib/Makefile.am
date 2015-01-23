-include $(top_srcdir)/common.mk

EXTRA_DIST = 

panel_resources = $(shell glib-compile-resources --sourcedir=$(top_srcdir)/data --generate-dependencies $(top_srcdir)/data/libsimplepanel.gresource.xml)

BUILT_SOURCES = \
	libsimplepanel-resources.h \
	libsimplepanel-resources.c

CLEANFILES = \
	$(BUILT_SOURCES)

# resources
budgie-panel-resources.h: $(top_srcdir)/data/libsimplepanel.gresource.xml $(panel_resources)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(top_srcdir)/data --generate-header libsimplepanel.gresource.xml $<

libsimplepanel-resources.c: $(top_srcdir)/data/libsimplepanel.gresource.xml $(panel_resources)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(top_srcdir)/data --generate-source libsimplepanel.gresource.xml


pkglib_LTLIBRARIES = libsimplepanel.la

libsimplepanel_la_CPPFLAGS = $(simple_panel_CPPFLAGS)
nodist_libsimplepanel_la_SOURCES = libsimplepanel-resources.c
libsimplepanel_la_SOURCES = \
	Misc.vala \
	CSS.vala \
	MenuMaker.vala \
	configurator.c \
	IconGrid.vala \
	Panel.vala \
	Plugin.vala \
	PluginGSettings.vala
	
libsimplepanel_la_VALAFLAGS = \
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

budgie_panel_CFLAGS = \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
	$(GOBJECT_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(WNCK3_CFLAGS) \
	$(LIBPEAS_CFLAGS) \
	-DPACKAGE_LIB_DIR=\""$(libdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/simple-panel"\" \
	-DPACKAGE_UI_DIR=\""$(datadir)/simple-panel/ui"\" \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

budgie_panel_LDADD = \
	$(GOBJECT_LIBS) \
	$(GTK3_LIBS) \
	$(LIBPEAS_LIBS) \
	-lm


dist-hook:
	cd $(distdir) && \
	rm $(libsimplepanel_la_SOURCES:.vala=.c) libsimplepanel_la_vala.stamp
