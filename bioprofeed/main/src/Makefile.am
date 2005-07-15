## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ \
	-I../../include

bin_PROGRAMS = bsview

bsview_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	pref.c pref.h \
	dataaq.c dataaq.h \
	gdffile.c gdffile.h \
	draw.c draw.h \
	squeue.c squeue.h

bsview_CPPFLAGS	= \
	-Wall

bsview_LDFLAGS	= \
	-dlopen self

bsview_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) \
	-lltdl -lgthread-2.0

