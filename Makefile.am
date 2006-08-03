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

SUBDIRS += $(PLATFORMDIR) libtinymail-camel tinymail docs 

if BUILD_UNITTESTS
SUBDIRS += libtinymail-test 
endif

SUBDIRS += bindings po 

DIST_SUBDIRS = \
	libtinymail \
	libtinymailui \
	libtinymail-gnomevfs \
	libtinymailui-gtk \
	libtinymail-camel \
	tinymail docs \
	libtinymail-test \
	bindings po \
	libtinymail-maemo \
	libtinymail-olpc \
	libtinymail-gpe m4 \
	libtinymailui-mozembed \
	libtinymail-gnome-desktop

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-python-bindings --enable-unit-tests
