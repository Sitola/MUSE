AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS = muse_mirrord muse_mirrorctl

muse_mirrord_SOURCES = mirror.cpp common.cpp
muse_mirrorctl_SOURCES = mirrorctl.cpp common.cpp

muse_mirrord_CPPFLAGS =  $(MUSE_MIRROR_CFLAGS)
muse_mirrord_LDFLAGS = $(MUSE_MIRROR_LIBS)
