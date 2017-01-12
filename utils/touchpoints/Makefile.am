include $(SAGE_PATH)/config.mk

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS = touchpoints

touchpoints_SOURCES = touchpoints.cpp point_renderer.cpp

touchpoints_LDADD = $(DXT_GLSL_OBJS)

touchpoints_CPPFLAGS =  $(TOUCHPOINTS_CFLAGS) -I$(SAGE_PATH)/include $(QUANTA_CFLAGS) 
#$(DXT_INC)
touchpoints_LDFLAGS = $(TOUCHPOINTS_LIBS) -L$(SAGE_PATH)/lib -lsail -lquanta 
#$(DXT_LIB)
