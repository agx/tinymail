INCLUDES = -I. -I$(top_srcdir) -I$(top_srcdir)/tests/shared \
	$(TINYMAIL_CFLAGS) 				\
	-I$(top_srcdir)/libtinymail			\
	-I$(top_srcdir)/libtinymailui			\
	-I$(top_srcdir)/libtinymailui-gtk		\
	-I$(top_srcdir)/libtinymail-camel		\
	-I$(top_srcdir)/$(PLATFORMDIR)			\
	-DASYNC_HEADERS

if BUILD_GNOME
INCLUDES += -DGNOME
endif

if BUILD_MOZEMBED
INCLUDES += -DMOZEMBED
endif

bin_PROGRAMS = memory-test

memory_test_SOURCES = memory-test.c memory-test.h 

memory_test_LDADD = 							        \
	$(TINYMAIL_LIBS)						        \
	$(LIBTINYMAIL_GNOME_DESKTOP_LIBS)					\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 	        \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la	        \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la	\
	$(top_builddir)/$(PLATFORMDIR)/$(PLATFORMDIR)-$(API_VERSION).la		\
	$(top_builddir)/tests/shared/libtestsshared.a

