AC_PREREQ([2.66])
AC_INIT([muse_mirror], [0.0.0.1], [359687@mail.muni.cz], [muse_mirror])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([mirror.cpp])

AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

LT_PREREQ([2.2])
LT_INIT

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])
AC_LANG_SAVE
AC_PROG_INSTALL

# Checks for libraries.

PKG_CHECK_MODULES([libkerat], [libkerat], [
        AC_DEFINE([HAVE_LIBKERAT], [1], [Build with support for libkerat])
        MUSE_MIRROR_LIBS+=" $libkerat_LIBS"
        MUSE_MIRROR_CFLAGS+=" $libkerat_CFLAGS"
], [
        AC_MSG_ERROR([The kerat library is missing!])
])

AC_ARG_ENABLE(muse, [  --disable-muse        disable muse framework] )

if test x$enable_muse = x
then
    enable_muse=yes
fi

if test $enable_muse = yes
then
    PKG_CHECK_MODULES([muse], [libmuse], [
        AC_DEFINE([HAVE_MUSE], [1], [Build with support for MUSE framework])
        MUSE_MIRROR_LIBS+=" $muse_LIBS"
        MUSE_MIRROR_CFLAGS+=" $muse_CFLAGS"
    ])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# clock_gettime
AC_SEARCH_LIBS(clock_gettime, [rt posix4])
case "$ac_cv_search_clock_gettime" in
-l*) LIBS=$ac_cv_search_clock_gettime;;
esac

#End of line

AC_SUBST(MUSE_MIRROR_CFLAGS)
AC_SUBST(MUSE_MIRROR_LIBS)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_RESULT([
Configuration summary:

  Target ...................... $host
  Target ...................... $host
  Debug output ................ $debug_output
  Profiling support ........... $profile

  libkerat .................... $enable_libkerat
  MUSE ........................ $enable_muse

])
