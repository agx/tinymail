INCLUDES = 					 \
	$(LIBTINYMAIL_GNOME_DESKTOP_CFLAGS)	 \
	-I$(top_builddir)/libtinymail		 \
	-I$(top_builddir)/libtinymailui		 \
	-I$(top_builddir)/libtinymailui-gtk

lib_LTLIBRARIES = libtinymail-gnome-desktop-1.0.la

libtinymail_gnome_desktop_1_0_headers = 	 \
	tny-account-store.h			 \
	tny-device.h

libtinymail_gnome_desktop_1_0_la_SOURCES = 	 \
	$(libtinymail_gnome_desktop_1_0_headers) \
	tny-account-store.c			 \
	tny-device.c

libtinymail_gnome_desktop_1_0_la_LIBADD = 	\
	$(LIBTINYMAIL_GNOME_DESKTOP_LIBS)	\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la   \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la

libtinymailgnomedesktopincludedir = $(includedir)/libtinymail-gnome-desktop-$(API_VERSION)
libtinymailgnomedesktopinclude_HEADERS = $(libtinymail_gnome_desktop_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-gnome-desktop-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
