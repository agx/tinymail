m4_define([tny_major_version], [1])
m4_define([tny_minor_version], [0])
m4_define([tny_micro_version], [0])
m4_define([tny_version],
          [tny_major_version.tny_minor_version.tny_micro_version])
m4_define([tny_interface_age], [0])
m4_define([tny_binary_age],
          [m4_eval(100 * tny_minor_version + tny_micro_version)])
m4_define([tny_api_version], [1.0])
m4_define([lt_current], [m4_eval(100 * tny_minor_version + tny_micro_version - tny_interface_age)])
m4_define([lt_revision], [tny_interface_age])
m4_define([lt_age], [m4_eval(tny_binary_age - tny_interface_age)])

AC_INIT([libtinymail], [tny_version], [http://www.pvanhoof.be])

AC_CONFIG_SRCDIR(tinymail/tny-main.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

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
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_LIBTOOL

dnl ### default values (will get overwritten)
build_gnome=true
build_mozembed=true
with_html_component=mozembed
build_gtkhtml=false
PLATFORMDIR=libtinymail-gnome-desktop

dnl ### Select the platform library ##
AC_ARG_WITH(platform,
[  --with-platform=gnome-desktop                platform to compile [gnome-desktop]],
[case "${with_platform}" in
  gnome-desktop) PLATFORMDIR=libtinymail-gnome-desktop ;;
  olpc)
        PLATFORMDIR=libtinymail-olpc
        build_gtkhtml=false
        build_mozembed=false
        build_gnome=false
        with_html_component=none
        ;;
  maemo)
        PLATFORMDIR=libtinymail-maemo
        build_gtkhtml=false
        build_mozembed=false
        build_gnome=false
        with_html_component=none
        ;;
  gpe) PLATFORMDIR=libtinymail-gpe ;;
  *) PLATFORMDIR=${with_platform}
esac], [PLATFORMDIR=libtinymail-gnome-desktop
        with_platform=gnome-desktop])
AC_SUBST(PLATFORMDIR)


dnl ### Enable gnome features ##
AC_ARG_ENABLE(gnome,
[  --enable-gnome		build using gnome features [default=yes]],
[case "${enableval}" in
  yes) build_gnome=true ;;
  no)  build_gnome=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-gnome) ;;
esac],[build_gnome=$build_gnome])
AM_CONDITIONAL(BUILD_GNOME, test x$build_gnome = xtrue)

dnl ### Select the html component ##
AC_ARG_WITH(html-component,
[  --with-html-component		set which html component to use [default=mozembed]],
[case "$with_html_component" in
  mozembed) 
	build_mozembed=true
	build_gtkhtml=false
  ;;
  gtkhtml)
	build_mozembed=false
	build_gtkhtml=false
  ;;
  none)
	build_mozembed=false 
	build_gtkhtml=false
  ;;
  *) AC_MSG_ERROR(bad value ${with_html_component} for --with-html-component) ;;
esac], [build_mozembed=true 
	build_gtkhtml=false
	with_html_component=mozembed])

AM_CONDITIONAL(BUILD_MOZEMBED, test x$build_mozembed = xtrue)
AM_CONDITIONAL(BUILD_GTKHTML, test x$build_gtkhtml = xtrue)

dnl ### Enable the python language bindings ##
AC_ARG_ENABLE(python-bindings,
[  --enable-python-bindings		build python bindings [default=no]],
[case "${enableval}" in
  yes) build_pytinymail=true ;;
  no)  build_pytinymail=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-py-tinymail) ;;
esac],[build_pytinymail=false])
AM_CONDITIONAL(BUILD_PYTINYMAIL, test x$build_pytinymail = xtrue)

dnl ### Enable building the unit tests ##
AC_ARG_ENABLE(unit-tests,
[  --enable-unit-tests		build unit tests [default=no]],
[case "${enableval}" in
  yes) build_unittests=true ;;
  no)  build_unittests=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-unit-tests) ;;
esac],[build_unittests=false])
AM_CONDITIONAL(BUILD_UNITTESTS, test x$build_unittests = xtrue)

dnl ### gtk-doc
GTK_DOC_CHECK([1.0])

dnl ### libtinymail, the abstraction library ##
PKG_CHECK_MODULES(LIBTINYMAIL, glib-2.0 >= 2.6 gobject-2.0)
AC_SUBST(LIBTINYMAIL_CFLAGS)
AC_SUBST(LIBTINYMAIL_LIBS)

dnl ### libtinymailui, the ui abstraction library ##
PKG_CHECK_MODULES(LIBTINYMAILUI, glib-2.0 >= 2.6 gobject-2.0)
AC_SUBST(LIBTINYMAILUI_CFLAGS)
AC_SUBST(LIBTINYMAILUI_LIBS)

dnl ### libtinymail-camel, a camel implementation of libtinymail ##
PKG_CHECK_MODULES(LIBTINYMAIL_CAMEL, glib-2.0 >= 2.6 gobject-2.0 camel-1.2 camel-provider-1.2)
AC_SUBST(LIBTINYMAIL_CAMEL_CFLAGS)
AC_SUBST(LIBTINYMAIL_CAMEL_LIBS)

dnl ### libtinymail-gnomevfs, a tnystreamiface for gnome-vfs ##
if test x$build_gnome = xtrue; then
extragtkpkgs="libgnomeui-2.0 gnome-keyring-1"
extratnypkgs="gnome-vfs-2.0"
  PKG_CHECK_MODULES(LIBTINYMAIL_GNOMEVFS, glib-2.0 >= 2.6 gobject-2.0 gnome-vfs-2.0)
else
  LIBTINYMAIL_GNOMEVFS_CFLAGS=
  LIBTINYMAIL_GNOMEVFS_LIBS=
  extragtkpkgs=
  extratnypkgs=
fi
AC_SUBST(LIBTINYMAIL_GNOMEVFS_CFLAGS)
AC_SUBST(LIBTINYMAIL_GNOMEVFS_LIBS)

dnl ### libtinymailui-gtk, a gtk+ implementation of libtinymail-ui ##
PKG_CHECK_MODULES(LIBTINYMAILUI_GTK, glib-2.0 >= 2.6 gobject-2.0 $extragtkpkgs gtk+-2.0)
AC_SUBST(LIBTINYMAILUI_GTK_CFLAGS)
AC_SUBST(LIBTINYMAILUI_GTK_LIBS)

dnl ### libtinymailui-mozembed, a library that wraps the gtkmozembed widget ##
if test x$build_mozembed = xtrue; then
  extraplatpkgs="firefox-nspr firefox-nss firefox-xpcom firefox-gtkmozembed"
  extratnypkgs="$extratnypkgs $extraplatpkgs"
  PKG_CHECK_MODULES(LIBTINYMAILUI_MOZEMBED, $extraplatpkgs glib-2.0 >= 2.6 gobject-2.0 gtk+-2.0 $extragtkpkgs)
else
  LIBTINYMAILUI_MOZEMBED_CFLAGS=
  LIBTINYMAILUI_MOZEMBED_LIBS=
fi

dnl ### libtinymail-gnome-desktop, a GNOME platform library implementation ##
if test x$PLATFORMDIR = xlibtinymail-gnome-desktop; then
  PKG_CHECK_MODULES(LIBTINYMAIL_GNOME_DESKTOP, gconf-2.0 $extraplatpkgs libnm_glib glib-2.0 >= 2.6 gobject-2.0 gtk+-2.0 $extragtkpkgs)
else
  LIBTINYMAIL_GNOME_DESKTOP_CFLAGS=
  LIBTINYMAIL_GNOME_DESKTOP_LIBS=
fi
AC_SUBST(LIBTINYMAIL_GNOME_DESKTOP_CFLAGS)
AC_SUBST(LIBTINYMAIL_GNOME_DESKTOP_LIBS)


dnl ### libtinymail-olpc, a platform library implementation for the One Laptop Per Child device ##
if test x$PLATFORMDIR = xlibtinymail-olpc; then
 PKG_CHECK_MODULES(LIBTINYMAIL_OLPC, glib-2.0 >= 2.8 gobject-2.0 gtk+-2.0)
else
 LIBTINYMAIL_OLPC_CFLAGS=
 LIBTINYMAIL_OLPC_LIBS=
fi
AC_SUBST(LIBTINYMAIL_OLPC_CFLAGS)
AC_SUBST(LIBTINYMAIL_OLPC_LIBS)
	      

dnl ### libtinymail-maemo, a platform library implementation for the maemo platform ##
if test x$PLATFORMDIR = xlibtinymail-maemo; then
 PKG_CHECK_MODULES(LIBTINYMAIL_MAEMO, glib-2.0 >= 2.8 gobject-2.0 gconf-2.0 gtk+-2.0)
else
 LIBTINYMAIL_MAEMO_CFLAGS=
 LIBTINYMAIL_MAEMO_LIBS=
fi
AC_SUBST(LIBTINYMAIL_MAEMO_CFLAGS)
AC_SUBST(LIBTINYMAIL_MAEMO_LIBS)

dnl ### libtinymail-gpe, a platform library implementation for the GPE platform ##
if test x$PLATFORMDIR = xlibtinymail-gpe; then
 PKG_CHECK_MODULES(LIBTINYMAIL_GPE, glib-2.0 >= 2.8 gobject-2.0 gconf-2.0 gtk+-2.0)
else
 LIBTINYMAIL_GPE_CFLAGS=
 LIBTINYMAIL_GPE_LIBS=
fi
AC_SUBST(LIBTINYMAIL_GPE_CFLAGS)
AC_SUBST(LIBTINYMAIL_GPE_LIBS)


AC_SUBST(extraplatpkgs)
AC_SUBST(extragtkpkgs)

dnl ### libtinymail-test, a library for testing purposes ##
if test x$build_unittests = xtrue; then
  PKG_CHECK_MODULES(LIBTINYMAIL_TEST, gunit gtk+-2.0 glib-2.0 >= 2.6 gobject-2.0 camel-1.2 camel-provider-1.2 $extratnypkgs)
else
  LIBTINYMAIL_TEST_CFLAGS=
  LIBTINYMAIL_TEST_LIBS=
fi
AC_SUBST(LIBTINYMAIL_TEST_CFLAGS)
AC_SUBST(LIBTINYMAIL_TEST_LIBS)

dnl ### tinymail, the demo-ui ##
PKG_CHECK_MODULES(TINYMAIL, glib-2.0 >= 2.6 gobject-2.0 gtk+-2.0 $extratnypkgs)
AC_SUBST(TINYMAIL_CFLAGS)
AC_SUBST(TINYMAIL_LIBS)

dnl ### tinymail-python, the python language bindings ##
if test x$build_pytinymail = xtrue; then
  AM_PATH_PYTHON(2.3)
  AC_PATH_PROG(PYTHON, python, no)
  if test x$PYTHON = xno; then
       AC_MSG_ERROR(Please install python)
  fi
  PYGTK_CODEGENDIR="`$PKG_CONFIG --variable=codegendir pygtk-2.0`"

  changequote(<<, >>)dnl
  PY_VER=`$PYTHON -c 'import distutils.sysconfig; print distutils.sysconfig.get_config_vars("VERSION")[0];'`
  PY_LIB=`$PYTHON -c 'import distutils.sysconfig; print distutils.sysconfig.get_python_lib(standard_lib=1);'`
  PY_INC=`$PYTHON -c 'import distutils.sysconfig; print distutils.sysconfig.get_config_vars("INCLUDEPY")[0];'`
  PY_PREFIX=`$PYTHON -c 'import sys; print sys.prefix'`
  PY_EXEC_PREFIX=`$PYTHON -c 'import sys; print sys.exec_prefix'`
  changequote([, ])dnl
  if test -f $PY_INC/Python.h; then
    PYTHON_LIBS="-L$PY_LIB/config -lpython$PY_VER -lpthread -lutil"
    PYTHON_CFLAGS="-I$PY_INC"
  else
    AC_MSG_ERROR([Can't find Python.h])
  fi

  AC_PATH_PROG(PYGTK_CODEGEN, pygtk-codegen-2.0, no)
  if test "x$PYGTK_CODEGEN" = xno; then
	AC_MSG_ERROR(could not find pygtk-codegen-2.0 script)
  fi
  AC_MSG_CHECKING(for pygtk defs)
  PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
  
  AC_SUBST(PYGTK_DEFSDIR)
  AC_MSG_RESULT($PYGTK_DEFSDIR)

  PKG_CHECK_MODULES(TINYMAIL_PYTHON, pygobject-2.0 pygtk-2.0 gnome-vfs-2.0)
  TINYMAIL_PYTHON_CFLAGS="$TINYMAIL_PYTHON_CFLAGS $PYTHON_CFLAGS"
  TINYMAIL_PYTHON_LIBS="$TINYMAIL_PYTHON_LIBS $PYTHON_LIBS"
else
  TINYMAIL_PYTHON_CFLAGS=
  TINYMAIL_PYTHON_LIBS=
fi
AC_SUBST(PYGTK_CODEGENDIR)
AC_SUBST(TINYMAIL_PYTHON_CFLAGS)
AC_SUBST(TINYMAIL_PYTHON_LIBS)

BINDIR=$prefix/bin
AC_SUBST(BINDIR)



AC_OUTPUT([
po/Makefile.in
libtinymail.spec
Makefile
docs/Makefile
docs/devel/Makefile
docs/devel/reference/Makefile
bindings/Makefile
bindings/python/Makefile
libtinymail/Makefile
libtinymail/libtinymail.pc
libtinymail-camel/Makefile
libtinymail-camel/libtinymail-camel.pc
libtinymailui/Makefile
libtinymailui/libtinymailui.pc
libtinymailui-gtk/Makefile
libtinymailui-gtk/libtinymailui-gtk.pc
libtinymailui-mozembed/Makefile
libtinymailui-mozembed/libtinymailui-mozembed.pc
libtinymail-gnomevfs/Makefile
libtinymail-gnomevfs/libtinymail-gnomevfs.pc
libtinymail-gnome-desktop/Makefile
libtinymail-gnome-desktop/libtinymail-gnome-desktop.pc
libtinymail-olpc/Makefile
libtinymail-olpc/libtinymail-olpc.pc
libtinymail-maemo/Makefile
libtinymail-maemo/libtinymail-maemo.pc
libtinymail-gpe/Makefile
libtinymail-gpe/libtinymail-gpe.pc
libtinymail-test/Makefile
tinymail/Makefile
m4/Makefile
])

echo ""
echo "The following options are selected:"
echo ""
echo "  Enable GNOME features:       $build_gnome"
echo "  HTML component:              ${with_html_component}"
echo "  Python language bindings:    $build_pytinymail"
echo "  Unit tests:                  $build_unittests"
echo "  Target platform:             ${with_platform}"
echo "  Prefix:                      ${prefix}"
echo ""
echo "Version info:"
echo ""
echo "  Version:                     $VERSION"
echo "  API version:                 $API_VERSION"
echo "  ABI version:                 $LT_VERSION_INFO - $LT_CURRENT_MINUS_AGE"
echo ""


