LTVERSION = "0:0:0"

INCLUDES = 									\
	$(LIBTINYMAIL_TEST_CFLAGS)						\
	-I$(top_builddir)/libtinymail						\
	-I$(top_builddir)/libtinymailui						\
	-I$(top_builddir)/libtinymailui-gtk					\
	-I$(top_builddir)/libtinymail-camel

lib_LTLIBRARIES = libtinymail-test-suite.la

bin_PROGRAMS = tny-simple-test-runner

libtinymail_test_suite_la_SOURCES = 						\
	tny-msg-header-iface-test.h						\
	tny-msg-header-iface-test.c						\
	tny-stream-iface-test.h							\
	tny-stream-iface-test.c							\
	tny-test-stream.c							\
	tny-test-stream.h							\
	tny-test-suite.c							\
	tny-test-suite.h

libtinymail_test_suite_la_LDFLAGS = $(LIBTINYMAIL_TEST_LIBS)

tny_simple_test_runner_SOURCES = tny-simple-test-runner.c


libtinymail_test_suite_la_LIBADD = 						\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 	        \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la	        \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la

tny_simple_test_runner_LDADD = 							\
	$(LIBTINYMAIL_TEST_LIBS)						\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la               \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la           \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la	\
	libtinymail-test-suite.la

