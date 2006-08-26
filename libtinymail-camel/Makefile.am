INCLUDES = $(LIBTINYMAIL_CAMEL_CFLAGS)  	\
	-I. -I$(top_srcdir)			\
	-I$(top_srcdir)/libtinymail		\
	-I$(top_srcdir)/$(PLATFORMDIR)

lib_LTLIBRARIES = libtinymail-camel-1.0.la 

libtinymail_camel_1_0_headers =			\
	tny-mime-part.h				\
	tny-header.h				\
	tny-msg.h				\
	tny-folder.h				\
	tny-account.h				\
	tny-store-account.h			\
	tny-transport-account.h			\
	tny-camel-stream.h			\
	tny-stream-camel.h			\
	tny-camel-shared.h			\
	tny-session-camel.h

libtinymail_camel_1_0_la_SOURCES = 		\
	tny-camel-common.c			\
	tny-camel-common-priv.h			\
	tny-account-priv.h			\
	tny-store-account-priv.h		\
	tny-transport-account-priv.h		\
	tny-msg-priv.h				\
	tny-header-priv.h			\
	tny-folder-priv.h			\
	$(libtinymail_camel_1_0_headers)	\
	tny-msg.c				\
	tny-header.c 				\
	tny-mime-part-priv.h			\
	tny-mime-part.c				\
	tny-folder.c				\
	tny-account.c				\
	tny-store-account.c			\
	tny-transport-account.c			\
	tny-camel-stream.c			\
	tny-stream-camel.c			\
	tny-session-camel.c			

libtinymail_camel_1_0_la_LIBADD =		\
	$(LIBTINYMAIL_CAMEL_LIBS)		\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/$(PLATFORMDIR)/$(PLATFORMDIR)-$(API_VERSION).la

libtinymail_camel_1_0_la_LDFLAGS = -export-dynamic -version-info \
	$(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)
libtinymail_camel_1_0includedir = $(includedir)/libtinymail-camel-$(API_VERSION)
libtinymail_camel_1_0include_HEADERS = $(libtinymail_camel_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-camel-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
