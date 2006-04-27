AC_INIT(TinyMail, 1.0, http://www.pvanhoof.be)
AC_CONFIG_SRCDIR(tinymail/tny-main.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

AM_CONFIG_HEADER(tinymail/config.h)
AM_CONFIG_HEADER(libtinymail/config.h)
AM_CONFIG_HEADER(libtinymail-camel/config.h)
AM_CONFIG_HEADER(libtinymail-gnomevfs/config.h)
AM_CONFIG_HEADER(libtinymailui/config.h)
AM_CONFIG_HEADER(libtinymailui-gtk/config.h)

API_VERSION=1.0
AC_SUBST(API_VERSION)

if test x$prefix = xNONE; then
        prefix=/usr/local
fi

AC_SUBST(prefix)

AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_LIBTOOL


AC_ARG_ENABLE(py-tinymail,
[  --enable-py-tinymail    build python bindings [default=no]],
[case "${enableval}" in
  yes) build_pytinymail=true ;;
  no)  build_pytinymail=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-py-tinymail) ;;
esac],[build_pytinymail=false])
AM_CONDITIONAL(BUILD_PYTINYMAIL, test x$build_pytinymail = xtrue)

AC_ARG_ENABLE(unit-tests,
[  --enable-unit-tests    build unit tests [default=no]],
[case "${enableval}" in
  yes) build_unittests=true ;;
  no)  build_unittests=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-unit-tests) ;;
esac],[build_unittests=false])
AM_CONDITIONAL(BUILD_UNITTESTS, test x$build_unittests = xtrue)

GTK_DOC_CHECK([1.0])

PKG_CHECK_MODULES(LIBTINYMAIL, glib-2.0 >= 2.8 gobject-2.0)
AC_SUBST(LIBTINYMAIL_CFLAGS)
AC_SUBST(LIBTINYMAIL_LIBS)

PKG_CHECK_MODULES(LIBTINYMAILUI, glib-2.0 >= 2.8 gobject-2.0)
AC_SUBST(LIBTINYMAILUI_CFLAGS)
AC_SUBST(LIBTINYMAILUI_LIBS)

PKG_CHECK_MODULES(LIBTINYMAIL_CAMEL, glib-2.0 >= 2.8 gobject-2.0 camel-1.2 camel-provider-1.2)
AC_SUBST(LIBTINYMAIL_CAMEL_CFLAGS)
AC_SUBST(LIBTINYMAIL_CAMEL_LIBS)

PKG_CHECK_MODULES(LIBTINYMAIL_GNOMEVFS, glib-2.0 >= 2.8 gobject-2.0 gnome-vfs-2.0)
AC_SUBST(LIBTINYMAIL_GNOMEVFS_CFLAGS)
AC_SUBST(LIBTINYMAIL_GNOMEVFS_LIBS)

dnl gnomeui-2 is for gnome_icon_lookup :-\
PKG_CHECK_MODULES(LIBTINYMAILUI_GTK, glib-2.0 >= 2.8 gobject-2.0 libgnomeui-2.0 gtk+-2.0)
AC_SUBST(LIBTINYMAILUI_GTK_CFLAGS)
AC_SUBST(LIBTINYMAILUI_GTK_LIBS)

if test x$build_unittests = xtrue; then
PKG_CHECK_MODULES(LIBTINYMAIL_TEST, gunit gtk+-2.0 glib-2.0 >= 2.8 gobject-2.0 camel-1.2 camel-provider-1.2 gnome-vfs-2.0)
else
LIBTINYMAIL_TEST_CFLAGS=
LIBTINYMAIL_TEST_LIBS=
fi
AC_SUBST(LIBTINYMAIL_TEST_CFLAGS)
AC_SUBST(LIBTINYMAIL_TEST_LIBS)

dnl Try to remove the gtk+ and gconf dependency by refactoring to libraries
PKG_CHECK_MODULES(TINYMAIL, glib-2.0 >= 2.8 gobject-2.0 gconf-2.0 gtk+-2.0 gnome-vfs-2.0)
AC_SUBST(TINYMAIL_CFLAGS)
AC_SUBST(TINYMAIL_LIBS)

if test x$build_pytinymail = xtrue; then
AM_PATH_PYTHON(2.3)
AM_CHECK_PYTHON_HEADERS(,[AC_MSG_ERROR(could not find Python headers)])
AC_PATH_PROG(PYGTK_CODEGEN, pygtk-codegen-2.0, no)
if test "x$PYGTK_CODEGEN" = xno; then
	AC_MSG_ERROR(could not find pygtk-codegen-2.0 script)
fi
AC_MSG_CHECKING(for pygtk defs)
PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
AC_SUBST(PYGTK_DEFSDIR)
AC_MSG_RESULT($PYGTK_DEFSDIR)

PKG_CHECK_MODULES(TINYMAIL_PYTHON, pygobject-2.0 pygtk-2.0 gnome-vfs-2.0)
else
TINYMAIL_PYTHON_CFLAGS=
TINYMAIL_PYTHON_LIBS=
fi

AC_SUBST(TINYMAIL_PYTHON_CFLAGS)
AC_SUBST(TINYMAIL_PYTHON_LIBS)

BINDIR=$prefix/bin
AC_SUBST(BINDIR)

AC_OUTPUT([
Makefile
docs/Makefile
docs/devel/Makefile
docs/devel/reference/Makefile
libtinymail/Makefile
libtinymail/libtinymail.pc
libtinymail-camel/Makefile
libtinymail-camel/libtinymail-camel.pc
libtinymailui/Makefile
libtinymailui/libtinymailui.pc
libtinymailui-gtk/Makefile
libtinymailui-gtk/libtinymailui-gtk.pc
libtinymail-gnomevfs/Makefile
libtinymail-gnomevfs/libtinymail-gnomevfs.pc
libtinymail-test/Makefile
tinymail/Makefile
tinymail-python/Makefile
])

