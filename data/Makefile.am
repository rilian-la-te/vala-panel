default_config_DATA = \
	default/config
default_configdir = $(sysconfdir)/xdg/simple-panel/default

default_config_panel_DATA = \
	default/panels/panel
default_config_paneldir = $(sysconfdir)/xdg/simple-panel/default/panels

two_panels_config_DATA = \
	two_panels/config
two_panels_configdir = $(sysconfdir)/xdg/simple-panel/two_panels

two_panels_config_panel_DATA = \
	two_panels/panels/bottom \
	two_panels/panels/top
two_panels_config_paneldir = $(sysconfdir)/xdg/simple-panel/two_panels/panels

gsettings_ENUM_NAMESPACE = org.simple.panel
gsettings_ENUM_FILES = \
        $(top_srcdir)/src/*.h \
        $(top_srcdir)/plugins/*.c \
        $(top_srcdir)/plugins/X/*.c \
        $(top_srcdir)/plugins/xkb/xkb.h

panel_gschemas_in_in = \
        gschemas/org.simple.panel.gschema.xml.in.in \
        gschemas/org.simple.panel.toplevel.gschema.xml.in.in \
        gschemas/org.simple.panel.toplevel.plugin.gschema.xml.in.in \
        gschemas/org.simple.panel.builtin.gschema.xml.in.in \
        gschemas/org.simple.panel.X.gschema.xml.in.in \
        gschemas/org.simple.panel.plugins.gschema.xml.in.in

panel_gschemas_in = $(panel_gschemas_in_in:.gschema.xml.in.in=.gschema.xml.in)
gsettings_SCHEMAS = $(panel_gschemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@
	
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

uidir=$(datadir)/simple-panel/ui
ui_in_files= \
	ui/launchtaskbar.glade \
	ui/icontasklist.glade \
	ui/netstatus.glade

ui_DATA = $(ui_in_files:.glade=.ui)

# Purge GtkBuilder UI files
.glade.ui:
	sed 's/<!--.*-->//' < $< | sed ':a;N;$$!ba;s/ *\n *</</g' > $@

CLEANFILES = \
	$(ui_DATA) \
	$(panel_gschemas_in) \
	$(gsettings_SCHEMAS)

simple_panel_images_DATA = \
	images/background.png \
	images/capslock-on.png \
	images/capslock-off.png \
	images/numlock-on.png \
	images/numlock-off.png \
	images/scrllock-on.png \
	images/scrllock-off.png \
	images/cpufreq-icon.png


simple_panel_imagesdir = $(datadir)/simple-panel/images

EXTRA_DIST = \
	default/panels/panel.in \
	$(simple_panel_DATA) \
	$(simple_panel_images_DATA) \
	$(ui_DATA) \
	$(panel_gschemas_in_in) \
	$(ui_in_files) \
	$(default_config_DATA) \
	$(two_panels_config_DATA)
