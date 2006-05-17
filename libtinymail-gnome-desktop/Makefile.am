INCLUDES = 					 \
	$(LIBTINYMAIL_GNOME_DESKTOP_CFLAGS)	 \
	-I$(top_srcdir)/libtinymail		 \
	-I$(top_srcdir)/libtinymailui		 \
	-I$(top_srcdir)/libtinymailui-gtk	 \
	-I$(top_srcdir)/libtinymail-camel	 \
	-I$(top_srcdir)/libtinymail-gnomevfs 

if BUILD_MOZEMBED
INCLUDES += -I$(top_srcdir)/libtinymailui-mozembed -DMOZEMBED
endif

if BUILD_GNOME
INCLUDES += -DGNOME
endif

lib_LTLIBRARIES = libtinymail-gnome-desktop-1.0.la

libtinymail_gnome_desktop_1_0_headers = 	 \
	tny-account-store.h			 \
	tny-password-dialog.h			 \
	tny-device.h				 \
	tny-platform-factory.h

libtinymail_gnome_desktop_1_0_la_SOURCES = 	 \
	$(libtinymail_gnome_desktop_1_0_headers) \
	tny-account-store.c			 \
	tny-device-priv.h			 \
	tny-device.c				 \
	tny-password-dialog.c			 \
	tny-platform-factory.c

libtinymail_gnome_desktop_1_0_la_LIBADD = 	\
	$(LIBTINYMAIL_GNOME_DESKTOP_LIBS)	\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la     \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la

if BUILD_MOZEMBED
libtinymail_gnome_desktop_1_0_la_LIBADD += $(top_builddir)/libtinymailui-mozembed/libtinymailui-mozembed-$(API_VERSION).la
endif

libtinymailgnomedesktopincludedir = $(includedir)/libtinymail-gnome-desktop-$(API_VERSION)
libtinymailgnomedesktopinclude_HEADERS = $(libtinymail_gnome_desktop_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymail-gnome-desktop-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
