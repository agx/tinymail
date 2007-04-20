INCLUDES = $(LIBTINYMAIL_QUEUES_CFLAGS) $(LIBTINYMAIL_CFLAGS) -I. -I$(top_srcdir) \
	-I$(top_srcdir)/libtinymail

lib_LTLIBRARIES = libtinymail-queues-1.0.la

libtinymail_queues_1_0_headers = \
	tny-get-msg-queue-priv.h \
	tny-get-msg-queue.h

libtinymail_queues_1_0_la_SOURCES = \
	$(libtinymail_queues_1_0_headers) \
	tny-get-msg-queue.c

libtinymail_queues_1_0_la_LIBADD = $(LIBTINYMAIL_LIBS) $(LIBTINYMAIL_QUEUES_LIBS)
libtinymail_queues_1_0_la_LDFLAGS = -export-dynamic -version-info $(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)
libtinymail_queues_1_0_includedir = $(includedir)/libtinymail-$(API_VERSION)
libtinymail_queues_1_0_include_HEADERS = $(libtinymail_queues_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-queues-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
