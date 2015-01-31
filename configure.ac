AC_PREREQ(2.53)
AC_INIT(vala-panel, 0.1,99, ria.freelander@gmail.com)
AM_INIT_AUTOMAKE([-Wno-portability no-dist-gzip dist-xz foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
IT_PROG_INTLTOOL([0.40.0], [no-xml])
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_PREREQ(2.2)
AC_PREFIX_DEFAULT(/usr/local)
AM_SILENT_RULES([yes])
LT_INIT([disable-static])

AC_SEARCH_LIBS([log], [m])

AC_SEARCH_LIBS([log], [m])
CFLAGS="$CFLAGS -DENABLE_GTK3"
pkg_modules="$pkg_modules gtk+-3.0"
pkg_modules="$pkg_modules \
             gio-unix-2.0 \
             gthread-2.0 \
             gmodule-2.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)
AC_PATH_PROG(GLIB_MKENUMS, glib-mkenums)
AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)

GLIB_GSETTINGS

GETTEXT_PACKAGE=vala-panel
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",
                   [The prefix for our gettext translation domains.])

# Package requirements
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.40.0])
PKG_CHECK_MODULES([GIO], [gio-2.0 >= 2.40.0])
PKG_CHECK_MODULES([GTK3], [gtk+-3.0 >= 3.12.0])


MODULEDIR=${libdir}/vala-panel
AC_SUBST(MODULEDIR)
AC_DEFINE_UNQUOTED(MODULEDIR, "$MODULEDIR",
                   [Installation directory for Vala Panel plugins])

# Technically unused right now but might be used in the future
MODULE_DATA_DIR=${datadir}/vala-panel/plugins
AC_SUBST(MODULE_DATA_DIR)
AC_DEFINE_UNQUOTED(MODULE_DATA_DIR, "$MODULEDIR",
                   [Installation directory for Vala Panel data])

# Wnck required for tasklist
PKG_CHECK_MODULES([WNCK3], [libwnck-3.0 >= 3.4.7])

# Required for power-applet
PKG_CHECK_MODULES([UPOWER], [upower-glib >= 0.9.20])

# Required for menu in the panel
#PKG_CHECK_MODULES([GMENU], [libgnome-menu-3.0 >= 3.10.1])

# Required for panel applets
PKG_CHECK_MODULES([LIBPEAS], [libpeas-gtk-1.0 >= 1.8.0])

# Because we target Linux.
PKG_CHECK_MODULES([GIO_UNIX], [gio-unix-2.0 >= 2.40.0])

AC_CONFIG_MACRO_DIR([m4])

GOBJECT_INTROSPECTION_CHECK([1.40.0])

# For various Vala things.
PKG_CHECK_MODULES([GEE], [gee-0.8])

# Need Vala.
AM_PROG_VALAC([0.26], [], [AC_MSG_ERROR([valac is required to build Vala Panel (it is on Vala, hmm...)])])

# Determine if upower is newer than we're supporting right now
PKG_CHECK_MODULES(UPOWER99, [upower-glib >= 0.9.99], [have_upower0999=yes], [have_upower0999=no])
if test "x$have_upower0999" = "xyes" ; then
    VALAFLAGS="$VALAFLAGS --define HAVE_UPOWER0999"
fi

# Check for GTK3.13+ (Le World Blow Up)
PKG_CHECK_MODULES(GTK313, [gtk+-3.0  >= 3.13], [have_gtk313=yes], [have_gtk313=no])
if test "x$have_gtk313" = "xyes" ; then
    VALAFLAGS="$VALAFLAGS --define HAVE_GTK313"
fi

AC_SUBST(VALAFLAGS)

AC_CONFIG_FILES([Makefile
                applets/Makefile
                applets/test/Makefile
                data/Makefile
                lib/Makefile
                po/Makefile.in
                app/Makefile
                vala-panel.pc
                app/vala-panel.gresource.xml
                data/libvalapanel.gresource.xml
                data/default/panels/panel
                data/two_panels/panels/top
                data/two_panels/panels/bottom
               ])
AC_OUTPUT

AC_MSG_RESULT([
        ValaPanel $VERSION
        ========

        prefix:                 ${prefix}
        libdir:                 ${libdir}
        sysconfdir:             ${sysconfdir}
        exec_prefix:            ${exec_prefix}
        bindir:                 ${bindir}
        datarootdir:            ${datarootdir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}
])
