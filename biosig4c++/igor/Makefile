# Makefile for XOP interface to Igor 
# This is experimental for testing the use of MinGW for XOP's
#
# Copyright 2010 A. Schloegl

CC	   = g++
CROSS      = i586-mingw32msvc-
CROSS      = /home/schloegl/src/mingw-cross-env/usr/bin/i686-pc-mingw32-
DEFINES	   = -D=WINIGOR -D=_WINDOWS_


#IGORDIR    = $(HOME)/.wine/drive_c/Program\ Files/WaveMetrics/XOP\ Toolkit\ 6/IgorXOPs6
IGORDIR     = $(HOME)/.wine/drive_c/Program\ Files/WaveMetrics/XOP\ Toolkit\ 5/IgorXOPs5
XOP1	    = $(IGORDIR)/XOP1/XOP1.c

INCDIR      = -I$(IGORDIR)/XOPSupport/
INCDIR     += -I$(IGORDIR)/XOP1/

LIBS	   += $(IGORDIR)/XOPSupport/VC6/XOPSupport.lib
LIBS	   += ../win32/libbiosig.a ../win32/zlib/lib/libz.a -lws2_32  
LIBS	   += -lversion -lmsvcrt
 
all:XOP1.xop

	
xopSLOAD.o:xopSLOAD.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) xopSLOAD.c -o xopSLOAD.o  -c 
xopSLOAD.xop:xopSLOAD.o libIgor.a
#	$(CROSS)$(CC) -o xopSLOAD.xop xopSLOAD.o  $(LIBS)
#	$(CROSS)$(CC) "$(IGORDIR)/XOPSupport/IGOR.lib" "$(IGORDIR)/XOPSupport/VC6/XOPSupport.lib" /usr/i586-mingw32msvc/lib/libmsvcrt.a xop1.o
	$(CROSS)$(CC)  -shared  -o xopSLOAD.xop $(DEFINES) $(INCDIR) xopSLOAD.c xopSLOAD.o $(LIBS) libIgor.a  

	
libIgor.a: $(IGORDIR)/XOPSupport/IGOR.lib
	$(CROSS)reimp -c --as $(CROSS)as --dlltool $(CROSS)dlltool $(IGORDIR)/XOPSupport/IGOR.lib

XOPSupport.o:$(IGORDIR)/XOPSupport/XOPSupport.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) $(IGORDIR)/XOPSupport/XOPSupport.c -o XOPSupport.o -c


## compiling XOP1: this should work
XOP1res.o:$(IGORDIR)/XOP1/XOP1.rc
	$(CROSS)windres $(DEFINES) $(INCDIR) $(IGORDIR)/XOP1/XOP1.rc -o XOP1res.o
XOP1.xop: XOPSupport.o libIgor.a XOP1res.o 
	$(CROSS)$(CC) -shared -o XOP1.xop  $(DEFINES) $(INCDIR) $(XOP1) XOP1res.o XOPSupport.o $(LIBS) libIgor.a 


## testing a modified XOP1
XOPasres.o:XOPas.rc
	$(CROSS)windres $(DEFINES) $(INCDIR) XOPas.rc -o XOPasres.o
XOPas.o:XOPas.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) XOPas.c -o XOPas.o -c
XOPas.xop:XOPas.o XOPSupport.o libIgor.a  XOPasres.o
	$(CROSS)$(CC) -shared -o XOPas.xop XOPas.o XOPasres.o XOPSupport.o $(LIBS) libIgor.a 


clean:
	rm *.o *.xop *.exe
distclean:
	rm *.o *.xop *~
	
	
	