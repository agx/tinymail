
srcdir=$1


find $srcdir -type f -iname "*.h" -printf "#include <%f>\n" | grep -v "\-priv" | grep -v "\-test" | grep -v "config.h" | grep -v "tny-summary-window.h" | grep -v "tny-summary-view.h" | grep -v "tny-folder-list.h" | grep -v "tny-platform-factory.h" | grep -v "tny-account-store.h" | grep -v "tny-device.h" | grep -v "tny-password-dialog.h" | grep -v "folder-lister.h" | grep -v "platfact.h" | grep -v "device.h" | grep -v "account-store.h" | grep -v camel > docs/devel/reference/libtinymail.types

echo >> docs/devel/reference/libtinymail.types

find $srcdir -type f -iname "*.h" -exec grep get_type {} \; | grep -v _tny_list_iterator | grep -v tny_summary_view | grep -v define | grep -v _test | grep -v tny_summary_window |grep -v tny_folder_list | grep -v "tny-platform-factory" | grep -v "tny-account-store" | grep -v "tny-device" | grep -v "device_get_type" | grep -v "tny-password-dialog" |grep -v camel | cut -d " " -f 2- | cut -d "(" -f 1| sed s/\ //g >> docs/devel/reference/libtinymail.types

