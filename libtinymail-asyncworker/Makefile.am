INCLUDES = $(LIBTINYMAIL_ASYNCWORKER_CFLAGS) $(LIBTINYMAIL_CFLAGS) \
	-I. -I$(top_srcdir) -I$(top_srcdir)/libtinymail \
	-I$(top_srcdir)/libtinymail-queues

lib_LTLIBRARIES = libtinymail-asyncworker-1.0.la

libtinymail_asyncworker_1_0_headers = \
	tny-async-worker-queue.h \
	tny-async-worker-queue-task.h

libtinymail_asyncworker_1_0_la_SOURCES = \
	tny-async-worker-queue-priv.h \
	$(libtinymail_asyncworker_1_0_headers) \
	tny-async-worker-queue.c \
	tny-async-worker-queue-task-priv.h \
	tny-async-worker-queue-task.c

libtinymail_asyncworker_1_0_la_LIBADD = $(LIBTINYMAIL_LIBS) 
	$(LIBTINYMAIL_QUEUES_LIBS) $(LIBTINYMAIL_ASYNCWORKER_LIBS) \
	$(top_builddir)/libtinymail-queues/libtinymail-queues-$(API_VERSION).la
libtinymail_asyncworker_1_0_la_LDFLAGS = -export-dynamic -version-info $(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-asyncworker-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
