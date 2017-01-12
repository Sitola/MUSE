#AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4
#include aminclude.am

libexec_tools_PROGRAMS = stdout static-source
libexec_toolsdir = $(libexecdir)/$(PACKAGE_NAME)

stdout_SOURCES = stdout.cpp
stdout_LDADD = ../libkerat.la
stdout_LDFLAGS = $(LIBKERAT_LIBS) $(LIB_CLOCK_GETTIME)
stdout_DEPENDENCIES = ../libkerat.la

static_source_SOURCES = static_source.cpp
static_source_LDADD = ../libkerat.la
static_source_LDFLAGS = $(LIBKERAT_LIBS) $(LIB_CLOCK_GETTIME)
static_source_DEPENDENCIES = ../libkerat.la


