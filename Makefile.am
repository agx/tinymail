if BUILD_UNITTESTS
UNITTESTDIR = libtinymail-test 
endif
if BUILD_MOZEMBED
MOZEMBEDDIR = libtinymailui-mozembed
endif
if BUILD_GNOME
TNYVFSDIR=libtinymail-gnomevfs
endif
SUBDIRS = m4 libtinymail libtinymailui $(TNYVFSDIR) libtinymailui-gtk $(MOZEMBEDDIR) $(PLATFORMDIR) libtinymail-camel tinymail docs $(UNITTESTDIR) bindings po

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc
