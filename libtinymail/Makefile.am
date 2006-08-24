INCLUDES = $(LIBTINYMAIL_CFLAGS) -I. -I$(top_srcdir)

lib_LTLIBRARIES = libtinymail-1.0.la

libtinymail_1_0_headers = 		\
	tny-shared.h			\
	tny-list-iface.h		\
	tny-iterator-iface.h		\
	tny-folder-iface.h		\
	tny-mime-part-iface.h   	\
	tny-header-iface.h		\
	tny-msg-iface.h			\
	tny-device-iface.h		\
	tny-account-iface.h		\
	tny-store-account-iface.h	\
	tny-transport-account-iface.h	\
	tny-account-store-iface.h	\
	tny-stream-iface.h		\
	tny-fs-stream.h			\
	tny-list.h			\
	tny-folder-store-iface.h	\
	tny-folder-store-query.h

libtinymail_1_0_la_SOURCES = 		\
	$(libtinymail_1_0_headers)	\
	tny-list-iface.c		\
	tny-iterator-iface.c		\
	tny-msg-iface.c			\
	tny-header-iface.c		\
	tny-mime-part-iface.c   	\
	tny-folder-iface.c		\
	tny-device-iface.c		\
	tny-account-iface.c		\
	tny-store-account-iface.c	\
	tny-transport-account-iface.c	\
	tny-account-store-iface.c	\
	tny-stream-iface.c		\
	tny-fs-stream.c			\
	tny-list-priv.h			\
	tny-list.c			\
	tny-list-iterator-priv.h	\
	tny-list-iterator.c		\
	tny-folder-store-iface.c	\
	tny-folder-store-query.c

libtinymail_1_0_la_LIBADD = $(LIBTINYMAIL_LIBS)
libtinymail_1_0_la_LDFLAGS = -export-dynamic -version-info $(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)
libtinymail_1_0_includedir = $(includedir)/libtinymail-$(API_VERSION)
libtinymail_1_0_include_HEADERS = $(libtinymail_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
