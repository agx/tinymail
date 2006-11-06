SUBDIRS = camel-lite .

INCLUDES = $(LIBTINYMAIL_CAMEL_CFLAGS)  	\
	-I. -I$(top_srcdir)			\
	-I$(top_srcdir)/libtinymail		\
	-I$(top_srcdir)/libtinymail-camel/camel-lite 

lib_LTLIBRARIES = libtinymail-camel-1.0.la 

libtinymail_camel_1_0_headers =	\
	tny-camel-mime-part.h \
	tny-camel-msg-remove-strategy.h \
	tny-camel-header.h \
	tny-camel-msg.h \
	tny-camel-folder.h \
	tny-camel-imap-folder.h \
	tny-camel-nntp-folder.h \
	tny-camel-pop-folder.h \
	tny-camel-account.h \
	tny-camel-store-account.h \
	tny-camel-imap-store-account.h \
	tny-camel-nntp-store-account.h \
	tny-camel-pop-store-account.h \
	tny-camel-transport-account.h \
	tny-camel-stream.h \
	tny-stream-camel.h \
	tny-camel-shared.h \
	tny-session-camel.h

libtinymail_camel_priv_headers = \
	tny-session-camel-priv.h \
	tny-camel-msg-remove-strategy.c \
	tny-camel-common.c \
	tny-camel-common-priv.h \
	tny-camel-account-priv.h \
	tny-camel-store-account-priv.h \
	tny-camel-transport-account-priv.h \
	tny-camel-msg-priv.h \
	tny-camel-header-priv.h \
	tny-camel-folder-priv.h 

libtinymail_camel_1_0_la_SOURCES = \
	$(libtinymail_camel_priv_headers) \
	$(libtinymail_camel_1_0_headers) \
	tny-camel-msg.c \
	tny-camel-header.c \
	tny-camel-mime-part-priv.h \
	tny-camel-mime-part.c \
	tny-camel-folder.c \
	tny-camel-imap-folder.c \
	tny-camel-pop-folder.c \
	tny-camel-nntp-folder.c \
	tny-camel-account.c \
	tny-camel-store-account.c \
	tny-camel-imap-store-account.c \
	tny-camel-nntp-store-account.c \
	tny-camel-pop-store-account.c \
	tny-camel-transport-account.c \
	tny-camel-stream.c \
	tny-stream-camel.c \
	tny-session-camel.c			

libtinymail_camel_1_0_la_LIBADD = \
	$(LIBTINYMAIL_CAMEL_LIBS) \
	$(top_builddir)/libtinymail-camel/camel-lite/camel/libcamel-lite-1.2.la \
	$(top_builddir)/libtinymail-camel/camel-lite/camel/libcamel-lite-provider-1.2.la \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 

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
