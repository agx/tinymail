INCLUDES =  \
	$(CHECK_CFLAGS) \
        $(LIBTINYMAIL_CFLAGS) \
        $(LIBTINYMAILUI_CFLAGS) \
        $(LIBTINYMAILUI_GTK_CFLAGS) \
        $(LIBTINYMAIL_CAMEL_CFLAGS) \
	-I$(top_srcdir)/libtinymail \
	-I$(top_srcdir)/libtinymailui \
	-I$(top_srcdir)/libtinymailui-gtk \
	-I$(top_srcdir)/libtinymail-camel \
	-I$(top_srcdir)/libtinymail-camel/camel-lite \
	-I$(top_srcdir)/tests/shared

AM_LDFLAGS = \
	@CHECK_LIBS@

LDADD = \
	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la \
	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la \
	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la \
	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la \
	$(top_builddir)/tests/shared/libtestsshared.a

# if BUILD_GNOME
# INCLUDES += -DGNOME
# endif

# lib_LTLIBRARIES = \
# 	libtinymail-test-suite.la \
# 	libtinymailui-test-suite.la


# # libtinymail test library

# libtinymail_test_suite_la_SOURCES = \
# 	check_libtinymail.h \
# 	check_libtinymail_main.c \
# 	tny-test-object.h \
# 	tny-test-object.c \
# 	tny-test-stream.h \
# 	tny-test-stream.c \
# 	tny-list-test.c \
# 	tny-folder-test.c \
# 	tny-folder-store-query-test.c \
# 	tny-account-store-test.c \
# 	tny-account-test.c \
# 	tny-device-test.c \
# 	tny-header-test.c \
# 	tny-mime-part-test.c \
# 	tny-msg-test.c \
# 	tny-platform-factory-test.c \
# 	tny-stream-test.c

# libtinymail_test_suite_la_CFLAGS = -fPIC

# libtinymail_test_suite_la_LDFLAGS = \
# 	-export-dynamic -version-info $(LT_VERSION_INFO) \
# 	$(LIBTOOL_EXPORT_OPTIONS)


# # libtinymailui test library

# libtinymailui_test_suite_la_SOURCES = \
# 	check_libtinymailui.h \
# 	check_libtinymailui_main.c \
# 	tny-platform-factory-test.c

# libtinymailui_test_suite_la_CFLAGS = -fPIC

# libtinymailui_test_suite_la_LDFLAGS = \
# 	-export-dynamic -version-info $(LT_VERSION_INFO) \
# 	$(LIBTOOL_EXPORT_OPTIONS)


# 'Check' unit tests

TESTS = \
	check_libtinymail \
	check_libtinymailui

noinst_PROGRAMS = \
	check_libtinymail \
	check_libtinymailui


# libtinymail tests

check_libtinymail_SOURCES = \
	check_libtinymail.h \
	check_libtinymail_main.c \
	tny-test-object.h \
	tny-test-object.c \
	tny-test-stream.h \
	tny-test-stream.c \
	tny-list-test.c \
	tny-folder-test.c \
	tny-folder-store-query-test.c \
	tny-account-store-test.c \
	tny-account-test.c \
	tny-device-test.c \
	tny-header-test.c \
	tny-mime-part-test.c \
	tny-msg-test.c \
	tny-platform-factory-test.c \
	tny-stream-test.c


# libtinymailui tests

check_libtinymailui_SOURCES = \
	check_libtinymailui.h \
	check_libtinymailui_main.c \
	tny-platform-factory-test.c
