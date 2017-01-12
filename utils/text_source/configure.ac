AC_PREREQ([2.66])
AC_INIT([text_source], [0.0.0.1], [396546@mail.muni.cz], [text_source])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([text_source.cpp])

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
#PKG_CHECK_MODULES([text_source], [libkerat, libconfig++, liblo])

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
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_RESULT([
Configuration summary:

  Target ...................... $host
  Debug output ................ $debug_output
  Profiling support ........... $profile

])

#  Realtime DXT (OpenGL) ....... $rtdxt
