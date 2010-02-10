INCLUDES = -I. -I$(top_srcdir) $(EXTRA_CFLAGS) \
	$(LIBTINYMAIL_MOBLIN_CFLAGS) \
	-I$(top_srcdir)/libtinymail \
	-DDBUS_API_SUBJECT_TO_CHANGE

lib_LTLIBRARIES = libtinymail-moblin-1.0.la

libtinymail_moblin_1_0_headers = 	\
	tny-moblin-platform-factory.h 	\
	tny-moblin-account-store.h	\
	tny-moblin-device.h

libtinymail_moblin_1_0_la_SOURCES =  		\
	$(libtinymail_moblin_1_0_headers)	\
	tny-moblin-platform-factory.c 		\
	tny-moblin-account-store.c 		\
	tny-moblin-device.c

libtinymail_moblin_1_0_la_LIBADD = \
	$(LIBTINYMAIL_MOBLIN_LIBS) \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la

libtinymail_moblin_1_0_la_LDFLAGS = -export-dynamic \
	-version-info $(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)

libtinymailmoblinincludedir = $(includedir)/libtinymail-moblin-$(API_VERSION)
libtinymailmoblininclude_HEADERS = $(libtinymail_moblin_1_0_headers)

libtinymail-moblin-$(API_VERSION).pc: libtinymail-moblin.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-moblin-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)