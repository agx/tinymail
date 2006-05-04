INCLUDES = 								\
	$(TINYMAIL_CFLAGS) 						\
	-I$(top_builddir)/libtinymail					\
	-I$(top_builddir)/libtinymailui					\
	-I$(top_builddir)/libtinymailui-gtk				\
	-I$(top_builddir)/libtinymail-camel				
	-DASYNC_HEADERS

bin_PROGRAMS = tinymail

tinymail_SOURCES = 							\
	tny-main.c							\
	tny-account-store.c						\
	tny-summary-window.c						\
	tny-summary-window.h						\
	tny-account-store.h						\
	tny-password-dialog.h						\
	tny-password-dialog.c

tinymail_LDADD = 							        \
	$(TINYMAIL_LIBS)						        \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 	        \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la	        \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la
