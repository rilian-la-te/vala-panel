## Process this file with automake to produce Makefile.in

bin_PROGRAMS =vala-panel

vala_panel_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/libvalapanel \
	-DGETTEXT_PACKAGE=\""vala-panel"\" \
	-DPACKAGE_LIB_DIR=\""$(libdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/vala-panel"\" \
	-DPACKAGE_UI_DIR=\""$(datadir)/vala-panel/ui"\" \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DG_SETTINGS_ENABLE_BACKEND \
	$(PACKAGE_CFLAGS) \
	$(LIBPEAS_CFLAGS) \
	$(G_CAST_CHECKS)

vala_panel_resdeps = $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies vala-panel.gresource.xml)

vala-panel-resources.c: vala-panel.gresource.xml $(simple_panel_resdeps)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source vala-panel.gresource.xml

BUILT_SOURCES = \
	vala-panel-resources.c

nodist_vala_panel_SOURCES = vala-panel-resources.c
vala_panel_SOURCES = runner.vala application.vala

vala_panel_VALAFLAGS = \
	--vapidir=$(top_srcdir)/vapi \
	--vapidir=. \
	--target-glib=2.40 \
	--gresources=$(srcdir)/vala-panel.gresource.xml \
	--pkg gtk+-3.0 \
	--pkg gio-2.0 \
	--pkg gio-unix-2.0 \
	--pkg libpeas-1.0 \
	--pkg config \
	--pkg vala-panel \
	--pkg gio-settingsbackend-2.0 \
	--pkg posix \
	$(VALAFLAGS)

vala_panel_LDFLAGS = -export-dynamic -rpath $(pkglibdir)

vala_panel_LDADD = \
		$(top_builddir)/lib/libvalapanel.la \
		$(PACKAGE_LIBS) \
		$(INTLLIBS) \
		$(LIBPEAS_LIBS)
##		$(X11_LIBS) \

CLEANFILES = \
	vala-panel-resources.c \
	vala-panel.gresource.xml \
	$(simple_panel_SOURCES:.vala=.c)
