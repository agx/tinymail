LTVERSION = "0:0:0"

INCLUDES = 									\
	$(LIBTINYMAIL_TEST_CFLAGS)						\
	-I$(top_builddir)/libtinymail						\
	-I$(top_builddir)/libtinymailui						\
	-I$(top_builddir)/libtinymailui-gtk					\
	-I$(top_builddir)/libtinymail-camel

lib_LTLIBRARIES = libtinymail-test-suite.la libgunittinymail-test-view.la

libtinymail_test_suite_la_SOURCES = 						\
	tny-msg-header-iface-test.h						\
	tny-msg-header-iface-test.c						\
	tny-test-suite.c							\
	tny-test-suite.h

libgunittinymail_test_view_la_SOURCES = 					\
	tny-test-tui-view.c							\
	tny-test-tui-view.h							\
	tny-test-view.c								\
	tny-test-view.h

libtinymail_test_suite_la_LDFLAGS = $(LIBTINYMAIL_TEST_LIBS)

libgunittinymail_test_view_la_LDFLAGS = $(LIBTINYMAIL_TEST_LIBS)

libtinymail_test_suite_la_LIBADD = 						\
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la 	        \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la	        \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la

libgunittinymail_test_view_la_LIBADD =                                          \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la               \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la           \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la

