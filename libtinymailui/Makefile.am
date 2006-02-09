INCLUDES = \
	$(LIBTINYMAILUI_CFLAGS)		\
	-I$(top_builddir)/libtinymail

lib_LTLIBRARIES = libtinymailui-1.0.la

libtinymailui_1_0_headers = 		\
	tny-msg-header-list-model.h	\
	tny-account-tree-model.h	\
	tny-summary-window-iface.h	\
	tny-msg-window-iface.h

libtinymailui_1_0_la_SOURCES = 		\
	$(libtinymailui_1_0_headers)	\
	tny-msg-header-list-model.c	\
	tny-account-tree-model.c	\
	tny-summary-window-iface.c	\
	tny-msg-window-iface.c

libtinymailui_1_0_la_LIBADD = 		\
	$(LIBTINYMAILUI_LIBS)		\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la

libtinymailuiincludedir = $(includedir)/libtinymailui-$(API_VERSION)
libtinymailuiinclude_HEADERS = $(libtinymailui_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymailui-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
