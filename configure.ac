m4_define([tny_major_version], [2])
m4_define([tny_minor_version], [0])
m4_define([tny_micro_version], [0])
m4_define([tny_version],
          [tny_major_version.tny_minor_version.tny_micro_version])
m4_define([tny_interface_age], [0])
m4_define([tny_binary_age],
          [m4_eval(100 * tny_minor_version + tny_micro_version)])
m4_define([tny_api_version], [2.0])
m4_define([lt_current], [m4_eval(100 * tny_minor_version + tny_micro_version - tny_interface_age)])
m4_define([lt_revision], [tny_interface_age])
m4_define([lt_age], [m4_eval(tny_binary_age - tny_interface_age)])

AC_INIT([libtinymail], [tny_version], [http://tinymail.org/trac/tinymail])

m4_pattern_allow([^AC_TNY_])

AC_CONFIG_SRCDIR(libtinymail/tny-folder.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

# Support silent build rules, requires at least automake-1.11. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_HEADERS(config.h)

LT_VERSION_INFO="lt_current:lt_revision:lt_age"
LT_CURRENT_MINUS_AGE=m4_eval(lt_current - lt_age)
AC_SUBST(LT_VERSION_INFO)
AC_SUBST(LT_CURRENT_MINUS_AGE)

dnl ### Don't export functions that start with _ ##
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

GETTEXT_PACKAGE=tinymail
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for gettext])
AM_GLIB_DEFINE_LOCALEDIR([TNY_LOCALE_DIR])
ALL_LINGUAS=""
AM_GLIB_GNU_GETTEXT

API_VERSION=tny_api_version
AC_SUBST(API_VERSION)

if test x$prefix = xNONE; then
        prefix=/usr/local
fi

AC_SUBST(prefix)

AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_LIBTOOL

AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])
AC_PATH_PROG([GLIB_MKENUMS],[glib-mkenums])

dnl ### default values (will get overwritten)
build_nm=false
build_acap=false
build_gnome=true
build_uigtk=true
build_demoui=true
build_webkit=false
build_gtkhtml=false
build_doxygen=false
build_tests=false
build_unittests=false
build_tp=false
debug=false
enable_deprecated=true
PLATFORMDIR=libtinymail-gnome-desktop
PLATFORMSPECIFICNAME=Gnome
PLATFORM=1
PLATFORM_LIBS='$(top_builddir)/$(PLATFORMDIR)/$(PLATFORMDIR)-$(API_VERSION).la'
PLATFORM_CFLAGS='-I$(top_srcdir)/$(PLATFORM)'
have_platform=yes

dnl ### Enable NetworkManager ##
PKG_CHECK_MODULES(libnm_glib, libnm_glib, [have_libnm_glib="true"],
                  [have_libnm_glib="false"])
AC_ARG_ENABLE(nm,
AC_HELP_STRING([--enable-nm],
  [Build using NetworkManager support (yes, no)]),
[case "${enableval}" in
  yes)  enable_nm=true  ;;
  no)   enable_nm=false ;;
  auto) enable_nm=auto  ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-nm) ;;
esac],[enable_nm=$enable_nm])

dnl ### Debugging ###
AC_ARG_ENABLE(debug,
AC_HELP_STRING([--enable-debug],
  [Enable debugging (yes, no)]),
[case "${enableval}" in
  yes) debug=true ;;
  no)  debug=false ;;
esac],[debug=$debug])

dnl ### Deprecated disable ###
AC_ARG_ENABLE(deprecated,
  AC_HELP_STRING([--enable-deprecated],
  [Enable deprecated symbols (yes, no)]),
[case "${enableval}" in
  yes) enable_deprecated=true ;;
  no)  enable_deprecated=false ;;
esac],[enable_deprecated=$enable_deprecated])


dnl ### Select the platform library ##
AC_ARG_WITH(platform,
AC_HELP_STRING([--with-platform=gnome-desktop], 
  [Platform to compile (gnome-desktop, moblin, olpc or gpe)]),
[case "${with_platform}" in
  gnome-desktop) 
	PLATFORMDIR=libtinymail-gnome-desktop
	PLATFORMSPECIFICNAME=Gnome 
	build_uigtk=true
	build_demoui=true
	build_gtkhtml=true
	PLATFORM=1
	;;
  gpe) 
	PLATFORMDIR=libtinymail-gpe
	PLATFORMSPECIFICNAME=Gpe 
	build_uigtk=true
	build_demoui=true
	PLATFORM=3
	;;
  olpc)
        PLATFORMDIR=libtinymail-olpc
	PLATFORMSPECIFICNAME=Olpc
        build_gtkhtml=false
        build_gnome=false
        with_html_component=none
	build_uigtk=true
        build_demoui=true
        PLATFORM=4
        ;;
  moblin)
        PLATFORMDIR=libtinymail-moblin
	PLATFORMSPECIFICNAME=Moblin
        build_gtkhtml=false
        build_gnome=true
	build_uigtk=true
	build_demoui=true
	PLATFORM=5
        ;;
  none)
	PLATFORMDIR=
	PLATFORMSPECIFICNAME=
	build_uigtk=false
	build_demoui=false
	PLATFORM=0
	PLATFORM_LIBS=
	PLATFORM_CFLAGS=
	have_platform=no
	;;
  *) 
	PLATFORMDIR=${with_platform}
	PLATFORMSPECIFICNAME=
	build_uigtk=false
	build_demoui=false
	PLATFORM=0
esac], [PLATFORMDIR=libtinymail-gnome-desktop
        with_platform=gnome-desktop])

AC_SUBST(PLATFORMDIR)
AC_SUBST(PLATFORM)
AC_SUBST(PLATFORMSPECIFICNAME)
AC_SUBST(PLATFORM_LIBS)
AC_SUBST(PLATFORM_CFLAGS)

AM_CONDITIONAL([HAVE_PLATFORM], [test x$have_platform = xyes])

dnl ### Select the html component ##
AC_ARG_WITH(html-component,
AC_HELP_STRING([--with-html-component],
  [Set which html component to use (gtkhtml, webkit, none)]),
[case "$with_html_component" in
  webkit)
	build_gtkhtml=false
	build_webkit=true
  ;;
  gtkhtml)
	build_webkit=false
	build_gtkhtml=true
	build_demoui=true
  ;;
  none)
	with_html_component=none
	build_gtkhtml=false
	build_webkit=false
  ;;
  *) AC_MSG_ERROR(bad value ${with_html_component} for --with-html-component) ;;
esac], [build_webkit=false 
	build_gtkhtml=false
	with_html_component=none])

AM_CONDITIONAL(BUILD_GTKHTML, test x$build_gtkhtml = xtrue)
AM_CONDITIONAL(BUILD_WEBKIT, test x$build_webkit = xtrue)

dnl ### Enable demoui ##
AC_ARG_ENABLE(demoui,
AC_HELP_STRING([--enable-demoui],
  [Build the demo user interface (yes, no)]),
[case "${enableval}" in
  yes) 
	build_demoui=true 
	build_uigtk=true
  ;;
  no)  build_demoui=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-demoui) ;;
esac],[build_demoui=$build_demoui])
AM_CONDITIONAL(BUILD_DEMOUI, test x$build_demoui = xtrue)

dnl ### Enable IMAP part-fetch feature ##
build_imapp=true
AC_ARG_ENABLE(imap-part-fetch,
AC_HELP_STRING([--enable-imap-part-fetch],
  [Build using imap part fetch]),
[case "${enableval}" in
  yes)
        build_imapp=true
  ;;
  no)  build_imapp=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-part-fetch) ;;
esac],[build_imapp=$build_imapp])
AM_CONDITIONAL(BUILD_IMAPP, test x$build_imapp = xtrue)

dnl ### Enable gnome features ##
AC_ARG_ENABLE(gnome,
AC_HELP_STRING([--enable-gnome],
  [Build using gnome features]),
[case "${enableval}" in
  yes) 
	build_gnome=true 
	build_uigtk=true
  ;;
  no)  build_gnome=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-gnome) ;;
esac],[build_gnome=$build_gnome])
AM_CONDITIONAL(BUILD_GNOME, test x$build_gnome = xtrue)


dnl ### Enable acap features ##
AC_ARG_ENABLE(acap,
AC_HELP_STRING([--enable-acap],
  [Build using acap features]),
[case "${enableval}" in
  yes) build_acap=true ;;
  no)  build_acap=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-acap) ;;
esac],[build_acap=$build_acap])
AM_CONDITIONAL(BUILD_ACAP, test x$build_acap = xtrue)

dnl ### Enable building introspection ##
AC_ARG_ENABLE([introspection],
        AS_HELP_STRING([--enable-introspection],
		       [Enable GObject introspection]),
       	[enable_introspection=$enableval],
	[enable_introspection=yes])
AC_MSG_RESULT([$enable_introspection])

G_IR_SCANNER=
G_IR_COMPILER=
G_IR_GENERATE=
GIRDIR=
GIRTYPELIBDIR=

if test "$enable_introspection" = "yes"; then
  GOBJECT_INTROSPECTION_REQUIRED=0.6
  PKG_CHECK_MODULES([GOBJECT_INTROSPECTION],[gobject-introspection-1.0 >= $GOBJECT_INTROSPECTION_REQUIRED])

  G_IR_SCANNER="$($PKG_CONFIG --variable=g_ir_scanner gobject-introspection-1.0)"
  G_IR_COMPILER="$($PKG_CONFIG --variable=g_ir_compiler gobject-introspection-1.0)"
  G_IR_GENERATE="$($PKG_CONFIG --variable=g_ir_generate gobject-introspection-1.0)"
  GIRDIR="$($PKG_CONFIG --variable=girdir gobject-introspection-1.0)"
  GIRTYPELIBDIR="$($PKG_CONFIG --variable=typelibdir gobject-introspection-1.0)"
fi

AC_SUBST([G_IR_SCANNER])
AC_SUBST([G_IR_COMPILER])
AC_SUBST([G_IR_GENERATE])
AC_SUBST([GIRDIR])
AC_SUBST([GIRTYPELIBDIR])

AM_CONDITIONAL([ENABLE_INTROSPECTION],[test "$enable_introspection" = "yes"])

dnl ### Enable building the unit tests ##
AC_ARG_ENABLE(unit-tests,
AC_HELP_STRING([--enable-unit-tests],
  [Build unit tests (no, yes)]),
[case "${enableval}" in
  yes) 
	build_tests=true 
	build_unittests=true 
	build_uigtk=true
  ;;
  no)  build_unittests=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-unit-tests) ;;
esac],[build_unittests=$build_unittests])
AM_CONDITIONAL(BUILD_UNITTESTS, test x$build_unittests = xtrue)

dnl ### Enable building the tests ##
AC_ARG_ENABLE(tests,
AC_HELP_STRING([--enable-tests],
  [Build tests (no, yes)]),
[case "${enableval}" in
  yes) 
	build_tests=true 
	build_uigtk=true
  ;;
  no)  build_tests=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-tests) ;;
esac],[build_tests=$build_tests])
AM_CONDITIONAL(BUILD_TESTS, test x$build_tests = xtrue)

dnl ### Enable uigtk ##
AC_ARG_ENABLE(uigtk, 
AC_HELP_STRING([--enable-uigtk],
  [Build the gtk+ implementation for the ui components (yes, no)]),
[case "${enableval}" in
  yes) build_uigtk=true ;;
  no)  build_uigtk=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-uigtk) ;;
esac],[build_uigtk=$build_uigtk])
AM_CONDITIONAL(BUILD_UIGTK, test x$build_uigtk = xtrue)

dnl ### Enable GMail support ##
AC_ARG_ENABLE(tp,
AC_HELP_STRING([--enable-tp],
  [Build the integration with telepathy (yes, no)]),
[case "${enableval}" in
  yes) build_tp=true ;;
  no)  build_tp=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-tp) ;;
esac],[build_tp=$build_tp])
AM_CONDITIONAL(BUILD_TP, test x$build_tp = xtrue)

dnl ### Enable the python language bindings ##
AC_ARG_ENABLE(python-bindings,
AC_HELP_STRING([--enable-python-bindings],
  [Build python bindings (no, yes)]),
[case "${enableval}" in
  yes) build_pytinymail=true ;;
  no)  build_pytinymail=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-python-bindings) ;;
esac],[build_pytinymail=false])
AM_CONDITIONAL(BUILD_PYTINYMAIL, test x$build_pytinymail = xtrue)

dnl ### Enable the Vala language bindings ##
AC_ARG_ENABLE(vala-bindings,
AC_HELP_STRING([--enable-vala-bindings],
  [Build vala bindings (no, yes)]),
[case "${enableval}" in
  yes) build_vala_bindings=true ;;
  no)  build_vala_bindings=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-vala-bindings) ;;
esac],[build_vala_bindings=false])
AM_CONDITIONAL(BUILD_VALA_BINDINGS, test x$build_vala_bindings = xtrue)


dnl ### Enable the .NET language bindings ##
AC_ARG_ENABLE(dotnet-bindings,
AC_HELP_STRING([--enable-dotnet-bindings],
  [Build .NET bindings (no, yes)]),
[case "${enableval}" in
  yes) build_net_bindings=true ;;
  no)  build_net_bindings=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-dotnet-bindings) ;;
esac],[build_net_bindings=false])

AC_TNY_DOTNET_CHECK

AM_CONDITIONAL(BUILD_NET_BINDINGS, test x$build_net_bindings = xtrue)


dnl ### doxygen
AC_ARG_ENABLE(doxygen,
AC_HELP_STRING([--enable-doxygen],
  [Build doxygen docs (no, yes)]),
[case "${enableval}" in
  yes) build_doxygen=true ;;
  no)  build_doxygen=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-doxygen) ;;
esac],[build_doxygen=false])
AM_CONDITIONAL(BUILD_DOXYGEN, test x$build_doxygen = xtrue)

if test x$build_doxygen = xtrue; then
  AC_CHECK_PROGS(DOXYGEN,doxygen, [You need doxygen])
  AC_CHECK_PROGS(PERL,perl, [You need perl])
fi

dnl ### gtk-doc
GTK_DOC_CHECK([1.0])

dnl ### libtinymail, the abstraction library ##
PKG_CHECK_MODULES(LIBTINYMAIL, glib-2.0 >= 2.32 gobject-2.0)
dnl ### libtinymailui, the ui abstraction library ##
PKG_CHECK_MODULES(LIBTINYMAILUI, glib-2.0 >= 2.32 gobject-2.0)
dnl ### libtinymail-camel, a camel implementation of libtinymail ##
PKG_CHECK_MODULES(LIBTINYMAIL_CAMEL, glib-2.0 >= 2.32 gobject-2.0)

if test x$debug == xtrue; then
	EXTRA_CFLAGS="-DDEBUG -DDBC -g -ggdb -O0"
else
	EXTRA_CFLAGS="-DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"
fi

if test x$enable_deprecated == xfalse; then
	EXTRA_CFLAGS="$EXTRA_CFLAGS -DTNY_DISABLE_DEPRECATED"
fi

AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(LIBTINYMAIL_CAMEL_CFLAGS)
AC_SUBST(LIBTINYMAIL_CAMEL_LIBS)
AC_SUBST(LIBTINYMAILUI_CFLAGS)
AC_SUBST(LIBTINYMAILUI_LIBS)
AC_SUBST(LIBTINYMAIL_CFLAGS)
AC_SUBST(LIBTINYMAIL_LIBS)

dnl ### libtinymail-tp, integration with telepathy
if test x$build_tp = xtrue; then
PKG_CHECK_MODULES(LIBTINYMAIL_TP, telepathy-glib)
else
LIBTINYMAIL_TP_CFLAGS=
LIBTINYMAIL_TP_LIBS=
fi
AC_SUBST(LIBTINYMAIL_TP_CFLAGS)
AC_SUBST(LIBTINYMAIL_TP_LIBS)

extragtkpkgs=
extratnypkgs=

dnl ### libtinymail-gio, a tnystreamiface for gio ##
build_gio="false"
PKG_CHECK_MODULES(LIBTINYMAIL_GIO, gio-2.0, [have_gio="true"],
					    [have_gio="false"])
AC_ARG_ENABLE(gio,
AC_HELP_STRING([--enable-gio],
	       [Build using Gio support (yes, no, auto)]),
	       [case "${enableval}" in
		  yes)  enable_gio=true  ;;
		  no)   enable_gio=false ;;
		  auto) enable_gio=auto  ;;
		  *) AC_MSG_ERROR(bad value ${enableval} for --enable-gio) ;;
	       esac],
	       [enable_gio=$enable_gio])
if test x$have_gio = xtrue || test x$enable_gio = xtrue; then
build_gio="true"
fi
if test x$enable_gio = xfalse; then
build_gio="false"
fi
if test x$enable_gio = xtrue && test x$have_gio = xfalse; then
AC_MSG_ERROR(GIO requested but not available)
fi
if test x$build_gnome = xtrue && test x$build_gio = xfalse; then
AC_MSG_ERROR(GIO is required for Gnome port)
fi
if test x$build_gio = xtrue; then
extratnypkgs="$extratnypkgs gio-2.0"
fi
AM_CONDITIONAL(BUILD_GIO, test x$build_gio = xtrue)
AC_SUBST(LIBTINYMAIL_GIO_CFLAGS)
AC_SUBST(LIBTINYMAIL_GIO_LIBS)

dnl ### libtinymail-gnomevfs, a tnystreamiface for gio ##
build_gnomevfs="false"
PKG_CHECK_MODULES(LIBTINYMAIL_GNOMEVFS, glib-2.0 >= 2.8 gobject-2.0 gnome-vfs-2.0, [have_gnomevfs="true"],
     					    	     	    			   [have_gnomevfs="false"])
AC_ARG_ENABLE(gnomevfs,
AC_HELP_STRING([--enable-gnomevfs],
	       [Build using GnomeVFS support (yes, no, auto)]),
	       [case "${enableval}" in
		  yes)  enable_gnomevfs=true  ;;
		  no)   enable_gnomevfs=false ;;
		  auto) enable_gnomevfs=auto  ;;
		  *) AC_MSG_ERROR(bad value ${enableval} for --enable-gnomevfs) ;;
	       esac],
	       [enable_gnomevfs=$enable_gnomevfs])
if test x$have_gnomevfs = xtrue || test x$enable_gnomevfs = xtrue; then
build_gnomevfs="true"
fi
if test x$enable_gnomevfs = xfalse; then
build_gnomevfs="false"
fi
if test x$enable_gnomevfs = xtrue && test x$have_gnomevfs = xfalse; then
AC_MSG_ERROR(GnomeVFS requested but not available)
fi
if test x$build_gnomevfs = xtrue; then
extratnypkgs="$extratnypkgs gnome-vfs-2.0"
fi
AM_CONDITIONAL(BUILD_GNOMEVFS, test x$build_gnomevfs = xtrue)
AC_SUBST(LIBTINYMAIL_GNOMEVFS_CFLAGS)
AC_SUBST(LIBTINYMAIL_GNOMEVFS_LIBS)

if test x$build_gnome = xtrue; then
   extragtkpkgs="gnome-keyring-1 gio-2.0"
fi

dnl ### libtinymailui-gtk, a gtk+ implementation of libtinymail-ui ##
if test x$build_uigtk = xtrue; then
  PKG_CHECK_MODULES(LIBTINYMAILUI_GTK, glib-2.0 >= 2.8 gobject-2.0 $extragtkpkgs gtk+-3.0)
else
  LIBTINYMAILUI_GTK_CFLAGS=
  LIBTINYMAILUI_GTK_LIBS=
fi
AC_SUBST(LIBTINYMAILUI_GTK_CFLAGS)
AC_SUBST(LIBTINYMAILUI_GTK_LIBS)

dnl ### libtinymailui-gtkhtml, a library that wraps the GtkHtml widget ##
if test x$build_gtkhtml = xtrue; then
  extraplatpkgs="libgtkhtml-3.8"
  extratnypkgs="$extratnypkgs $extraplatpkgs"
  PKG_CHECK_MODULES(LIBTINYMAILUI_GTKHTML, $extraplatpkgs glib-2.0 >= 2.8 gobject-2.0 gtk+-3.0 $extragtkpkgs)
else
  LIBTINYMAILUI_GTKHTML_CFLAGS=
  LIBTINYMAILUI_GTKHTML_LIBS=
fi

dnl ### libtinymail-gnome-desktop, a GNOME platform library implementation ##
if test x$have_libnm_glib = xtrue || test x$enable_nm = xtrue; then
build_nm="true"
fi
if test x$enable_nm = xfalse; then
build_nm="false"
fi

if test x$build_nm = xtrue; then
extranmpkgs="libnm_glib"
AC_DEFINE_UNQUOTED(NM_SUPPORT, 1, ["NetworkManager support."])
fi

if test x$PLATFORMDIR = xlibtinymail-gnome-desktop; then
  PKG_CHECK_MODULES(LIBTINYMAIL_GNOME_DESKTOP, gconf-2.0 $extraplatpkgs glib-2.0 >= 2.8 gobject-2.0 gtk+-3.0 $extragtkpkgs $extranmpkgs)
else
  LIBTINYMAIL_GNOME_DESKTOP_CFLAGS=
  LIBTINYMAIL_GNOME_DESKTOP_LIBS=
fi
AC_SUBST(LIBTINYMAIL_GNOME_DESKTOP_CFLAGS)
AC_SUBST(LIBTINYMAIL_GNOME_DESKTOP_LIBS)


dnl ### libtinymail-olpc, a platform library implementation for the One Laptop Per Child device ##
if test x$PLATFORMDIR = xlibtinymail-olpc; then
 PKG_CHECK_MODULES(LIBTINYMAIL_OLPC, glib-2.0 >= 2.8 gobject-2.0 gtk+-3.0)
else
 LIBTINYMAIL_OLPC_CFLAGS=
 LIBTINYMAIL_OLPC_LIBS=
fi
AC_SUBST(LIBTINYMAIL_OLPC_CFLAGS)
AC_SUBST(LIBTINYMAIL_OLPC_LIBS)
	      

AC_DEFUN([ARMEL_TARGET],
	[if sb-conf cu | grep -i 'armel' >/dev/null 2>&1; then
	    armel_target=yes])

dnl ### libtinymail-gpe, a platform library implementation for the GPE platform ##
if test x$PLATFORMDIR = xlibtinymail-gpe; then
 PKG_CHECK_MODULES(LIBTINYMAIL_GPE, glib-2.0 >= 2.8 gobject-2.0 gconf-2.0 gtk+-3.0)
else
 LIBTINYMAIL_GPE_CFLAGS=
 LIBTINYMAIL_GPE_LIBS=
fi
AC_SUBST(LIBTINYMAIL_GPE_CFLAGS)
AC_SUBST(LIBTINYMAIL_GPE_LIBS)


AC_SUBST(extraplatpkgs)
AC_SUBST(extragtkpkgs)

dnl ### libtinymail-moblin, a platform library implementation for the Moblin platform ##
if test x$PLATFORMDIR = xlibtinymail-moblin; then
 PKG_CHECK_MODULES(LIBTINYMAIL_MOBLIN, glib-2.0 >= 2.8 gobject-2.0 gconf-2.0 gtk+-3.0 connman)
 AC_DEFINE_UNQUOTED(CONNMAN_SUPPORT, 1, ["Moblin Connection Manager support."])
else
 LIBTINYMAIL_MOBLIN_CFLAGS=
 LIBTINYMAIL_MOBLIN_LIBS=
fi
AC_SUBST(LIBTINYMAIL_MOBLIN_CFLAGS)
AC_SUBST(LIBTINYMAIL_MOBLIN_LIBS)

AC_SUBST(extraplatpkgs)
AC_SUBST(extragtkpkgs)

dnl ### libtinymail-test, a library for testing purposes ##
if test x$build_unittests = xtrue; then
  PKG_CHECK_MODULES(CHECK, check >= 0.9.4 glib-2.0)
fi

dnl ### tinymail, the demo-ui ##
if test x$build_demoui = xtrue; then
  PKG_CHECK_MODULES(TINYMAIL, glib-2.0 >= 2.32 gobject-2.0 gtk+-3.0 $extratnypkgs)
else
  TINYMAIL_CFLAGS=
  TINYMAIL_LIBS=
fi
AC_SUBST(TINYMAIL_CFLAGS)
AC_SUBST(TINYMAIL_LIBS)

BINDIR=$prefix/bin
AC_SUBST(BINDIR)

AC_CONFIG_SUBDIRS(libtinymail-camel/camel-lite)

AC_OUTPUT([
po/Makefile.in
libtinymail.spec
Makefile
docs/Makefile
docs/devel/Makefile
docs/devel/reference/Makefile
docs/devel/doxygen/Makefile
docs/devel/doxygen/libtinymail
docs/devel/doxygen/libtinymail-camel
docs/devel/doxygen/libtinymailui
docs/devel/doxygen/libtinymailui-gtk
libtinymail/Makefile
libtinymail/libtinymail.pc
libtinymail-camel/Makefile
libtinymail-camel/libtinymail-camel.pc
libtinymail-tp/libtinymail-tp.pc
libtinymail-tp/Makefile
libtinymailui/Makefile
libtinymailui/libtinymailui.pc
libtinymailui-gtk/Makefile
libtinymailui-gtk/libtinymailui-gtk.pc
libtinymailui-webkit/Makefile
libtinymailui-webkit/libtinymailui-webkit.pc
libtinymailui-gtkhtml/Makefile
libtinymailui-gtkhtml/libtinymailui-gtkhtml.pc
libtinymailui-gnome-keyring/Makefile
libtinymail-gio/Makefile
libtinymail-gio/libtinymail-gio.pc
libtinymail-gnome-desktop/Makefile
libtinymail-gnome-desktop/libtinymail-gnome-desktop.pc
libtinymail-olpc/Makefile
libtinymail-olpc/libtinymail-olpc.pc
libtinymail-gpe/Makefile
libtinymail-gpe/libtinymail-gpe.pc
libtinymail-test/Makefile
libtinymail-acap/Makefile
libtinymail-acap/libtinymail-acap.pc
libtinymail-moblin/Makefile
libtinymail-moblin/libtinymail-moblin.pc
tests/Makefile
tests/c-demo/Makefile
tests/shared/Makefile
tests/memory/Makefile
tests/functional/Makefile
m4/Makefile
])

echo ""
echo "The following options are selected:"
echo ""
echo "  Enable Gio support:          $build_gio"
echo "  Enable GnomeVFS support:     $build_gnomevfs"
echo "  Enable GNOME features:       $build_gnome"
echo "  Enable NetworkManager:       $build_nm"
echo "  Enable ACAP features:        $build_acap"
echo "  Enable Telepathy features:   $build_tp"
echo "  HTML component:              ${with_html_component}"
echo "  Enable Bodystruct support:   ${build_imapp}"
echo "  Python language bindings:    $build_pytinymail"
echo "  API tests:                   $build_apitests"
echo "  Unit tests:                  $build_unittests"
echo "  Tests:                       $build_tests"
echo "  Target platform:             ${with_platform}"
echo "  Prefix:                      ${prefix}"
echo ""
echo "Version info:"
echo ""
echo "  Version:                     $VERSION"
echo "  API version:                 $API_VERSION"
echo "  ABI version:                 $LT_VERSION_INFO - $LT_CURRENT_MINUS_AGE"
echo ""


