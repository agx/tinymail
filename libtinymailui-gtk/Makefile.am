INCLUDES = -I. -I$(top_srcdir)		\
	$(LIBTINYMAILUI_GTK_CFLAGS)	\
	-I$(top_srcdir)/libtinymail	\
	-I$(top_srcdir)/libtinymailui 

if BUILD_GNOME
INCLUDES += -I$(top_srcdir)/libtinymail-gnomevfs -DGNOME
endif

lib_LTLIBRARIES = libtinymailui-gtk-1.0.la

libtinymailui_gtk_1_0_headers = 	\
	tny-gtk-save-strategy.h		\
	tny-gtk-msg-view.h		\
	tny-gtk-msg-window.h		\
	tny-header-list-model.h		\
	tny-account-tree-model.h	\
	tny-attach-list-model.h		\
	tny-text-buffer-stream.h	\
	tny-gtk-header-view.h

libtinymailui_gtk_1_0_la_SOURCES =		\
	$(libtinymailui_gtk_1_0_headers)	\
	tny-gtk-save-strategy.c			\
	tny-gtk-msg-view.c			\
	tny-gtk-msg-window.c			\
	tny-header-list-model-priv.h       	\
	tny-header-list-iterator-priv.h    	\
	tny-account-tree-model-iterator-priv.h	\
	tny-account-tree-model-iterator.c	\
	tny-account-tree-model-priv.h		\
	tny-header-list-model.c	       		\
	tny-header-list-iterator.c		\
	tny-attach-list-model-iterator-priv.h	\
	tny-attach-list-model-iterator.c	\
	tny-account-tree-model.c		\
	tny-attach-list-model-priv.h		\
	tny-attach-list-model.c			\
	tny-text-buffer-stream.c		\
	tny-gtk-header-view.c

libtinymailui_gtk_1_0_la_LIBADD = 	\
	$(LIBTINYMAILUI_GTK_LIBS)	\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la                   \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la

libtinymailui_gtk_1_0_la_LDFLAGS = -export-dynamic -version-info $(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)

if BUILD_GNOME
libtinymailui_gtk_1_0_la_LIBADD += $(top_builddir)/libtinymail-gnomevfs/libtinymail-gnomevfs-$(API_VERSION).la
endif

libtinymailuigtkincludedir = $(includedir)/libtinymailui-gtk-$(API_VERSION)
libtinymailuigtkinclude_HEADERS = $(libtinymailui_gtk_1_0_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtinymailui-gtk-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

DISTCLEANFILES = $(pkgconfig_DATA)
