INCLUDES = $(LIBTINYMAIL_CAMEL_CFLAGS) -I.	\
	-I$(top_builddir)/libtinymail		\
	-I$(top_builddir)/$(PLATFORMDIR)

# This makes tinymail faster, but wastes more memory
#	-DPERFORMANCE_MODE

lib_LTLIBRARIES = libtinymail-camel-1.0.la 

libtinymail_camel_1_0_headers =			\
	tny-msg-mime-part.h			\
	tny-msg-header.h			\
	tny-msg.h				\
	tny-msg-folder.h			\
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
	tny-msg-header-priv.h			\
	tny-msg-folder-priv.h			\
	$(libtinymail_camel_1_0_headers)	\
	tny-msg.c				\
	tny-msg-header.c 			\
	tny-msg-mime-part-priv.h		\
	tny-msg-mime-part.c			\
	tny-msg-folder.c			\
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

libtinymail_camel_1_0includedir = $(includedir)/libtinymail-camel-$(API_VERSION)
libtinymail_camel_1_0include_HEADERS = $(libtinymail_camel_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-camel-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
