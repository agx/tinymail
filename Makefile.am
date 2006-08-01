SUBDIRS = m4 libtinymail libtinymailui

if BUILD_GNOME
SUBDIRS += libtinymail-gnomevfs
endif

SUBDIRS += libtinymailui-gtk

if BUILD_MOZEMBED
SUBDIRS += libtinymailui-mozembed
endif

SUBDIRS += $(PLATFORMDIR) libtinymail-camel tinymail docs

if BUILD_UNITTESTS
SUBDIRS += libtinymail-test 
endif

SUBDIRS += bindings po

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc
