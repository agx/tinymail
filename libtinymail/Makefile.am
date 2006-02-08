INCLUDES = $(LIBTINYMAIL_CFLAGS) -I.

lib_LTLIBRARIES = libtinymail-1.0.la

libtinymail_1_0_headers = 		\
	tny-shared.h			\
	tny-msg-folder-iface.h		\
	tny-msg-attachment-iface.h   	\
	tny-msg-body-iface.h         	\
	tny-msg-header-iface.h		\
	tny-msg-iface.h			\
	tny-account-iface.h		\
	tny-msg-header-proxy.h

#	tny-msg-folder-proxy.h	

libtinymail_1_0_la_SOURCES = 		\
	$(libtinymail_1_0_headers)	\
	tny-msg-iface.c			\
	tny-msg-header-iface.c		\
	tny-msg-body-iface.c         	\
	tny-msg-attachment-iface.c   	\
	tny-msg-folder-iface.c		\
	tny-account-iface.c		\
	tny-msg-header-proxy.c	


#	tny-msg-folder-proxy.c	
libtinymail_1_0_la_LIBADD = $(LIBTINYMAIL_LIBS)

libtinymail_1_0_includedir = $(includedir)/libtinymail-$(API_VERSION)
libtinymail_1_0_include_HEADERS = $(libtinymail_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
