INCLUDES = 								\
	$(TINYMAIL_CFLAGS) 						\
	-I$(top_builddir)/libtinymail					\
	-I$(top_builddir)/libtinymailui					\
	-I$(top_builddir)/libtinymailui-gtk				\
	-I$(top_builddir)/libtinymail-camel				\
	-I$(top_builddir)/libtinymail-gnome-desktop			\
	-DASYNC_HEADERS -DGTKMOZEMBED

bin_PROGRAMS = tinymail

tinymail_SOURCES = 							\
	tny-main.c							\
	tny-summary-window.c						\
	tny-summary-window.h

tinymail_LDADD = 							        \
	$(TINYMAIL_LIBS)						        \
	$(LIBTINYMAIL_GNOME_DESKTOP_LIBS)					\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 	        \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la	        \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la	\
	$(top_builddir)/libtinymail-gnome-desktop/libtinymail-gnome-desktop-$(API_VERSION).la
