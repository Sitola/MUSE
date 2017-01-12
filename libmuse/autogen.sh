#!/bin/sh
srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`

libtoolize --copy && \
autoheader && \
aclocal -I m4 && \
automake --copy --add-missing && \
autoconf && \
./configure "$@"

STATUS=$?

cd $ORIGDIR

([ $STATUS -eq 0 ] && echo "Autogen done." ) || echo "Autogen failed."

exit $STATUS

