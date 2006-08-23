SUBDIRS = m4 

SUBDIRS += libtinymail 
SUBDIRS += libtinymailui 

if BUILD_GNOME
SUBDIRS += libtinymail-gnomevfs 
endif

SUBDIRS += libtinymailui-gtk 

if BUILD_MOZEMBED
SUBDIRS += libtinymailui-mozembed 
endif

SUBDIRS += $(PLATFORMDIR) libtinymail-camel tinymail docs bindings po tests

if BUILD_UNITTESTS
SUBDIRS += libtinymail-test 
endif

DIST_SUBDIRS = \
	libtinymail \
	libtinymailui \
	libtinymail-gnomevfs \
	libtinymailui-gtk \
	libtinymail-camel \
	tinymail docs \
	libtinymail-test tests \
	bindings po \
	libtinymail-maemo \
	libtinymail-olpc \
	libtinymail-gpe m4 \
	libtinymailui-mozembed \
	libtinymail-gnome-desktop

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-python-bindings --enable-unit-tests --enable-tests

