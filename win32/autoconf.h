/* include/autoconf.h.  Generated by configure.  */
/* include/autoconf.h.in.  Generated from configure.in by autoheader.  */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_DIRENT_H */

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the `getrlimit' function. */
#define HAVE_GETRLIMIT 1

/* Define to 1 if you have the `getrusage' function. */
/* #undef HAVE_GETRUSAGE */

/* Define to 1 if you have the <in6.h> header file. */
/* #undef HAVE_IN6_H */

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef HAVE_LIBNSL */

/* Define to 1 if you have the `pcre' library (-lpcre). */
#define HAVE_LIBPCRE 1

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the `ssl' library (-lssl). */
#define HAVE_LIBSSL 1

/* Define to 1 if you have the <linux/in6.h> header file. */
/* #undef HAVE_LINUX_IN6_H */

/* Define to 1 if long double works and has more range or precision than
   double. */
#define HAVE_LONG_DOUBLE 1

/* Define to 1 if you have the `mallinfo' function. */
/* #undef HAVE_MALLINFO */

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netinet6/in6.h> header file. */
/* #undef HAVE_NETINET6_IN6_H */

/* ssl headers are under openssl dir. */
#define HAVE_OPENSSL 

/* Define to 1 if you have the `random' function. */
/* #undef HAVE_RANDOM */

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if `tm_zone' is member of `struct tm'. */
/* #undef HAVE_STRUCT_TM_TM_ZONE */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/errno.h> header file. */
#define HAVE_SYS_ERRNO_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/signal.h> header file. */
#define HAVE_SYS_SIGNAL_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
/* #undef HAVE_SYS_TIME_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <timebits.h> header file. */
/* #undef HAVE_TIMEBITS_H */

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
/* #undef HAVE_TM_ZONE */

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
#define HAVE_TZNAME 1

/* Define to 1 if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define to 1 if you have the <varargs.h> header file. */
/* #undef HAVE_VARARGS_H */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Configures which LINT to use to evaluate the sources. */
/* #undef LINT */

/* Enables memory usage profiling. */
/* #undef MALLOC_PROFILING */

/* With MALLOC_PROFILING, can detect double-frees, buffer overruns, etc. */
/* #undef MALLOC_PROFILING_EXTRA */

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "revar@belfry.com"

/* Define to the full name of this package. */
#define PACKAGE_NAME "fbmuck"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "fbmuck 6.02"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "fbmuck"

/* Define to the version of this package. */
#define PACKAGE_VERSION "6.02"

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `long int', as computed by sizeof. */
#define SIZEOF_LONG_INT 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
/* #undef TIME_WITH_SYS_TIME */

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* The value of the uname -a command. */
#define UNAME_VALUE "MSVC++ foxbird 1.5.10(0.116/4/2) 2004-05-25 22:07 i686 unknown unknown Cygwin"

/* Use IPv6 instead of IPv4 sockets. */
/* #undef USE_IPV6 */

/* Makefile is configured to run lint on sources. */
/* #undef WITH_LINT */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
   if it is not supported. */
/* #undef inline */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

/* The type that holds socket length values on this system. */
/* #undef socklen_t */

/* The base path to the installation for openssl. Usually /usr. */
#define ssldir "e:/cyg/openssl"

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

/* Defined to 1 if _timezone is defined. */
#define HAVE_DECL__TIMEZONE 1

/* Defined to 1 if struct tm.tm_gmtoff exists. */
/* #undef HAVE_STRUCT_TM_TM_GMTOFF */

/* Defined to 1 if struct mallinfo.hblks exists. */
/* #undef HAVE_STRUCT_MALLINFO_HBLKS */

/* Defined to 1 if struct mallinfo.keepcost exists. */
/* #undef HAVE_STRUCT_MALLINFO_KEEPCOST */

/* Defined to 1 if struct mallinfo.treeoverhead exists. */
/* #undef HAVE_STRUCT_MALLINFO_TREEOVERHEAD */

/* Defined to 1 if struct mallinfo.grain exists. */
/* #undef HAVE_STRUCT_MALLINFO_GRAIN */

/* Defined to 1 if struct mallinfo.allocated exists. */
/* #undef HAVE_STRUCT_MALLINFO_ALLOCATED */


