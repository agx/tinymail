AC_DEFUN([AC_TNY_MOZ_CHECK],
[

mozilla_nspr="no"
AC_MSG_CHECKING(Mozilla NSPR pkg-config module name)
mozilla_nspr_pcs="nspr mozilla-nspr firefox-nspr xulrunner-nspr microb-engine-nspr"
for pc in $mozilla_nspr_pcs; do
        if $PKG_CONFIG --exists $pc; then
                mozilla_nspr=$pc
                break;
        fi
done
AC_MSG_RESULT($mozilla_nspr)

mozilla_nss="no"
AC_MSG_CHECKING(Mozilla NSS pkg-config module name)
mozilla_nss_pcs="nss mozilla-nss firefox-nss xulrunner-nss microb-engine-nss"
for pc in $mozilla_nss_pcs; do
        if $PKG_CONFIG --exists $pc; then
                mozilla_nss=$pc
                break;
        fi
done
AC_MSG_RESULT($mozilla_nss)

mozilla_xpcom="no"
mozilla_home="no"
AC_MSG_CHECKING(Mozilla XPCOM pkg-config module name)
mozilla_xpcom_pcs="xpcom mozilla-xpcom firefox-xpcom xulrunner-xpcom microb-engine-xpcom"
for pc in $mozilla_xpcom_pcs; do
        if $PKG_CONFIG --exists $pc; then
                mozilla_xpcom=$pc
		mozilla_home="`$PKG_CONFIG --variable=libdir $pc`"
                break;
        fi
done
AC_MSG_RESULT($mozilla_xpcom)
AC_MSG_RESULT($mozilla_home)

mozilla_gtkmozembed="no"
AC_MSG_CHECKING(Mozilla gtkmozembed pkg-config module name)
mozilla_gtkmozembed_pcs="gtkmozembed mozilla-gtkmozembed firefox-gtkmozembed xulrunner-gtkmozembed microb-engine-gtkembedmoz gtkembedmoz"
for pc in $mozilla_gtkmozembed_pcs; do
        if $PKG_CONFIG --exists $pc; then
                mozilla_gtkmozembed=$pc
                break;
        fi
done
AC_MSG_RESULT($mozilla_gtkmozembed)

#Detect Mozilla XPCom version
if test x$mozilla_xpcom != x; then

   AC_MSG_CHECKING(Mozilla xpcom engine version)
   #The only real way to detect the MOZILLA engine version is using the version in mozilla-config.h
   #of the engine we use.
   mozilla_xpcom_includedir="`$PKG_CONFIG --variable=includedir $mozilla_xpcom`"
   mozilla_version=`cat $mozilla_xpcom_includedir/mozilla-config.h | grep MOZILLA_VERSION_U | sed "s/.*_VERSION_U\ //"|tr ".abpre+" " "`
   mozilla_major_version=`echo $mozilla_version | awk ' { print $[1]; } '`
   mozilla_minor_version=`echo $mozilla_version | awk ' { print $[2]; } '`

   if test "$mozilla_major_version" != "1"; then
      AC_DEFINE([HAVE_MOZILLA_1_9],[1],[Define if we have mozilla api 1.9])
      AC_DEFINE([HAVE_MOZILLA_1_8],[1],[Define if we have mozilla api 1.8])
   fi

   if test "$mozilla_major_version" = "1" -a "$mozilla_minor_version" -ge "8"; then
      AC_DEFINE([HAVE_MOZILLA_1_8],[1],[Define if we have mozilla api 1.8])
   fi
      
   if test "$mozilla_major_version" = "1" -a "$mozilla_minor_version" -ge "9"; then
      AC_DEFINE([HAVE_MOZILLA_1_9],[1],[Define if we have mozilla api 1.9])
   fi

   AC_MSG_RESULT($mozilla_version)


fi


AM_CONDITIONAL([HAVE_MOZILLA_1_8],[test "$mozilla_major_version" = "1" -a "$mozilla_minor_version" -ge "8"])
AM_CONDITIONAL([HAVE_MOZILLA_1_9],[test "$mozilla_major_version" = "1" -a "$mozilla_minor_version" -ge "9"])
AC_DEFINE([MOZEMBED_MOZILLA_VERSION],["$mozilla_version"],[Detected mozilla engine version for mozembed])

])

