# Makefile for XOP interface to Igor 
# This is experimental for testing the use of MinGW for XOP's
#
# Copyright 2010 A. Schloegl

CC	        = g++
CROSS      = i586-mingw32msvc-
DEFINES	  = -D=WINIGOR -D=_WINDOWS_


#IGORDIR  = $(HOME)/.wine/drive_c/Program\ Files/WaveMetrics/XOP\ Toolkit\ 6/IgorXOPs6
IGORDIR   = $(HOME)/.wine/drive_c/Program\ Files/WaveMetrics/XOP\ Toolkit\ 5/IgorXOPs5
XOP1	    = $(IGORDIR)/XOP1/XOP1.c

INCDIR    = -I$(IGORDIR)/XOPSupport/
INCDIR   += -I$(IGORDIR)/XOP1/

LIBDIR	+= -L $(IGORDIR)/XOPSupport/VC6/
LIBDIR	+= -L $(IGORDIR)/XOPSupport/

LIBS	   += -lversion -lmsvcrt -lws2_32
LIBS	   += $(IGORDIR)/XOPSupport/VC6/XOPSupport.lib
LIBS	   += $(IGORDIR)/XOPSupport/IGOR.lib
LIBS	   += ../win32/libbiosig.a ../win32/zlib/lib/libz.a  

all:x xopSLOAD.xop
	
xopSLOAD.o:xopSLOAD.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) xopSLOAD.c -o xopSLOAD.o -c
xopSLOAD.xop:xopSLOAD.o
	$(CROSS)$(CC)  xopSLOAD.o $(LIBS) -o xopSLOAD.xop
#	$(CROSS)$(CC) "$(IGORDIR)/XOPSupport/IGOR.lib" "$(IGORDIR)/XOPSupport/VC6/XOPSupport.lib" /usr/i586-mingw32msvc/lib/libmsvcrt.a xop1.o
	

XOPSupport.o:$(IGORDIR)/XOPSupport/XOPSupport.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) $(IGORDIR)/XOPSupport/XOPSupport.c -o XOPSupport.o -c

xop.o:$(XOP1)
#	patch -d $(IGORDIR) -p2 <patchIgorXOPs.diff 
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) $(XOP1) -o xop.o -c
	$(CROSS)windres $(DEFINES) $(INCDIR) $(IGORDIR)/XOP1/XOP1.rc -o XOP1res.o
x:xop.o XOPSupport.o
	$(CROSS)$(CC) xop.o XOP1res.o XOPSupport.o $(LIBS) -o xop1.xop 


clean:
	rm *.o *.xop *.exe
distclean:
	rm *.o *.xop *~
	
	
	