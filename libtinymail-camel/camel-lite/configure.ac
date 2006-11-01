AC_INIT([camel-lite],[1.0],[http://tinymail.org/])
AC_CONFIG_SRCDIR(camel/camel-object.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS")
AC_CONFIG_HEADERS(config.h)

BASE_VERSION=1.0
API_VERSION=1.2
AC_SUBST(BASE_VERSION)
AC_SUBST(API_VERSION)
AC_DEFINE_UNQUOTED(BASE_VERSION, "$BASE_VERSION", [Base version (Major.Minor)])
AC_DEFINE_UNQUOTED(API_VERSION, "$API_VERSION", [Api version (Major.Minor)])

# Libtool versioning
LIBEDATASERVER_CURRENT=8
LIBEDATASERVER_REVISION=0
LIBEDATASERVER_AGE=1
LIBCAMEL_CURRENT=9
LIBCAMEL_REVISION=0
LIBCAMEL_AGE=1

AC_SUBST(LIBEDATASERVER_CURRENT)
AC_SUBST(LIBEDATASERVER_REVISION)
AC_SUBST(LIBEDATASERVER_AGE)
AC_SUBST(LIBCAMEL_CURRENT)
AC_SUBST(LIBCAMEL_REVISION)
AC_SUBST(LIBCAMEL_AGE)

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

dnl Initialize maintainer mode
AC_PROG_LIBTOOL
AC_ISC_POSIX
AC_PROG_CC
AC_C_INLINE
AC_HEADER_STDC([])
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_SYS_LARGEFILE

GETTEXT_PACKAGE=camel-lite
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for gettext])


dnl ******************************
dnl Check for Win32
dnl ******************************

AC_MSG_CHECKING([for Win32])
case "$host" in
*-mingw*)
    os_win32=yes
    NO_UNDEFINED='-no-undefined'
    SOCKET_LIBS='-lws2_32 -ldnsapi'
    DL_LIB=''
    SOFTOKN3_LIB=''
    LIBEXECDIR_IN_SERVER_FILE='../../../libexec'
    ;;
*)  os_win32=no
    NO_UNDEFINED=''
    SOCKET_LIBS=''
    DL_LIB='-ldl'
    SOFTOKN3_LIB='-lsoftokn3'
    LIBEXECDIR_IN_SERVER_FILE="$libexecdir"
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, [test $os_win32 = yes])
AC_SUBST(NO_UNDEFINED)
AC_SUBST(SOCKET_LIBS)
AC_SUBST(LIBEXECDIR_IN_SERVER_FILE)

AC_CHECK_HEADERS(pthread.h semaphore.h sys/wait.h)
AC_CHECK_FUNCS(fsync strptime strtok_r)
AC_CHECK_HEADERS(alloca.h)

AC_CHECK_FUNCS(regexec,,[AC_CHECK_LIB(regex,regexec,
	      [REGEX_LIBS=-lregex
	       AC_DEFINE(HAVE_REGEXEC,1,[Define to 1 if you have the regexec function.])],
	      [AC_MSG_ERROR([No regex library found])])])
AC_SUBST(REGEX_LIBS)

have_iconv=no
AC_ARG_WITH(libiconv, [  --with-libiconv         Prefix where libiconv is installed])
case $withval in
/*)
    ICONV_CFLAGS="-I$withval/include"
    ICONV_LIBS="-L$withval/lib"
    ;;
esac

save_CFLAGS="$CFLAGS"
save_LIBS="$LIBS"
CFLAGS="$CFLAGS $ICONV_CFLAGS"
LIBS="$LIBS $ICONV_LIBS -liconv"
AC_CACHE_CHECK(for iconv in -liconv, ac_cv_libiconv, AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <iconv.h>
]], [[
	iconv_t cd;
	cd = iconv_open ("UTF-8", "ISO-8859-1");
]])],[ac_cv_libiconv=yes],[ac_cv_libiconv=no]))
if test $ac_cv_libiconv = yes; then
	ICONV_LIBS="$ICONV_LIBS -liconv"
	have_iconv=yes
else
	CFLAGS="$save_CFLAGS"
	LIBS="$save_LIBS"
	AC_CHECK_FUNC(iconv, have_iconv=yes, have_iconv=no)
fi

if test $have_iconv = yes; then
	if test $ac_cv_libiconv = no; then
		AC_CHECK_FUNCS(gnu_get_libc_version)
	fi
	AC_CACHE_CHECK([if iconv() handles UTF-8], ac_cv_libiconv_utf8, AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_GNU_GET_LIBC_VERSION
#include <gnu/libc-version.h>
#endif

int main (int argc, char **argv)
{
	const char *from = "Some Text \xA4";
	const char *utf8 = "Some Text \xC2\xA4";
	char *transbuf = malloc (20), *trans = transbuf;
	iconv_t cd;
	size_t from_len = strlen (from), utf8_len = 20;
	size_t utf8_real_len = strlen (utf8);

#ifdef HAVE_GNU_GET_LIBC_VERSION
	/* glibc 2.1.2's iconv is broken in hard to test ways. */
	if (!strcmp (gnu_get_libc_version (), "2.1.2"))
		exit (1);
#endif

	cd = iconv_open ("UTF-8", "ISO-8859-1");
	if (cd == (iconv_t) -1)
		exit (1);
	if (iconv (cd, &from, &from_len, &trans, &utf8_len) == -1 || from_len != 0)
		exit (1);
	if (memcmp (utf8, transbuf, utf8_real_len) != 0)
		exit (1);

	exit (0);
}]])],[ac_cv_libiconv_utf8=yes],[ac_cv_libiconv_utf8=no; have_iconv=no],[ac_cv_libiconv_utf8=no; have_iconv=no]))
fi

if test "$have_iconv" = no; then
	AC_MSG_ERROR([You need to install a working iconv implementation, such as ftp://ftp.gnu.org/pub/gnu/libiconv])
fi
AC_SUBST(ICONV_CFLAGS)
AC_SUBST(ICONV_LIBS)

CFLAGS="$CFLAGS -I$srcdir"

AC_MSG_CHECKING(preferred charset formats for system iconv)
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#define CONFIGURE_IN
#include "iconv-detect.c"
]])],[
	AC_MSG_RESULT(found)
],[
	AC_MSG_RESULT(not found)
],[])

CFLAGS="$save_CFLAGS"
LIBS="$save_LIBS"

dnl Check to see if strftime supports the use of %l and %k

AC_MSG_CHECKING(for %l and %k support in strftime)
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
	char buf[10];
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo=localtime(&rawtime);
	buf[0] = '\0';
	strftime(buf, 10, "%lx%k", timeinfo);

	if (buf[0] == '\0' || buf[0] == 'x' || strstr(buf, "l") || strstr(buf, "k"))
		exit(1);
	else
		exit(0);
}]])],[
AC_DEFINE(HAVE_LKSTRFTIME, 1, [strftime supports use of l and k])
ac_cv_lkstrftime=yes
],[ac_cv_lkstrftime=no],[ac_cv_lkstrftime=no])
AC_MSG_RESULT($ac_cv_lkstrftime)

msg_ssl="no"
msg_smime="no"
mozilla_nspr_pcs=""

AC_MSG_CHECKING(Mozilla NSPR pkg-config module name)
mozilla_nspr_pcs="nspr mozilla-nspr firefox-nspr xulrunner-nspr"
for pc in $mozilla_nspr_pcs; do
	if $PKG_CONFIG --exists $pc; then
		AC_MSG_RESULT($pc)
		mozilla_nspr=$pc
		break;
	fi
done
		
AC_MSG_CHECKING(Mozilla NSS pkg-config module name)
mozilla_nss_pcs="nss mozilla-nss firefox-nss xulrunner-nss"
for pc in $mozilla_nss_pcs; do
	if $PKG_CONFIG --exists $pc; then
		AC_MSG_RESULT($pc)
		mozilla_nss=$pc
		break;
	fi
done

SSL_LIBS=""

if test -n "$mozilla_nspr" -a -n "$mozilla_nss"; then
	msg_ssl="yes (Mozilla NSS)"
	msg_smime="yes (Mozilla NSS)"
	AC_DEFINE(ENABLE_SMIME,1,[Define if SMIME should be enabled])
	AC_DEFINE(HAVE_NSS,1,[Define if you have NSS])
	AC_DEFINE(HAVE_SSL,1,[Define if you have a supported SSL library])
	AC_DEFINE(HAVE_OPENSSL,0,[Define if you want to use OpenSSL as SSL library])
else
	AC_MSG_CHECKING(for NSPR and NSS)
	AC_MSG_RESULT(no)

	AC_CHECK_LIB(crypto, EVP_md5, [
		SSL_LIBS="-lcrypto -lssl"
		msg_ssl="yes (OpenSSL)"
		AC_DEFINE(HAVE_SSL,1,[Define if you have a supported SSL library])
		AC_DEFINE(HAVE_OPENSSL,1,[Define if you want to use OpenSSL as SSL library])
	],,)
fi

AC_SUBST(SSL_LIBS)
AC_MSG_CHECKING(Support for SSL)
AC_MSG_RESULT($msg_ssl)
AC_MSG_CHECKING(Support for SMIME)
AC_MSG_RESULT($msg_smime)

AM_CONDITIONAL(ENABLE_SMIME, test "x$msg_smime" != "xno")
AM_CONDITIONAL(ENABLE_OPENSSL, [test "x$msg_ssl" == "xyes (OpenSSL)"])

dnl
dnl system mail stuff
dnl
AC_PATH_PROG(SENDMAIL, sendmail, /usr/sbin/sendmail, /usr/sbin:/usr/lib)
AC_DEFINE_UNQUOTED(SENDMAIL_PATH, "$SENDMAIL", [Path to a sendmail binary, or equivalent])

AC_MSG_CHECKING(system mail directory)
if test -d /var/mail -a '!' -h /var/mail ; then
	system_mail_dir=/var/mail
else
	system_mail_dir=/var/spool/mail
fi
AC_DEFINE_UNQUOTED(SYSTEM_MAIL_DIR, "$system_mail_dir", [Directory local mail is delivered to])

case `ls -ld $system_mail_dir 2>&1 | awk '{print $1;}'` in
d??????rw?)
	CAMEL_LOCK_HELPER_USER=""
	CAMEL_LOCK_HELPER_GROUP=""
	system_mail_perm="world writable"
	;;

d???rw????)
	CAMEL_LOCK_HELPER_USER=""
	CAMEL_LOCK_HELPER_GROUP=`ls -ld $system_mail_dir 2>&1 | awk '{print $4;}'`
	system_mail_perm="writable by group $CAMEL_LOCK_HELPER_GROUP"
	;;

drw???????)
	CAMEL_LOCK_HELPER_USER=`ls -ld $system_mail_dir 2>&1 | awk '{print $3;}'`
	CAMEL_LOCK_HELPER_GROUP=""
	system_mail_perm="writable by user $CAMEL_LOCK_HELPER_USER"
	;;

*)
	CAMEL_LOCK_HELPER_USER=""
	CAMEL_LOCK_HELPER_GROUP=""
	system_mail_perm="???"
	;;
esac

AC_MSG_RESULT([$system_mail_dir, $system_mail_perm])
AC_SUBST(CAMEL_LOCK_HELPER_USER)
AC_SUBST(CAMEL_LOCK_HELPER_GROUP)


dnl ***************
dnl Timezone checks
dnl ***************
AC_CACHE_CHECK(for tm_gmtoff in struct tm, ac_cv_struct_tm_gmtoff,
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <time.h>
		]], [[
		struct tm tm;
		tm.tm_gmtoff = 1;
		]])],[ac_cv_struct_tm_gmtoff=yes],[ac_cv_struct_tm_gmtoff=no]))
if test $ac_cv_struct_tm_gmtoff = yes; then
	AC_DEFINE(HAVE_TM_GMTOFF, 1, [Define if struct tm has a tm_gmtoff member])
else
	AC_CACHE_CHECK(for timezone variable, ac_cv_var_timezone,
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
			#include <time.h>
		]], [[
			timezone = 1;
		]])],[ac_cv_var_timezone=yes],[ac_cv_var_timezone=no]))
	if test $ac_cv_var_timezone = yes; then
		AC_DEFINE(HAVE_TIMEZONE, 1, [Define if libc defines a timezone variable])
		AC_CACHE_CHECK(for altzone variable, ac_cv_var_altzone,
			AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
				#include <time.h>
			]], [[
				altzone = 1;
			]])],[ac_cv_var_altzone=yes],[ac_cv_var_altzone=no]))
		if test $ac_cv_var_altzone = yes; then
			AC_DEFINE(HAVE_ALTZONE, 1, [Define if libc defines an altzone variable])
		fi
	else
		AC_MSG_ERROR([unable to find a way to determine timezone])
	fi
fi


AC_CHECK_FUNCS(mkstemp mkdtemp isblank)

dnl **************************************************
dnl gethostbyname_r prototype
dnl **************************************************

AC_CHECK_FUNCS(gethostbyname_r,[
AC_CACHE_CHECK([if gethostbyname_r wants five arguments], ac_cv_gethostbyname_r_five_args,
[
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include "confdefs.h"
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <netdb.h>

		#define BUFSIZE (sizeof(struct hostent)+10)
	]], [[
		struct hostent hent;
		char buffer[BUFSIZE];
		int bufsize=BUFSIZE;
		int h_errno;

		(void)gethostbyname_r ("www.ximian.com", &hent, buffer, bufsize, &h_errno);
	]])],[
		ac_cv_gethostbyname_r_five_args=yes
	],[
		ac_cv_gethostbyname_r_five_args=no
	])
])])
	
if test "x$ac_cv_gethostbyname_r_five_args" = "xyes" ; then
	AC_DEFINE(GETHOSTBYNAME_R_FIVE_ARGS, 1, [Solaris-style gethostbyname_r])
fi

dnl **************************************************
dnl gethostbyaddr_r prototype
dnl **************************************************

AC_CHECK_FUNCS(gethostbyaddr_r,[
AC_CACHE_CHECK([if gethostbyaddr_r wants seven arguments], ac_cv_gethostbyaddr_r_seven_args,
[
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include "confdefs.h"
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <netdb.h>

		#define BUFSIZE (sizeof(struct hostent)+10)
	]], [[
		struct hostent hent;
		char buffer[BUFSIZE];
		int bufsize=BUFSIZE;
		int h_errno;

		(void)gethostbyaddr_r ("www.ximian.com", 14, AF_INET, &hent, buffer, bufsize, &h_errno);
	]])],[
		ac_cv_gethostbyaddr_r_seven_args=yes
	],[
		ac_cv_gethostbyaddr_r_seven_args=no
	])
])])
	
if test "x$ac_cv_gethostbyaddr_r_seven_args" = "xyes" ; then
	AC_DEFINE(GETHOSTBYADDR_R_SEVEN_ARGS, 1, [Solaris-style gethostbyaddr_r])
fi

dnl **************************************************
dnl stat(v)fs location/type
dnl **************************************************

AC_CHECK_HEADERS(sys/statvfs.h)
AC_CHECK_FUNCS(statvfs)
AC_CHECK_HEADERS(sys/param.h)
AC_CHECK_HEADERS(sys/mount.h)
AC_CHECK_FUNCS(statfs)

dnl **************************************************
dnl * IPv6 support and getaddrinfo calls
dnl **************************************************
AC_CACHE_CHECK([if system supports getaddrinfo and getnameinfo], have_addrinfo,
[
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include "confdefs.h"
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <netdb.h>
		#include <stddef.h>

	]], [[
		struct addrinfo hints, *res;
		struct sockaddr_in6 sin6;
		int af = AF_INET6;
		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];

		getaddrinfo ("www.ximian.com", NULL, &hints, &res);
		freeaddrinfo (res);
		getnameinfo((struct sockaddr *)&sin6, sizeof(sin6), host, sizeof(host), serv, sizeof(serv), 0);
	]])],[
		have_addrinfo=yes
	],[
		have_addrinfo=no
	])
])

if test x"$have_addrinfo" = "xno" ; then
   AC_DEFINE(NEED_ADDRINFO,1,[Enable getaddrinfo emulation])
   if test x"$enable_ipv6" = "xyes" ; then
      AC_MSG_ERROR([system doesn't support necessary interfaces for ipv6 support])
   fi
   msg_ipv6=no
else
   AC_ARG_ENABLE(ipv6, [  --enable-ipv6=[no/yes]      Enable support for resolving IPv6 addresses.],,enable_ipv6=yes)
   if test x"$enable_ipv6" = "xyes"; then
        msg_ipv6=yes
	AC_DEFINE(ENABLE_IPv6,1,[Enable IPv6 support])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include "confdefs.h"
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <netdb.h>

	]], [[
		struct addrinfo hints;
		
		hints.ai_flags = AI_ADDRCONFIG;
	]])],[
		AC_DEFINE(HAVE_AI_ADDRCONFIG,1,[Define if the system defines the AI_ADDRCONFIG flag for getaddrinfo])
	],[])
   else
	msg_ipv6=no
   fi
fi

AM_CONDITIONAL(ENABLE_IPv6, test "x$enable_ipv6" = "xyes")

dnl **************************************************
dnl NNTP support.
dnl **************************************************
AC_ARG_ENABLE(nntp, 
[  --enable-nntp=[no/yes]      Build Usenet news (NNTP) backend],,enable_nntp=yes)
if test "x$enable_nntp" = "xyes"; then
	AC_DEFINE(ENABLE_NNTP,1,[Build NNTP backend])
	msg_nntp=yes
else
	msg_nntp=no
fi
AM_CONDITIONAL(ENABLE_NNTP, test x$enable_nntp = xyes)

dnl **************************************************
dnl New IMAP code support.
dnl **************************************************
AC_ARG_ENABLE(imapp, 
[  --enable-imapp=[no/yes]      Attempt to compile alternative, incomplete, very unsupported IMAPv4r1 code],,enable_imapp=no)
if test "x$enable_imapp" = "xyes"; then
	AC_DEFINE(ENABLE_IMAPP,1,[Really don't try this at home])
	msg_imapp=yes
else
	msg_imapp=no
fi
AM_CONDITIONAL(ENABLE_IMAPP, test x$enable_imapp = xyes)

dnl **************************************************
dnl * File locking
dnl **************************************************

AC_ARG_ENABLE(dot-locking, 
[  --enable-dot-locking=[yes/no] Enable support for locking mail files with dot locking],,enable_dot_locking=yes)
if test $os_win32 != yes -a "x$enable_dot_locking" = "xyes"; then
  AC_DEFINE(USE_DOT,1,[Define to use dot locking for mbox files])
  msg_dot=yes
else
  msg_dot=no	
fi

AC_ARG_ENABLE(file-locking, 
[  --enable-file-locking=[fcntl/flock/no] Enable support for locking mail files with file locking],,enable_file_locking=fcntl)
if test $os_win32 != yes -a "x$enable_file_locking" = "xfcntl"; then
  AC_DEFINE(USE_FCNTL,1,[Define to use fcntl locking for mbox files])
  msg_file=fcntl
else
  if test $os_win32 != yes -a "x$enable_file_locking" = "xflock"; then
    AC_DEFINE(USE_FLOCK,1,[Define to use flock locking for mbox files])
    msg_file=flock
  else
    msg_file=no	
  fi
fi

dnl **************************************************
dnl * sendmail operation
dnl **************************************************

AC_MSG_CHECKING(for SunOS broken spool format)
if test "x$host_os" = "xsunos" ; then
   with_broken_spool="yes"
fi

AC_ARG_WITH(broken-spool, 
[  --with-broken-spool=[yes/no] Using SunOS/Solaris sendmail which has a broken spool format],,with_broken_spool=${with_broken_spool:=no})

if test "x$with_broken_spool" = "xyes"; then
  AC_DEFINE(HAVE_BROKEN_SPOOL,1,[Define if mail delivered to the system mail directory is in broken Content-Length format])
fi

AC_MSG_RESULT($with_broken_spool)

dnl ********
dnl Kerberos
dnl ********
AC_ARG_WITH(krb5, [  --with-krb5=DIR      Location of Kerberos 5 install dir], with_krb5="$withval", with_krb5="no")
AC_ARG_WITH(krb5-libs, [  --with-krb5-libs=DIR Location of Kerberos 5 libraries], with_krb5_libs="$withval", with_krb5_libs="$with_krb5/lib")
AC_ARG_WITH(krb5-includes, [  --with-krb5-includes=DIR Location of Kerberos 5 headers], with_krb5_includes="$withval", with_krb5_includes="")
AC_ARG_WITH(krb4, [  --with-krb4=DIR      Location of Kerberos 4 install dir], with_krb4="$withval", with_krb4="no")
AC_ARG_WITH(krb4-libs, [  --with-krb4-libs=DIR Location of Kerberos 4 libraries], with_krb4_libs="$withval", with_krb4_libs="$with_krb4/lib")
AC_ARG_WITH(krb4-includes, [  --with-krb4-includes=DIR Location of Kerberos 4 headers], with_krb4_includes="$withval", with_krb4_includes="")

msg_krb5="no"
if test "x${with_krb5}" != "xno"; then
	LDFLAGS_save="$LDFLAGS"
	
	mitlibs="-lkrb5 -lk5crypto -lcom_err -lgssapi_krb5"
	heimlibs="-lkrb5 -lcrypto -lasn1 -lcom_err -lroken -lgssapi"
	AC_CACHE_CHECK([for Kerberos 5], ac_cv_lib_kerberos5,
	[
		LDFLAGS="$LDFLAGS -L$with_krb5_libs $mitlibs"
		AC_TRY_LINK_FUNC(krb5_init_context, ac_cv_lib_kerberos5="$mitlibs",
		[
			LDFLAGS="$LDFLAGS_save -L$with_krb5_libs $heimlibs"
			AC_TRY_LINK_FUNC(krb5_init_context, ac_cv_lib_kerberos5="$heimlibs", ac_cv_lib_kerberos5="no")
		])
		LDFLAGS="$LDFLAGS_save"
	])
	if test "$ac_cv_lib_kerberos5" != "no"; then
		AC_DEFINE(HAVE_KRB5,1,[Define if you have Krb5])
		if test "$ac_cv_lib_kerberos5" = "$mitlibs"; then
			AC_DEFINE(HAVE_MIT_KRB5,1,[Define if you have MIT Krb5])
			if test -z "$with_krb5_includes"; then
				KRB5_CFLAGS="-I$with_krb5/include"
			else
				KRB5_CFLAGS="-I$with_krb5_includes"
			fi
			msg_krb5="yes (MIT)"
		else
			AC_DEFINE(HAVE_HEIMDAL_KRB5,1,[Define if you have Heimdal])
			if test -z "$with_krb5_includes"; then
				KRB5_CFLAGS="-I$with_krb5/include/heimdal"
			else
				KRB5_CFLAGS="-I$with_krb5_includes"
			fi
			msg_krb5="yes (Heimdal)"
		fi
		KRB5_LDFLAGS="-L$with_krb5_libs $ac_cv_lib_kerberos5"
	fi
else
	AC_MSG_CHECKING(for Kerberos 5)
	AC_MSG_RESULT($with_krb5)
fi
AM_CONDITIONAL(ENABLE_KRB5, test x$with_krb5 != xno)

AC_CHECK_HEADER([et/com_err.h],[AC_DEFINE([HAVE_ET_COM_ERR_H], 1, [Have et/comm_err.h])])
AC_CHECK_HEADER([com_err.h],[AC_DEFINE([HAVE_COM_ERR_H], 1, [Have comm_err.h])])

msg_krb4="no"
if test "x${with_krb4}" != "xno"; then
	LDFLAGS_save="$LDFLAGS"
	AC_CACHE_CHECK(for Kerberos 4, ac_cv_lib_kerberos4,
	[
		ac_cv_lib_kerberos4="no"

		mitcompatlibs="-lkrb4 -ldes425 -lkrb5 -lk5crypto -lcom_err"
		# Look for MIT krb5 compat krb4
		LDFLAGS="$LDFLAGS -L$with_krb4_libs $mitcompatlibs"
		AC_TRY_LINK_FUNC(krb_mk_req, ac_cv_lib_kerberos4="$mitcompatlibs")
		
		if test "$ac_cv_lib_kerberos4" = "no"; then
			# Look for KTH krb4
			LDFLAGS="$LDFLAGS_save -L$with_krb4_libs -lkrb -lcrypto -lcom_err -lroken"
			AC_TRY_LINK_FUNC(krb_mk_req, ac_cv_lib_kerberos4="-lkrb -lcrypto -lcom_err -lroken")
		fi
		if test "$ac_cv_lib_kerberos4" = "no"; then
			# Look for old MIT krb4
			LDFLAGS="$LDFLAGS_save -L$with_krb4_libs -lkrb"
			AC_TRY_LINK_FUNC(krb_mk_req, ac_cv_lib_kerberos4="-lkrb",
			[
				LDFLAGS="$LDFLAGS -ldes"
				AC_TRY_LINK_FUNC(krb_mk_req, ac_cv_lib_kerberos4="-lkrb -ldes")
			])
		fi
	])
	LDFLAGS="$LDFLAGS_save"
	if test "$ac_cv_lib_kerberos4" != "no"; then
		AC_DEFINE(HAVE_KRB4,1,[Define if you have Krb4])
		msg_krb4="yes"

		if test -z "$with_krb4_includes"; then
			if test -f "$with_krb4/include/krb.h" -o -f "$with_krb4/include/port-sockets.h"; then
				KRB4_CFLAGS="-I$with_krb4/include"
			fi
			if test -d "$with_krb4/include/kerberosIV"; then
				KRB4_CFLAGS="$KRB4_CFLAGS -I$with_krb4/include/kerberosIV"
			fi
		else
			KRB4_CFLAGS="-I$with_krb4_includes"
		fi
		KRB4_LDFLAGS="-L$with_krb4_libs $ac_cv_lib_kerberos4"
		
		CFLAGS_save="$CFLAGS"
		CFLAGS="$CFLAGS $KRB4_CFLAGS"
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include "krb.h"
		int krb_sendauth;
		]], [[return 0]])],[AC_DEFINE(NEED_KRB_SENDAUTH_PROTO,1,[Need krb_sendauth proto])],[])
		CFLAGS="$CFLAGS_save"
	fi
else
	AC_MSG_CHECKING(for Kerberos 4)
	AC_MSG_RESULT(${with_krb4})
fi

AC_SUBST(KRB5_CFLAGS)
AC_SUBST(KRB5_LDFLAGS)
AC_SUBST(KRB4_CFLAGS)
AC_SUBST(KRB4_LDFLAGS)

GLIB_REQUIRED="2.4.0"

AC_SUBST(GLIB_REQUIRED)

dnl We use AC_SUBST_FILE because AC_SUBST won't deal with newlines
EVO_MARSHAL_RULE=$srcdir/marshal.mk
AC_SUBST_FILE(EVO_MARSHAL_RULE)

AC_DEFINE(ENABLE_THREADS,1,[Required])


CFLAGS="$CFLAGS $WARN_CFLAGS"

PKG_CHECK_MODULES(E_DATA_SERVER, libxml-2.0 glib-2.0 gobject-2.0 $mozilla_nspr $mozilla_nss)
AC_SUBST(E_DATA_SERVER_CFLAGS)
AC_SUBST(E_DATA_SERVER_LIBS)


zlib_found=false
dnl deflateInit is a #define, use deflateEnd instead
AC_CHECK_LIB(z, deflateEnd,
    [ AC_CHECK_HEADER(zlib.h, zlib_found=true) ])                                                                                                                                      
if test "x$zlib_found" = "xfalse"; then
    AC_MSG_ERROR([*** zlib is required])
fi

PKG_CHECK_MODULES(CAMEL, gthread-2.0 gmodule-2.0 gobject-2.0 $mozilla_nss $mozilla_nspr)
CAMEL_CFLAGS="$CAMEL_CFLAGS $THREADS_CFLAGS $KRB4_CFLAGS $KRB5_CFLAGS"
CAMEL_LIBS="$CAMEL_LIBS -lz $THREADS_LIBS $KRB4_LDFLAGS $KRB5_LDFLAGS"

AC_SUBST(CAMEL_CFLAGS)
AC_SUBST(CAMEL_LIBS)

camel_lite_providerdir='${libdir}'/camel-lite-$API_VERSION/camel-providers
camel_providerdir=$camel_lite_providerdir
AC_SUBST(camel_lite_providerdir)
AC_SUBST(camel_providerdir)

AC_OUTPUT([
Makefile
camel/Makefile
camel/providers/Makefile
camel/providers/imap/Makefile
camel/providers/imapp/Makefile
camel/providers/imap4/Makefile
camel/providers/local/Makefile
camel/providers/nntp/Makefile
camel/providers/pop3/Makefile
camel/providers/sendmail/Makefile
camel/providers/smtp/Makefile
camel/camel-lite.pc
camel/camel-lite-provider.pc
libedataserver/Makefile
])

