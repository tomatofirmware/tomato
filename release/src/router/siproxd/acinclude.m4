dnl
dnl	History
dnl	-------
dnl	before time		a lot happend before start of history
dnl	13-Sep-2002	tries	test for libosip2 functions (number of args)
dnl	18-Dec-2007	tries	test for new osip_message_t structure
dnl
dnl
dnl --------------------------------------------------------------------
dnl @synopsis ACX_PTHREAD([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl This macro figures out how to build C programs using POSIX
dnl threads.  It sets the PTHREAD_LIBS output variable to the threads
dnl library and linker flags, and the PTHREAD_CFLAGS output variable
dnl to any special C compiler flags that are needed.  (The user can also
dnl force certain compiler flags/libs to be tested by setting these
dnl environment variables.)
dnl
dnl Also sets PTHREAD_CC to any special C compiler that is needed for
dnl multi-threaded programs (defaults to the value of CC otherwise).
dnl (This is necessary on AIX to use the special cc_r compiler alias.)
dnl
dnl If you are only building threads programs, you may wish to
dnl use these variables in your default LIBS, CFLAGS, and CC:
dnl
dnl        LIBS="$PTHREAD_LIBS $LIBS"
dnl        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
dnl        LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"
dnl        CC="$PTHREAD_CC"
dnl
dnl In addition, if the PTHREAD_CREATE_JOINABLE thread-attribute
dnl constant has a nonstandard name, defines PTHREAD_CREATE_JOINABLE
dnl to that name (e.g. PTHREAD_CREATE_UNDETACHED on AIX).
dnl
dnl ACTION-IF-FOUND is a list of shell commands to run if a threads
dnl library is found, and ACTION-IF-NOT-FOUND is a list of commands
dnl to run it if it is not found.  If ACTION-IF-FOUND is not specified,
dnl the default action will define HAVE_PTHREAD.
dnl
dnl Please let the authors know if this macro fails on any platform,
dnl or if you have any other suggestions or comments.  This macro was
dnl based on work by SGJ on autoconf scripts for FFTW (www.fftw.org)
dnl (with help from M. Frigo), as well as ac_pthread and hb_pthread
dnl macros posted by AFC to the autoconf macro repository.  We are also
dnl grateful for the helpful feedback of numerous users.
dnl
dnl @version $Id: acinclude.m4 399 2008-11-08 15:18:59Z hb9xar $
dnl @author Steven G. Johnson <stevenj@alum.mit.edu> and Alejandro Forero Cuervo <bachue@bachue.com>

AC_DEFUN([ACX_PTHREAD], [
AC_REQUIRE([AC_CANONICAL_HOST])
acx_pthread_ok=no

# We used to check for pthread.h first, but this fails if pthread.h
# requires special compiler flags (e.g. on True64 or Sequent).
# It gets checked for in the link test anyway.

# First of all, check if the user has set any of the PTHREAD_LIBS,
# etcetera environment variables, and if threads linking works using
# them:
if test x"$PTHREAD_LIBS$PTHREAD_CFLAGS" != x; then
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        AC_MSG_CHECKING([for pthread_join in LIBS=$PTHREAD_LIBS with CFLAGS=$PTHREAD_CFLAGS])
        AC_TRY_LINK_FUNC(pthread_join, acx_pthread_ok=yes)
        AC_MSG_RESULT($acx_pthread_ok)
        if test x"$acx_pthread_ok" = xno; then
                PTHREAD_LIBS=""
                PTHREAD_CFLAGS=""
                PTHREAD_LDFLAGS=""
        fi
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
        LDFLAGS="$save_LDFLAGS"
fi

# We must check for the threads library under a number of different
# names; the ordering is very important because some systems
# (e.g. DEC) have both -lpthread and -lpthreads, where one of the
# libraries is broken (non-POSIX).

# Create a list of thread flags to try.  Items starting with a "-" are
# C compiler flags, and other items are library names, except for "none"
# which indicates that we try without any flags at all.

acx_pthread_flags="pthreads none -Kthread -kthread lthread -pthread -pthreads -mthreads pthread --thread-safe -mt"

# The ordering *is* (sometimes) important.  Some notes on the
# individual items follow:

# pthreads: AIX (must check this before -lpthread)
# none: in case threads are in libc; should be tried before -Kthread and
#       other compiler flags to prevent continual compiler warnings
# -Kthread: Sequent (threads in libc, but -Kthread needed for pthread.h)
# -kthread: FreeBSD kernel threads (preferred to -pthread since SMP-able)
# lthread: LinuxThreads port on FreeBSD (also preferred to -pthread)
# -pthread: Linux/gcc (kernel threads), BSD/gcc (userland threads)
# -pthreads: Solaris/gcc
# -mthreads: Mingw32/gcc, Lynx/gcc
# -mt: Sun Workshop C (may only link SunOS threads [-lthread], but it
#      doesn't hurt to check since this sometimes defines pthreads too;
#      also defines -D_REENTRANT)
# pthread: Linux, etcetera
# --thread-safe: KAI C++

case "${host_cpu}-${host_os}" in
        *solaris*)

        # On Solaris (at least, for some versions), libc contains stubbed
        # (non-functional) versions of the pthreads routines, so link-based
        # tests will erroneously succeed.  (We need to link with -pthread or
        # -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
        # a function called by this macro, so we could check for that, but
        # who knows whether they'll stub that too in a future libc.)  So,
        # we'll just look for -pthreads and -lpthread first:

        acx_pthread_flags="-pthread -pthreads pthread -mt $acx_pthread_flags"
        ;;
esac

if test x"$acx_pthread_ok" = xno; then
for flag in $acx_pthread_flags; do

        case $flag in
                none)
                AC_MSG_CHECKING([whether pthreads work without any flags])
                ;;

                -*)
                AC_MSG_CHECKING([whether pthreads work with $flag])
                PTHREAD_CFLAGS="$flag"
                ;;

                *)
                AC_MSG_CHECKING([for the pthreads library -l$flag])
                PTHREAD_LIBS="-l$flag"
                ;;
        esac

        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        save_LDFLAGS="$LDFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"

        # Check for various functions.  We must include pthread.h,
        # since some functions may be macros.  (On the Sequent, we
        # need a special flag -Kthread to make this header compile.)
        # We check for pthread_join because it is in -lpthread on IRIX
        # while pthread_create is in libc.  We check for pthread_attr_init
        # due to DEC craziness with -lpthreads.  We check for
        # pthread_cleanup_push because it is one of the few pthread
        # functions on Solaris that doesn't have a non-functional libc stub.
        # We try pthread_create on general principles.
        AC_TRY_LINK([#include <pthread.h>],
                    [pthread_t th; pthread_join(th, 0);
                     pthread_attr_init(0); pthread_cleanup_push(0, 0);
                     pthread_create(0,0,0,0); pthread_cleanup_pop(0); ],
                    [acx_pthread_ok=yes])

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
        LDFLAGS="$save_LDFLAGS"

        AC_MSG_RESULT($acx_pthread_ok)
        if test "x$acx_pthread_ok" = xyes; then
                break;
        fi

        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
        PTHREAD_LDFLAGS=""
done
fi

# Various other checks:
if test "x$acx_pthread_ok" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"

        # Detect AIX lossage: threads are created detached by default
        # and the JOINABLE attribute has a nonstandard name (UNDETACHED).
        AC_MSG_CHECKING([for joinable pthread attribute])
        AC_TRY_LINK([#include <pthread.h>],
                    [int attr=PTHREAD_CREATE_JOINABLE;],
                    ok=PTHREAD_CREATE_JOINABLE, ok=unknown)
        if test x"$ok" = xunknown; then
                AC_TRY_LINK([#include <pthread.h>],
                            [int attr=PTHREAD_CREATE_UNDETACHED;],
                            ok=PTHREAD_CREATE_UNDETACHED, ok=unknown)
        fi
        if test x"$ok" != xPTHREAD_CREATE_JOINABLE; then
                AC_DEFINE(PTHREAD_CREATE_JOINABLE, $ok,
                          [Define to the necessary symbol if this constant
                           uses a non-standard name on your system.])
        fi
        AC_MSG_RESULT(${ok})
        if test x"$ok" = xunknown; then
                AC_MSG_WARN([we do not know how to create joinable pthreads])
        fi

        AC_MSG_CHECKING([if more special flags are required for pthreads])
        flag=no
        case "${host_cpu}-${host_os}" in
                *-aix* | *-freebsd*)     flag="-D_THREAD_SAFE";;
                *solaris* | alpha*-osf*) flag="-D_REENTRANT";;
        esac
        AC_MSG_RESULT(${flag})
        if test "x$flag" != xno; then
                PTHREAD_CFLAGS="$flag $PTHREAD_CFLAGS"
        fi

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
        LDFLAGS="$save_LDFLAGS"

        # More AIX lossage: must compile with cc_r
        AC_CHECK_PROG(PTHREAD_CC, cc_r, cc_r, ${CC})
else
        PTHREAD_CC="$CC"
fi

AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LDFLAGS)
AC_SUBST(PTHREAD_CC)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_pthread_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.]),[$1])
        :
else
        acx_pthread_ok=no
        $2
fi

])dnl ACX_PTHREAD

dnl --------------------------------------------------------------------
dnl @synopsis ACX_WHICH_GETHOSTBYNAME_R
dnl
dnl Provides a test to determine the correct 
dnl way to call gethostbyname_r
dnl
dnl defines HAVE_FUNC_GETHOSTBYNAME_R_6 if it needs 6 arguments (e.g linux)
dnl defines HAVE_FUNC_GETHOSTBYNAME_R_5 if it needs 5 arguments (e.g. solaris)
dnl defines HAVE_FUNC_GETHOSTBYNAME_R_3 if it needs 3 arguments (e.g. osf/1)
dnl
dnl if used in conjunction in gethostname.c the api demonstrated
dnl in test.c can be used regardless of which gethostbyname_r 
dnl exists. These example files found at
dnl http://www.csn.ul.ie/~caolan/publink/gethostbyname_r
dnl
dnl @version $Id: acinclude.m4 399 2008-11-08 15:18:59Z hb9xar $
dnl @author Caolan McNamara <caolan@skynet.ie>
dnl
dnl based on David Arnold's autoconf suggestion in the threads faq
dnl
AC_DEFUN([ACX_WHICH_GETHOSTBYNAME_R], [

AC_CHECK_FUNC(gethostbyname_r, [
  AC_MSG_CHECKING(how many arguments takes gethostbyname_r)
  AC_TRY_COMPILE([
	#include <stdlib.h>
        #include <netdb.h>], [
        char *name;
        struct hostent *he;
        struct hostent_data data;
        (void) gethostbyname_r(name, he, &data);
  ],acx_which_gethostname_r=three, [
dnl
    AC_TRY_COMPILE([
	#include <stdlib.h>
        #include <netdb.h>], [
	char *name;
	struct hostent *he, *res;
	char buffer[2048];
	int buflen = 2048;
	int h_errnop;
	(void) gethostbyname_r(name, he, buffer, buflen, &res, &h_errnop)
    ],acx_which_gethostname_r=six, [
dnl
      AC_TRY_COMPILE([
	#include <stdlib.h>
        #include <netdb.h>], [
	char *name;
	struct hostent *he;
	char buffer[2048];
	int buflen = 2048;
	int h_errnop;
	(void) gethostbyname_r(name, he, buffer, buflen, &h_errnop)
      ],acx_which_gethostname_r=five,acx_which_gethostname_r=no)

    ])
  ])
], acx_which_gethostname_r=no)

if test $acx_which_gethostname_r = six; then
  AC_DEFINE(HAVE_FUNC_GETHOSTBYNAME_R_6,,[gethostbyname_r takes 6 arguments])
elif test $acx_which_gethostname_r = five; then
  AC_DEFINE(HAVE_FUNC_GETHOSTBYNAME_R_5,,[gethostbyname_r takes 5 arguments])
elif test $acx_which_gethostname_r = three; then
  AC_DEFINE(HAVE_FUNC_GETHOSTBYNAME_R_3,,[gethostbyname_r takes 3 arguments])

fi

AC_MSG_RESULT($acx_which_gethostname_r)

])dnl ACX_WHICH_GETHOSTBYNAME_R

dnl --------------------------------------------------------------------
dnl @synopsis ACX_CHECK_LIBOSIP_VERSION
dnl
dnl Provides a test to determine the version of libosip installed.
dnl
dnl Fails ./configure if a version < 3.0.0 
dnl
dnl Actually, this is quite some guesswork as libosip2 does not include
dnl any version info in its header files. So I just look for the new data
dnl structures in osip_message_t structure.
dnl
AC_DEFUN([ACX_CHECK_LIBOSIP_VERSION], [

acx_check_libosip_version=fail

dnl -----------------------------------
AC_MSG_CHECKING(libosip2 version > 3.0.0)

dnl check if contacts member is a structure with nb_elt (ok) or a
dnl pointer to the structure (bad)
AC_COMPILE_IFELSE([
  #include <osipparser2/osip_parser.h>
  main() {
  osip_message_t t;
  int  e;
  e=t.contacts.nb_elt;
  }
],acx_check_libosip_version=ok, )

if test $acx_check_libosip_version = fail; then
  echo "*** ERROR: libosip2-3.x.x is required!";exit 1;
fi
AC_MSG_RESULT(ok)


dnl
dnl With libosip2-3.1.0, some MD5 related function names have changed.
dnl Check for new names and if not present, create #defines that point
dnl to the original old ones.
dnl 
AC_CHECK_FUNCS(osip_MD5Init,
    ,
    AC_MSG_RESULT(libosip2 is < 3.1.0 - creating some required defines)
    AC_DEFINE(osip_MD5Init, [MD5Init],[function  osip_MD5Init])
    AC_DEFINE(osip_MD5Update, [MD5Update],[function  osip_MD5Update])
    AC_DEFINE(osip_MD5Final, [MD5Final],[function  osip_MD5Final])
    AC_DEFINE(osip_MD5Init, [MD5Init],[function  osip_MD5Init])
    AC_DEFINE(osip_MD5_CTX, [MD5_CTX],[type osip_MD5_CTX]),
    )

])dnl ACX_CHECK_LIBOSIP_VERSION

