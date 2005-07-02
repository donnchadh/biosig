## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ \
	-I../../../include

lib_LTLIBRARIES	= gmobilab.la

gmobilab_la_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h

gmobilab_la_LDFLAGS	= -module

gmobilab_la_LIBADD = @PACKAGE_LIBS@ $(INTLLIBS) -lspa20a

