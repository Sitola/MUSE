AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS = text_source

text_source_SOURCES = text_source.cpp

text_source_LDADD = ../../tuio/libkerat/libkerat.la
text_source_LDFLAGS = $(LIBKERAT_LIBS) $(LIB_CLOCK_GETTIME)
text_source_DEPENDENCIES = ../../tuio/libkerat/libkerat.la
text_source_CPPFLAGS =  $(TEXT_SOURCE_CFLAGS)


