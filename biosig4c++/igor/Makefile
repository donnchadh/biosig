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
XOP1	    = XOP1.c

INCDIR      = -I$(IGORDIR)/XOPSupport/
INCDIR     += -I$(IGORDIR)/XOP1/

LIBDIR	   += -L $(IGORDIR)/XOPSupport/VC6/
LIBDIR	   += -L $(IGORDIR)/XOPSupport/
LIBDIR 	   += -L .

LIBS	   += $(IGORDIR)/XOPSupport/VC6/XOPSupport.lib
LIBS	   += libIgor.a
LIBS	   += ../win32/libbiosig.a ../win32/zlib/lib/libz.a -lws2_32  
LIBS	   += -lversion -lmsvcrt
 
all:XOPas.xop
	
xopSLOAD.o:xopSLOAD.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) xopSLOAD.c -o xopSLOAD.o  -c 
xopSLOAD.xop:xopSLOAD.o
#	$(CROSS)$(CC) -o xopSLOAD.xop xopSLOAD.o  $(LIBS)
#	$(CROSS)$(CC) "$(IGORDIR)/XOPSupport/IGOR.lib" "$(IGORDIR)/XOPSupport/VC6/XOPSupport.lib" /usr/i586-mingw32msvc/lib/libmsvcrt.a xop1.o
	$(CROSS)$(CC)  -shared  -o xopSLOAD.xop $(DEFINES) $(INCDIR) xopSLOAD.c xopSLOAD.o $(LIBS) 
	

XOPSupport.o:$(IGORDIR)/XOPSupport/XOPSupport.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) $(IGORDIR)/XOPSupport/XOPSupport.c -o XOPSupport.o -c

xop.o:$(XOP1)
#	patch -d $(IGORDIR) -p2 <patchIgorXOPs.diff 
	$(CROSS)$(CC) -shared $(DEFINES) $(INCDIR) $(XOP1) -o xop.o -c
	$(CROSS)windres $(DEFINES) $(INCDIR) $(IGORDIR)/XOP1/XOP1.rc -o XOP1res.o
x:xop.o XOPSupport.o
	$(CROSS)$(CC) -Wl,-T,LinkScript -o xop1.xop xop.o XOP1res.o XOPSupport.o $(LIBS) 

IGOR.a: $(IGORDIR)/XOPSupport/IGOR.lib
	/home/schloegl/src/mingw/utils/reimp/reimp -c --as $(CROSS)as --dlltool $(CROSS)dlltool IGOR.lib
XOPSupport.a: $(IGORDIR)/XOPSupport/VC6/XOPSupport.lib
	/home/schloegl/src/mingw/utils/reimp/reimp -c --as $(CROSS)as --dlltool $(CROSS)dlltool XOPSupport.lib

y: XOPSupport.o	IGOR.a 
	$(CROSS)windres $(DEFINES) $(INCDIR) $(IGORDIR)/XOP1/XOP1.rc -o XOP1res.o
	$(CROSS)$(CC) -shared -o XOP1.xop  $(DEFINES) $(INCDIR) $(XOP1)  XOP1res.o XOPSupport.o  $(LIBS)

XOPas.o:XOPas.c
#	patch -d $(IGORDIR) -p2 <patchIgorXOPs.diff 
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) XOPas.c -o XOPas.o -c
	$(CROSS)windres $(DEFINES) $(INCDIR) $(IGORDIR)/XOP1/XOP1.rc -o XOP1res.o
XOPas.xop:XOPas.o XOPSupport.o
	$(CROSS)$(CC)  -o XOPas.xop XOPas.o XOP1res.o XOPSupport.o $(LIBS) 


clean:
	rm *.o *.xop *.exe
distclean:
	rm *.o *.xop *~
	
	
	