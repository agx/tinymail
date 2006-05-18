INCLUDES = \
	$(LIBTINYMAILUI_CFLAGS)		\
	-I$(top_srcdir)/libtinymail

lib_LTLIBRARIES = libtinymailui-1.0.la

libtinymailui_1_0_headers = 		\
	tny-summary-window-iface.h	\
	tny-msg-view-iface.h		\
	tny-msg-window-iface.h		\
	tny-platform-factory-iface.h

libtinymailui_1_0_la_SOURCES = 		\
	$(libtinymailui_1_0_headers)	\
	tny-summary-window-iface.c	\
	tny-msg-view-iface.c		\
	tny-msg-window-iface.c		\
	tny-platform-factory-iface.c

libtinymailui_1_0_la_LIBADD = 		\
	$(LIBTINYMAILUI_LIBS)		\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la
libtinymailui_1_0_la_LDFLAGS = -export-dynamic -version-info $(LT_VERSION_INFO)
libtinymailuiincludedir = $(includedir)/libtinymailui-$(API_VERSION)
libtinymailuiinclude_HEADERS = $(libtinymailui_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymailui-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
