INCLUDES = -I. -I$(top_srcdir) -I$(top_srcdir)/tests/shared \
	$(TINYMAIL_CFLAGS) 				\
	-I$(top_srcdir)/libtinymail			\
	-I$(top_srcdir)/libtinymailui			\
	-I$(top_srcdir)/libtinymailui-gtk		\
	-I$(top_srcdir)/libtinymail-camel		\
	-DASYNC_HEADERS

if BUILD_GNOME
INCLUDES += -DGNOME
endif

if BUILD_MOZEMBED
INCLUDES += -DMOZEMBED
endif

bin_PROGRAMS = folder-lister

folder_lister_SOURCES = folder-lister.c folder-lister.h 

folder_lister_LDADD = 							        \
	$(TINYMAIL_LIBS)						        \
	$(LIBTINYMAIL_GNOME_DESKTOP_LIBS)					\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 	        \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la	        \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la	\
	$(top_builddir)/tests/shared/libtestsshared.a

