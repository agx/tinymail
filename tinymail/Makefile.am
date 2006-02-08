INCLUDES = 								\
	$(TINYMAIL_CFLAGS) 						\
	-I$(top_builddir)/libtinymail					\
	-I$(top_builddir)/libtinymailui					\
	-I$(top_builddir)/libtinymail-camel

bin_PROGRAMS = tinymail

tinymail_SOURCES = 							\
	tny-main.c							\
	tny-summary-window.c						\
	tny-msg-window.h						\
	tny-msg-window.c						\
	tny-summary-window.h						\
	tny-account-store.c						\
	tny-account-store.h						\
	tny-password-dialog.h						\
	tny-password-dialog.c

tinymail_LDADD = 							\
	$(TINYMAIL_LIBS)						\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 	\
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la	\
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la
