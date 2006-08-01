INCLUDES =  -I. -I$(top_srcdir)			\
	$(LIBTINYMAIL_TEST_CFLAGS)		\
	-I$(top_srcdir)/libtinymail		\
	-I$(top_srcdir)/libtinymailui		\
	-I$(top_srcdir)/libtinymailui-gtk	\
	-I$(top_srcdir)/libtinymail-camel	\
	-I$(top_srcdir)/$(PLATFORMDIR) 

lib_LTLIBRARIES = libtinymail-test-suite.la

bin_PROGRAMS = tny-simple-test-runner tny-test-anything

libtinymail_test_suite_la_SOURCES = 		\
	tny-test-object.h			\
	tny-test-object.c			\
	tny-msg-header-iface-test.h		\
	tny-msg-header-iface-test.c		\
	tny-stream-iface-test.h			\
	tny-stream-iface-test.c			\
	tny-list-iface-test.h			\
	tny-list-iface-test.c			\
	tny-test-stream.c			\
	tny-test-stream.h			\
	tny-test-suite.c			\
	tny-msg-mime-part-iface-test.c		\
	tny-msg-mime-part-iface-test.h		\
	tny-account-iface-test.c		\
	tny-account-iface-test.h		\
	tny-device-iface-test.c			\
	tny-device-iface-test.h			\
	tny-iterator-iface-test.c		\
	tny-iterator-iface-test.h		\
	tny-msg-folder-iface-test.c		\
	tny-msg-folder-iface-test.h		\
	tny-msg-iface-test.c			\
	tny-msg-iface-test.h			\
	tny-test-suite.h

libtinymail_test_suite_la_LDFLAGS = $(LIBTINYMAIL_TEST_LIBS) \
	-export-dynamic -version-info $(LT_VERSION_INFO)     \
	$(LIBTOOL_EXPORT_OPTIONS)

tny_simple_test_runner_SOURCES = tny-simple-test-runner.c
tny_test_anything_SOURCES = tny-test-anything.c

libtinymail_test_suite_la_LIBADD = 						\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 	        \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la	        \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la	\
	$(top_builddir)/$(PLATFORMDIR)/$(PLATFORMDIR)-$(API_VERSION).la

tny_simple_test_runner_LDFLAGS = $(LIBTINYMAIL_TEST_LIBS)

tny_test_anything_LDFLAGS = $(LIBTINYMAIL_TEST_LIBS)

tny_simple_test_runner_LDADD = 							\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la               \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la           \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la	\
	libtinymail-test-suite.la

tny_test_anything_LDADD =                                                       \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la               \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la           \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la   \
	libtinymail-test-suite.la

