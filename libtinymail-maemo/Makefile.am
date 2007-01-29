INCLUDES = -I. -I$(top_srcdir) \
	$(LIBTINYMAIL_MAEMO_CFLAGS) \
	$(LIBTINYMAIL_CAMEL_CFLAGS) \
	-I$(top_srcdir)/libtinymail \
	-I$(top_srcdir)/libtinymailui \
	-I$(top_srcdir)/libtinymailui-gtk \
	-I$(top_srcdir)/libtinymail-camel \
	-I$(top_srcdir)/libtinymail-camel/camel-lite \
	-I$(top_srcdir)/libtinymail-gnomevfs \
	-DDBUS_API_SUBJECT_TO_CHANGE


lib_LTLIBRARIES = libtinymail-maemo-1.0.la

libtinymail_maemo_1_0_headers = \
	tny-maemo-account-store.h \
	tny-maemo-password-dialog.h \
	tny-maemo-device.h \
	tny-maemo-platform-factory.h

libtinymail_maemo_1_0_la_SOURCES =  \
	$(libtinymail_maemo_1_0_headers) \
	tny-maemo-account-store.c \
	tny-maemo-device-priv.h	\
	tny-maemo-device.c \
	tny-maemo-password-dialog.c \
	tny-maemo-platform-factory.c

libtinymail_maemo_1_0_la_LIBADD = \
	$(LIBTINYMAIL_MAEMO_LIBS) \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la

libtinymail_maemo_1_0_la_LDFLAGS = -export-dynamic \
	-version-info $(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)

libtinymailmaemoincludedir = $(includedir)/libtinymail-maemo-$(API_VERSION)
libtinymailmaemoinclude_HEADERS = $(libtinymail_maemo_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-maemo-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
