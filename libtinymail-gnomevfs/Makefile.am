INCLUDES = $(LIBTINYMAIL_GNOMEVFS_CFLAGS) -I.	\
	-I$(top_builddir)/libtinymail

lib_LTLIBRARIES = libtinymail-gnomevfs-1.0.la 

libtinymail_gnomevfs_1_0_headers =		\
	tny-stream-gnomevfs.h

libtinymail_gnomevfs_1_0_la_SOURCES = 		\
	$(libtinymail_gnomevfs_1_0_headers)	\
	tny-stream-gnomevfs.c

libtinymail_gnomevfs_1_0_la_LIBADD =		\
	$(LIBTINYMAIL_GNOMEVFS_LIBS)		\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la

libtinymail_gnomevfs_1_0includedir = $(includedir)/libtinymail-gnomevfs-$(API_VERSION)
libtinymail_gnomevfs_1_0include_HEADERS = $(libtinymail_gnomevfs_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-gnomevfs-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
