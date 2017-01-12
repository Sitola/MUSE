AC_PREREQ([2.66])
AC_INIT([touchpoints], [0.0.0.1], [359687@mail.muni.cz], [touchpoints])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([touchpoints.cpp])

MUSE_ROOT=`echo $PWD | sed -r 's%[[/]][[^/]]+[[/]][[^/]]+$%%'`

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

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

TOUCHPOINTS_LIBS=""
TOUCHPOINTS_CFLAGS=""

# clock_gettime
AC_SEARCH_LIBS(clock_gettime, [rt posix4])
case "$ac_cv_search_clock_gettime" in
-l*) LIBS=$ac_cv_search_clock_gettime;;
esac

# Checks for libraries.
PKG_CHECK_MODULES([libkerat], [libkerat], [
        AC_DEFINE([HAVE_LIBKERAT], [1], [Build with support for libkerat])
        TOUCHPOINTS_LIBS+=" $libkerat_LIBS"
        TOUCHPOINTS_CFLAGS+=" $libkerat_CFLAGS"
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
        TOUCHPOINTS_LIBS+=" $muse_LIBS"
        TOUCHPOINTS_CFLAGS+=" $muse_CFLAGS"
    ])
fi

# -------------------------------------------------------------------------------------------------
# FastDXT stuff

AC_ARG_ENABLE(fastdxt, [  --disable-fastdxt        disable FastDXT compression engine])
AC_ARG_WITH(fastdxt,
        [  --with-fastdxt=DIR      specify location of FastDXT],
        [DXT_INC=$withval
         DXT_LIB=$withval],
        [DXT_INC=${MUSE_ROOT}/3rd_party/libs/FastDXT
         DXT_LIB=${MUSE_ROOT}/3rd_party/libs/FastDXT]
)

if test x$enable_fastdxt = x
then
    enable_fastdxt=yes
fi

if test $enable_fastdxt = yes
then
        AC_CHECK_HEADER(${DXT_INC}/libdxt.h, FOUND_DXT_H=yes, FOUND_DXT_H=no)
        AC_CHECK_LIB(dxt, main, FOUND_DXT_L=yes, FOUND_DXT_L=no, -I${DXT_INC} -L${DXT_LIB})

        if test $FOUND_DXT_L = yes -a $FOUND_DXT_H = yes
        then
                AC_DEFINE([HAVE_FASTDXT], [1], [Build with support for FastDXT])
                DXT_INC=-I${DXT_INC}
                DXT_LIB="-L${DXT_LIB} -ldxt"

                TOUCHPOINTS_LIBS+=" $DXT_LIB"
                TOUCHPOINTS_CFLAGS+=" $DXT_INC"
        else
                enable_fastdxt=no
        fi

fi

# -------------------------------------------------------------------------------------------------
#
#GLSL DXT

AC_ARG_ENABLE(glsldxt, [  --disable-glsldxt        disable GLSL DXT compression engine])
AC_ARG_WITH(glsldxt,
        [  --with-glsldxt=DIR      specify location of GLSL DXT],
        [GLSLDXT_ROOT=$withval],
        [GLSLDXT_ROOT=${MUSE_ROOT}/3rd_party/libs/glsl_dxt]
)

if test x$enable_glsldxt = x
then
    enable_glsldxt=yes
fi

if test $enable_glsldxt = yes
then
        AC_CHECK_LIB(GLUT, glutInit)
        AC_CHECK_HEADER(GL/glew.h, FOUND_GLEW_H=yes)
        AC_CHECK_LIB(GLEW, glewIsSupported, FOUND_GLEW_L=yes)
        AC_CHECK_HEADER(GL/glx.h, FOUND_GLX_H=yes)
        AC_CHECK_HEADER(GL/gl.h, FOUND_GL_H=yes)
        AC_CHECK_LIB(GL, glXCreateNewContext, FOUND_GLX_L=yes)

        if test "$FOUND_GLEW_L" = yes -a "$FOUND_GLEW_H" = yes -a "$FOUND_GLX_L" = yes \
                -a "$FOUND_GLX_H" = yes -a "$FOUND_GL_H" = yes \
                -a `expr "$host_os" : ".*darwin.*"` -eq 0 # not mac, just for now
        then
                AC_DEFINE([HAVE_GLSLDXT], [1], [Build with GLSL DXT support])
                DXT_GLSL_OBJS="${GLSLDXT_ROOT}/dxt_encoder.c.o ${GLSLDXT_ROOT}/dxt_common.c.o ${GLSLDXT_ROOT}/dxt_util.c.o"

                TOUCHPOINTS_LIBS+=" -lGLEW -lGL -lglut"
                TOUCHPOINTS_CFLAGS+=" -I${GLSLDXT_ROOT}"

        else
                enable_glsldxt=no
        fi
fi
AC_SUBST(DXT_GLSL_OBJS)

# -------------------------------------------------------------------------------------------------
# SAGE stuff
AC_ARG_ENABLE(sage, [  --enable-sage        enable SAGE])
AC_ARG_WITH(sage,
        [  --with-sage=DIR      specify location of SAGE],
        [SAGE_PATH=$withval],
        [SAGE_PATH=${SAGE_DIRECTORY}]
)

if test x$enable_sage = x
then
    enable_sage=yes
fi

if test $enable_sage = yes
then

        AC_DEFINE([HAVE_SAGE], [1], [Build with support for SAGE (required)])
        AC_CHECK_HEADER(${SAGE_PATH}/include/sail.h, FOUND_SAIL_H=yes, FOUND_SAIL_H=no)
        #AC_CHECK_LIB(sail, [sail::swapBuffer], FOUND_SAIL_L=yes, FOUND_SAIL_L=no)
        #AC_CHECK_FILE ([${SAGE_PATH}/lib/libsail.so], FOUND_SAIL_L=yes, FOUND_SAIL_L=no)

        #if test $FOUND_SAIL_L = yes -a $FOUND_SAIL_H = yes
        if test $FOUND_SAIL_H = yes
        then
                AC_DEFINE([HAVE_SAGE], [1], [Build with support for SAGE])
        else
                enable_sage=no
        fi

        AC_SUBST(SAGE_PATH)
fi

if test $enable_sage = no
then
        AC_MSG_FAILURE([Support for SAGE was not found, yet required])
fi

AC_SUBST(TOUCHPOINTS_LIBS)
AC_SUBST(TOUCHPOINTS_CFLAGS)

#End of line
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_RESULT([
Configuration summary:

  Target ...................... $host
  Debug output ................ $debug_output
  Profiling support ........... $profile

  libkerat .................... $enable_libkerat
  MUSE ........................ $enable_muse
  SAGE ........................ $enable_sage
  FastDXT ..................... $enable_fastdxt
  GLSL DXT .................... $enable_glsldxt
])

#  Realtime DXT (OpenGL) ....... $rtdxt
