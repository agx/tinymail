AC_INIT(TinyMail, 1.0, http://www.pvanhoof.be)
AC_CONFIG_SRCDIR(tinymail/tny-main.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AM_CONFIG_HEADER(tinymail/config.h)
AM_CONFIG_HEADER(libtinymail/config.h)
AM_CONFIG_HEADER(libtinymail-camel/config.h)
AM_CONFIG_HEADER(libtinymailui/config.h)

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

PKG_CHECK_MODULES(LIBTINYMAIL, glib-2.0 >= 2.8 gobject-2.0)
AC_SUBST(LIBTINYMAIL_CFLAGS)
AC_SUBST(LIBTINYMAIL_LIBS)

PKG_CHECK_MODULES(LIBTINYMAILUI, glib-2.0 >= 2.8 gobject-2.0)
AC_SUBST(LIBTINYMAILUI_CFLAGS)
AC_SUBST(LIBTINYMAILUI_LIBS)

PKG_CHECK_MODULES(LIBTINYMAILCAMEL, glib-2.0 >= 2.8 gobject-2.0 camel-1.2 camel-provider-1.2)
AC_SUBST(LIBTINYMAILCAMEL_CFLAGS)
AC_SUBST(LIBTINYMAILCAMEL_LIBS)

dnl gnomeui-2 is for gnome_icon_lookup :-\

PKG_CHECK_MODULES(LIBTINYMAILUI_GTK, libgnomeui-2.0 gtk+-2.0 glib-2.0 >= 2.8 gobject-2.0)
AC_SUBST(LIBTINYMAILUI_GTK_CFLAGS)
AC_SUBST(LIBTINYMAILUI_GTK_LIBS)


PKG_CHECK_MODULES(TINYMAIL, gconf-2.0 gtk+-2.0 glib-2.0 >= 2.8 gobject-2.0)
AC_SUBST(TINYMAIL_CFLAGS)
AC_SUBST(TINYMAIL_LIBS)


BINDIR=$prefix/bin
AC_SUBST(BINDIR)

AC_OUTPUT([
Makefile
libtinymail/Makefile
libtinymail/libtinymail.pc
libtinymail-camel/Makefile
libtinymail-camel/libtinymail-camel.pc
libtinymailui/Makefile
libtinymailui/libtinymailui.pc
libtinymailui-gtk/Makefile
libtinymailui-gtk/libtinymailui-gtk.pc
tinymail/Makefile
])

