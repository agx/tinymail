INCLUDES = \
	$(LIBTINYMAILUI_CFLAGS)		\
	-I$(top_builddir)/libtinymail

lib_LTLIBRARIES = libtinymailui-1.0.la

libtinymailui_1_0_headers = 		\
	tny-msg-header-list-model.h	\
	tny-msg-account-tree-model.h

libtinymailui_1_0_la_SOURCES = 		\
	$(libtinymailui_1_0_headers)	\
	tny-msg-header-list-model.c	\
	tny-msg-account-tree-model.c

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
