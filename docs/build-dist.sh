
# apt-get install check

svn co https://svn.tinymail.org/svn/tinymail
svn co http://svn.gnome.org/svn/asyncworker

cd asyncworker/trunk
./autogen.sh --prefix=/opt/asyncworker
make && sudo make install

cd tinymail/trunk
PKG_CONFIG_PATH=/opt/asyncworker/lib/pkgconfig/ ./autogen.sh --enable-gtk-doc --enable-tests --enable-asyncworker --enable-unit-tests
make
make distcheck

