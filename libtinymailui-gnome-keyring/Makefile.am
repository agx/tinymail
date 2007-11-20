AM_CFLAGS = -fPIC -DPIC

INCLUDES = -I. -I$(top_srcdir) \
	$(TINYMAIL_CFLAGS) \
	$(LIBTINYMAILUI_GTK_CFLAGS) \
	-I$(top_srcdir)/libtinymail \
	-I$(top_srcdir)/libtinymailui \
	-I$(top_srcdir)/libtinymailui-gtk 

if BUILD_GNOME
INCLUDES += -DGNOME
endif

if BUILD_MOZEMBED
INCLUDES += -DMOZEMBED
endif

noinst_LIBRARIES = libtinymailui-gnome-keyring.a

libtinymailui_gnome_keyring_a_SOURCES = \
	tny-gnome-keyring-password-getter.c \
	tny-gnome-keyring-password-getter.h
