INCLUDES =  -I. -I$(top_srcdir) -I$(top_srcdir)/tests/shared \
	$(LIBTINYMAIL_TEST_CFLAGS) \
	$(TINYMAIL_CFLAGS) \
        $(LIBTINYMAIL_CAMEL_CFLAGS) \
	-I$(top_srcdir)/libtinymail \
	-I$(top_srcdir)/libtinymailui \
	-I$(top_srcdir)/libtinymailui-gtk \
	-I$(top_srcdir)/libtinymail-camel \
	-I$(top_srcdir)/libtinymail-gnomevfs \
	-I$(top_srcdir)/libtinymail-camel/camel-lite

if BUILD_GNOME
INCLUDES += -DGNOME
endif

lib_LTLIBRARIES = libtinymail-test-suite.la

bin_PROGRAMS = tny-simple-test-runner tny-test-anything

if BUILD_APITESTS
lib_LTLIBRARIES += libtinymail-api-test-suite.la
bin_PROGRAMS += tny-simple-api-test-runner
endif

libtinymail_test_suite_la_SOURCES = \
	tny-folder-store-query-test.c \
	tny-folder-store-query-test.h \
	tny-test-object.h \
	tny-test-object.c \
	tny-header-test.h \
	tny-header-test.c \
	tny-stream-test.h \
	tny-stream-test.c \
	tny-list-test.h \
	tny-list-test.c \
	tny-test-stream.c \
	tny-test-stream.h \
	tny-test-suite.c \
	tny-mime-part-test.c \
	tny-mime-part-test.h \
	tny-platform-factory-test.c \
	tny-platform-factory-test.h \
	tny-account-store-test.c \
	tny-account-store-test.h \
	tny-account-test.c \
	tny-account-test.h \
	tny-device-test.c \
	tny-device-test.h \
	tny-iterator-test.c \
	tny-iterator-test.h \
	tny-folder-test.c \
	tny-folder-test.h \
	tny-msg-test.c \
	tny-msg-test.h \
	tny-test-suite.h

libtinymail_test_suite_la_LDFLAGS = $(LIBTINYMAIL_TEST_LIBS) \
	-export-dynamic -version-info $(LT_VERSION_INFO) \
	$(LIBTOOL_EXPORT_OPTIONS)

tny_simple_test_runner_SOURCES = tny-simple-test-runner.c
tny_test_anything_SOURCES = tny-test-anything.c

libtinymail_test_suite_la_CFLAGS = -fPIC

libtinymail_test_suite_la_LIBADD = \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la \
	$(top_builddir)/tests/shared/libtestsshared.a


tny_simple_test_runner_LDFLAGS = $(LIBTINYMAIL_TEST_LIBS)

tny_test_anything_LDFLAGS = $(LIBTINYMAIL_TEST_LIBS) \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la \
	$(top_builddir)/tests/shared/libtestsshared.a

tny_simple_test_runner_LDADD = \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la \
	$(top_builddir)/tests/shared/libtestsshared.a \
	libtinymail-test-suite.la

tny_test_anything_LDADD = \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la \
	$(top_builddir)/tests/shared/libtestsshared.a \
	libtinymail-test-suite.la

# API test suite

libtinymail_api_test_suite_la_SOURCES = \
	tny-api-test-suite.c \
	tny-list-test.c \
	tny-device-test.c \
	tny-account-test.c \
	tny-folder-test.c \
	tny-mime-part-test.c \
	tny-test-stream.c \
	tny-header-test.c \
	tny-folder-store-query-test.c \
	tny-account-store-test.c \
	tny-platform-factory-test.c \
	tny-test-object.c

libtinymail_api_test_suite_la_CFLAGS = -fPIC \
	$(LIBTINYMAIL_API_TEST_CFLAGS)

libtinymail_api_test_suite_la_LDFLAGS = $(LIBTINYMAIL_API_TEST_LIBS) \
	-export-dynamic -version-info $(LT_VERSION_INFO) \
	$(LIBTOOL_EXPORT_OPTIONS)

# API test runner

tny_simple_api_test_runner_SOURCES = tny-simple-api-test-runner.c

tny_simple_api_test_runner_CFLAGS = $(LIBTINYMAIL_API_TEST_CFLAGS)

tny_simple_api_test_runner_LDADD = \
	libtinymail-api-test-suite.la \
	$(top_builddir)/tests/shared/libtestsshared.a
