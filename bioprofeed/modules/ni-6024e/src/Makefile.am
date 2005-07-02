## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ \
	-I../../../include

lib_LTLIBRARIES	= ni6024e.la

ni6024e_la_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h

ni6024e_la_LDFLAGS	= -module

ni6024e_la_LIBADD = @PACKAGE_LIBS@ $(INTLLIBS) -lcomedi

