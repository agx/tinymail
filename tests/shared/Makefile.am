INCLUDES = -I. -I$(top_srcdir) \
	$(TINYMAIL_CFLAGS)                              \
	$(LIBTINYMAIL_CAMEL_CFLAGS)			\
	-I$(top_srcdir)/libtinymail                     \
	-I$(top_srcdir)/libtinymailui                   \
	-I$(top_srcdir)/libtinymailui-gtk               \
	-I$(top_srcdir)/libtinymail-camel		\
	-DASYNC_HEADERS

if BUILD_GNOME
INCLUDES += -DGNOME
endif

if BUILD_MOZEMBED
INCLUDES += -DMOZEMBED
endif

noinst_LIBRARIES = libtestsshared.a
libtestsshared_a_SOURCES = account-store.c account-store.h \
	platfact.c platfact.h device.c device.h device-priv.h

#libtestsshared_a_LDFLAGS = $(LIBTINYMAIL_LIBS) \
#	$(LIBTINYMAIL_GNOME_DESKTOP_LIBS)                                       \
#	$(top_builddir)/libtinymail/libtinymail-$(API_VERSION).la               \
#	$(top_builddir)/libtinymailui/libtinymailui-$(API_VERSION).la           \
#	$(top_builddir)/libtinymailui-gtk/libtinymailui-gtk-$(API_VERSION).la   \
#	$(top_builddir)/libtinymail-camel/libtinymail-camel-$(API_VERSION).la
