## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ \
	-I../../../include

lib_LTLIBRARIES	= testmodule.la

testmodule_la_CPPFLAGS = \
	-Wall

testmodule_la_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h

testmodule_la_LDFLAGS	= -module

testmodule_la_LIBADD = @PACKAGE_LIBS@ $(INTLLIBS)

