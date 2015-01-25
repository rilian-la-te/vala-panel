-include $(top_srcdir)/common.mk

EXTRA_DIST = 

panel_resources = $(shell glib-compile-resources --sourcedir=$(top_srcdir)/data --generate-dependencies $(top_srcdir)/data/vala-panel.gresource.xml)

BUILT_SOURCES = \
	vala-panel-resources.h \
	vala-panel-resources.c

# resources
vala-panel-resources.h: $(top_srcdir)/data/vala-panel.gresource.xml
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(top_srcdir)/data --generate-header $(top_srcdir)/data/vala-panel.gresource.xml

vala-panel-resources.c: $(top_srcdir)/data/vala-panel.gresource.xml $(panel_resources)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(top_srcdir)/data --generate-source $(top_srcdir)/data/vala-panel.gresource.xml


bin_PROGRAMS = vala-panel

vala_panel_CPPFLAGS = $(simple_panel_CPPFLAGS)
nodist_vala_panel_SOURCES = vala-panel-resources.c
vala_panel_SOURCES = \
	panel-app.vala
#	Misc.vala 
#	configurator.vala 
#	Panel.vala 
#	Plugin.vala 

vala_panel_VALAFLAGS = \
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

vala_panel_CFLAGS = \
	$(GOBJECT_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(LIBPEAS_CFLAGS) \
	-DGETTEXT_PACKAGE=\""vala-panel"\" \
	-DPACKAGE_LIB_DIR=\""$(libdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/simple-panel"\" \
	-DPACKAGE_UI_DIR=\""$(datadir)/simple-panel/ui"\" \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

vala_panel_LIBADD = \
	$(GOBJECT_LIBS) \
	$(GTK3_LIBS) \
	$(LIBPEAS_LIBS) \
	-lm

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(vala_panel_SOURCES:.vala=.c)

dist-hook:
	cd $(distdir) && \
	rm $(vala_panel_SOURCES:.vala=.c) vala_panel_vala.stamp
