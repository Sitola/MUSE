#AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4
#include aminclude.am

bin_PROGRAMS = libreco_template_gen

libreco_template_gen_SOURCES = template_gen.cpp
libreco_template_gen_LDADD = ../libmuse_recognizers.la
libreco_template_gen_CFLAGS = $(CHECK_CFLAGS)
libreco_template_gen_LDFLAGS = $(MUSE_RECOGNIZERS_LIBS)
libreco_template_gen_DEPENDENCIES = ../libmuse_recognizers.la
