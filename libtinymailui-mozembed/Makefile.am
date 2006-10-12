INCLUDES = -I. -I$(top_srcdir)		   \
	$(LIBTINYMAILUI_MOZEMBED_CFLAGS)   \
	-I$(top_srcdir)/libtinymail	   \
	-I$(top_srcdir)/libtinymailui	   \
	-I$(top_srcdir)/libtinymailui-gtk  \
	-I$(top_srcdir)/libtinymail-camel

if BUILD_GNOME
INCLUDES +=  -I$(top_srcdir)/libtinymail-gnomevfs -DGNOME
endif

lib_LTLIBRARIES = libtinymailui-mozembed-1.0.la

libtinymailui_mozembed_1_0_headers = 		 \
	tny-moz-embed-msg-view.h		 \
	tny-moz-embed-html-mime-part-view.h	\
	tny-moz-embed-stream.h

libtinymailui_mozembed_1_0_la_SOURCES = 	 \
	$(libtinymailui_mozembed_1_0_headers)	 \
	tny-moz-embed-html-mime-part-view.c	 \
	tny-moz-embed-msg-view.c		 \
	tny-moz-embed-stream.c

libtinymailui_mozembed_1_0_la_LIBADD = 	\
	$(LIBTINYMAILUI_MOZEMBED_LIBS)	\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la   \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la

libtinymailui_mozembed_1_0_la_LDFLAGS = -export-dynamic -version-info $(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)

if BUILD_GNOME
libtinymailui_mozembed_1_0_la_LIBADD +=  $(top_builddir)/libtinymail-gnomevfs/libtinymail-gnomevfs-$(API_VERSION).la
endif

libtinymailuimozembedincludedir = $(includedir)/libtinymailui-mozembed-$(API_VERSION)
libtinymailuimozembedinclude_HEADERS = $(libtinymailui_mozembed_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymailui-mozembed-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
