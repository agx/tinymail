INCLUDES = \
	$(LIBTINYMAILUI_GTK_CFLAGS)	\
	-I$(top_builddir)/libtinymail   \
	-I$(top_builddir)/libtinymailui

lib_LTLIBRARIES = libtinymailui-gtk-1.0.la

libtinymailui_gtk_1_0_headers = 	\
	tny-msg-window.h		\
	tny-msg-header-list-model.h	\
	tny-account-tree-model.h	\
	tny-attach-list-model.h		\
	tny-text-buffer-stream.h

libtinymailui_gtk_1_0_la_SOURCES = 	\
	$(libtinymailui_gtk_1_0_headers)\
	tny-msg-window.c		\
	tny-msg-header-list-model.c	\
	tny-account-tree-model.c	\
	tny-attach-list-model-priv.h	\
	tny-attach-list-model.c		\
	tny-text-buffer-stream.c

libtinymailui_gtk_1_0_la_LIBADD = 	\
	$(LIBTINYMAILUI_GTK_LIBS)	\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la

libtinymailuigtkincludedir = $(includedir)/libtinymailui-gtk-$(API_VERSION)
libtinymailuigtkinclude_HEADERS = $(libtinymailui_gtk_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymailui-gtk-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
