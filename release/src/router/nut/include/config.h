/* include/config.h.  Generated from config.h.in by configure.  */
/* include/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Path for pid files of drivers and upsd (usually STATEPATH) */
#define ALTPIDPATH "/var/state/ups"

/* Default path for user executables */
#define BINDIR "/bin"

/* Default path for CGI programs */
#define CGIPATH "/www/nut"

/* Default path for configuration files */
#define CONFPATH "/usr/local/nut"

/* Define processor type */
#define CPU_TYPE mipsel

/* Default path for data files */
#define DATADIR "/usr/share"

/* Default path for UPS drivers */
#define DRVPATH "/usr/bin"

/* Define to nothing if C supports flexible array members, and to 1 if it does
   not. That way, with a declaration like `struct s { int n; double
   d[FLEXIBLE_ARRAY_MEMBER]; };', the struct hack can be used with pre-C99
   compilers. When computing the size of such an object, don't use 'sizeof
   (struct s)' as it overestimates the size. Use 'offsetof (struct s, d)'
   instead. Don't use 'offsetof (struct s, d[0])', as this doesn't work with
   MSVC and with C++ compilers. */
#define FLEXIBLE_ARRAY_MEMBER /**/

/* Define to 1 if you have the `atexit' function. */
#define HAVE_ATEXIT 1

/* Define to 1 if you have the <avahi-client/client.h> header file. */
/* #undef HAVE_AVAHI_CLIENT_CLIENT_H */

/* Define to 1 if you have the `avahi_client_new' function. */
/* #undef HAVE_AVAHI_CLIENT_NEW */

/* Define to 1 if you have the <avahi-common/malloc.h> header file. */
/* #undef HAVE_AVAHI_COMMON_MALLOC_H */

/* Define to 1 if you have the `avahi_free' function. */
/* #undef HAVE_AVAHI_FREE */

/* Define to 1 if you have the `cfsetispeed' function. */
#define HAVE_CFSETISPEED 1

/* Define to 1 if C supports variable-length arrays. */
#define HAVE_C_VARARRAYS 1

/* Define to 1 if you have the declaration of `i2c_smbus_read_block_data', and
   to 0 if you don't. */
#define HAVE_DECL_I2C_SMBUS_READ_BLOCK_DATA 0

/* Define to 1 if you have the declaration of `i2c_smbus_read_word_data', and
   to 0 if you don't. */
#define HAVE_DECL_I2C_SMBUS_READ_WORD_DATA 0

/* Define to 1 if you have the declaration of `i2c_smbus_write_word_data', and
   to 0 if you don't. */
#define HAVE_DECL_I2C_SMBUS_WRITE_WORD_DATA 0

/* Define to 1 if you have the declaration of `LOG_UPTO', and to 0 if you
   don't. */
#define HAVE_DECL_LOG_UPTO 1

/* Define to 1 if you have the declaration of `optind', and to 0 if you don't.
   */
#define HAVE_DECL_OPTIND 1

/* Define to 1 if you have the declaration of `uu_lock', and to 0 if you
   don't. */
#define HAVE_DECL_UU_LOCK 0

/* Define to 1 if you have the declaration of `__FUNCTION__', and to 0 if you
   don't. */
/* #undef HAVE_DECL___FUNCTION__ */

/* Define to 1 if you have the declaration of `__func__', and to 0 if you
   don't. */
#define HAVE_DECL___FUNC__ 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `fcvt' function. */
/* #undef HAVE_FCVT */

/* Define to 1 if you have the `fcvtl' function. */
/* #undef HAVE_FCVTL */

/* Define to 1 if you have the `flock' function. */
#define HAVE_FLOCK 1

/* Define if FreeIPMI support is available */
/* #undef HAVE_FREEIPMI */

/* Define if FreeIPMI 1.1.X / 1.2.X support is available */
/* #undef HAVE_FREEIPMI_11X_12X */

/* Define to 1 if you have the <freeipmi/freeipmi.h> header file. */
/* #undef HAVE_FREEIPMI_FREEIPMI_H */

/* Define if FreeIPMI monitoring support is available */
/* #undef HAVE_FREEIPMI_MONITORING */

/* Define to 1 if you have the <gdfontmb.h> header file. */
/* #undef HAVE_GDFONTMB_H */

/* Define to 1 if you have the <gd.h> header file. */
/* #undef HAVE_GD_H */

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define to 1 if you have the `getopt_long' function. */
#define HAVE_GETOPT_LONG 1

/* Define to 1 if you have the `getpassphrase' function. */
/* #undef HAVE_GETPASSPHRASE */

/* Define to 1 if you have the `init_snmp' function. */
/* #undef HAVE_INIT_SNMP */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <ipmi_monitoring.h> header file. */
/* #undef HAVE_IPMI_MONITORING_H */

/* Define if you have Boutell's libgd installed */
/* #undef HAVE_LIBGD */

/* Define to enable libltdl support */
/* #undef HAVE_LIBLTDL */

/* Define to 1 if you have the <libpowerman.h> header file. */
/* #undef HAVE_LIBPOWERMAN_H */

/* Define to 1 if you have the `lockf' function. */
#define HAVE_LOCKF 1

/* Define to 1 if the system has the type `long double'. */
/* #undef HAVE_LONG_DOUBLE */

/* Define to 1 if the system has the type 'long long int'. */
/* #undef HAVE_LONG_LONG_INT */

/* Define to 1 if you have the <ltdl.h> header file. */
/* #undef HAVE_LTDL_H */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <net-snmp/net-snmp-config.h> header file. */
/* #undef HAVE_NET_SNMP_NET_SNMP_CONFIG_H */

/* Define to 1 if you have the `ne_set_connect_timeout' function. */
/* #undef HAVE_NE_SET_CONNECT_TIMEOUT */

/* Define to 1 if you have the `ne_sock_connect_timeout' function. */
/* #undef HAVE_NE_SOCK_CONNECT_TIMEOUT */

/* Define to 1 if you have the <ne_xmlreq.h> header file. */
/* #undef HAVE_NE_XMLREQ_H */

/* Define to 1 if you have the `ne_xml_dispatch_request' function. */
/* #undef HAVE_NE_XML_DISPATCH_REQUEST */

/* Define to 1 if you have the <nss.h> header file. */
/* #undef HAVE_NSS_H */

/* Define to 1 if you have the `NSS_Init' function. */
/* #undef HAVE_NSS_INIT */

/* Define to 1 if you have the `on_exit' function. */
#define HAVE_ON_EXIT 1

/* Define to 1 if you have the <openssl/ssl.h> header file. */
/* #undef HAVE_OPENSSL_SSL_H */

/* Define to 1 if you have the `pm_connect' function. */
/* #undef HAVE_PM_CONNECT */

/* Define to enable pthread support code */
#define HAVE_PTHREAD 1

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the `seteuid' function. */
#define HAVE_SETEUID 1

/* Define to 1 if you have the `setlogmask' function. */
#define HAVE_SETLOGMASK 1

/* Define to 1 if you have the `setsid' function. */
#define HAVE_SETSID 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the `SSL_library_init' function. */
/* #undef HAVE_SSL_LIBRARY_INIT */

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strptime' function. */
#define HAVE_STRPTIME 1

/* Define to 1 if you have the <sys/modem.h> header file. */
/* #undef HAVE_SYS_MODEM_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/termios.h> header file. */
#define HAVE_SYS_TERMIOS_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <tcpd.h> header file. */
/* #undef HAVE_TCPD_H */

/* Define to 1 if you have the `tcsendbreak' function. */
#define HAVE_TCSENDBREAK 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if the system has the type 'unsigned long long int'. */
#define HAVE_UNSIGNED_LONG_LONG_INT 1

/* Define to 1 if you have the `usb_detach_kernel_driver_np' function. */
#define HAVE_USB_DETACH_KERNEL_DRIVER_NP 1

/* Define to 1 if you have the <usb.h> header file. */
#define HAVE_USB_H 1

/* Define to 1 if you have the `usb_init' function. */
#define HAVE_USB_INIT 1

/* Use uu_lock for locking (FreeBSD) */
/* #undef HAVE_UU_LOCK */

/* Define to 1 if you have the <varargs.h> header file. */
/* #undef HAVE_VARARGS_H */

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to enable libwrap support */
/* #undef HAVE_WRAP */

/* Default path for HTML files */
#define HTMLPATH "/usr/html"

/* Desired syslog facility - see syslog(3) */
#define LOG_FACILITY LOG_DAEMON

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Define to use explicit getopt declarations */
/* #undef NEED_GETOPT_DECLS */

/* Define if getopt.h is needed */
#define NEED_GETOPT_H 1

/* NUT network protocol version */
#define NUT_NETVERSION "1.2"

/* Name of package */
#define PACKAGE "nut"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://github.com/networkupstools/nut/issues"

/* Define to the full name of this package. */
#define PACKAGE_NAME "nut"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "nut 2.7.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "nut"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.7.3"

/* Path where the pid files should go */
#define PIDPATH "/var/run"

/* Port for network communications */
#define PORT 3493

/* Group membership of user to switch to if started as root */
#define RUN_AS_GROUP "nobody"

/* User to switch to if started as root */
#define RUN_AS_USER "nobody"

/* Default path for system executables */
#define SBINDIR "/usr/sbin"

/* Path for UPS driver state files */
#define STATEPATH "/var/state/ups"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 for Sun version of the libusb. */
/* #undef SUN_LIBUSB */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* NUT tree version */
#define TREE_VERSION "2.7"

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "2.7.3"

/* Define to enable Asciidoc support */
/* #undef WITH_ASCIIDOC */

/* Define to enable Avahi support */
/* #undef WITH_AVAHI */

/* Define to enable CGI (HTTP) support */
/* #undef WITH_CGI */

/* Define to enable development files support */
/* #undef WITH_DEV */

/* Define to enable IPMI support using FreeIPMI */
/* #undef WITH_FREEIPMI */

/* Define to enable IPMI support */
/* #undef WITH_IPMI */

/* Define to enable libltdl (Libtool dlopen abstraction) support */
/* #undef WITH_LIBLTDL */

/* Define to enable Powerman PDU support */
/* #undef WITH_LIBPOWERMAN */

/* Define to enable I2C support */
/* #undef WITH_LINUX_I2C */

/* Define to enable Mac OS X meta-driver */
/* #undef WITH_MACOSX */

/* Define to enable Neon HTTP support */
/* #undef WITH_NEON */

/* Define to enable SSL support using Mozilla NSS */
/* #undef WITH_NSS */

/* Define to enable SSL support using OpenSSL */
/* #undef WITH_OPENSSL */

/* Define to enable serial support */
#define WITH_SERIAL 1

/* Define to enable SNMP support */
/* #undef WITH_SNMP */

/* Define to enable SSL */
/* #undef WITH_SSL */

/* Define to enable USB support */
#define WITH_USB 1

/* Define to enable libwrap (tcp-wrappers) support */
/* #undef WITH_WRAP */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Replace missing __func__ declaration */
/* #undef __func__ */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* type to use in place of socklen_t if not defined */
/* #undef socklen_t */
