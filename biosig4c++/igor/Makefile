# Makefile for XOP interface to Igor 
# This is experimental for testing the use of MinGW for XOP's
#
# Copyright 2010 A. Schloegl


# configure your cross-compiler and path to IGOR XOP toolkit 
CROSS      = i586-mingw32msvc-
CROSS      = $(HOME)/src/mingw-cross-env/usr/bin/i686-pc-mingw32-

# define path to XOP toolkit
#IGORDIR    = $(HOME)/.wine/drive_c/Program\ Files/WaveMetrics/XOP\ Toolkit\ 6/IgorXOPs6
IGORDIR     = $(HOME)/.wine/drive_c/Program\ Files/WaveMetrics/XOP\ Toolkit\ 5/IgorXOPs5

##### start of Makefile:  

XOP1	    = $(IGORDIR)/XOP1/XOP1.c
GBDIR	    = $(IGORDIR)/GBLoadWave

INCDIR      = -I$(IGORDIR)/XOPSupport/
INCDIR     += -I$(IGORDIR)/XOP1/
INCDIR     += -I$(IGORDIR)/GBLoadWave/
INCDIR     += -I.

LIBS	   += $(IGORDIR)/XOPSupport/VC6/XOPSupport.lib
LIBS	   += ../win32/libbiosig.a ../win32/zlib/lib/libz.a -lws2_32  
LIBS	   += -lversion -lmsvcrt -lcomdlg32

CC	   = g++
DEFINES	   = -D=WINIGOR -D=_WINDOWS_
 
all:XOP1.xop XOPas.xop GBLoadWave.xop


GBLoadWaveres.o: $(GBDIR)/GBLoadWave.rc  
	$(CROSS)windres $(DEFINES) $(INCDIR) $(GBDIR)/GBLoadWave.rc -o GBLoadWaveres.o
GBLoadWave.o: $(GBDIR)/GBLoadWave.c  
	$(CROSS)$(CC) -shared -c -o GBLoadWave.o $(DEFINES) $(INCDIR) -I./$(GBDIR)/ $(GBDIR)/GBLoadWave.c   
GBLoadWaveDialog.o: $(GBDIR)/GBLoadWaveDialog.c  
	$(CROSS)$(CC) -shared -c -o GBLoadWaveDialog.o $(DEFINES) $(INCDIR) -I./$(GBDIR)/ $(GBDIR)/GBLoadWaveDialog.c   
GBLoadWaveOperation.o: $(GBDIR)/GBLoadWaveOperation.c
	$(CROSS)$(CC) -shared -c -o GBLoadWaveOperation.o $(DEFINES) $(INCDIR) -I./$(GBDIR)/ $(GBDIR)/GBLoadWaveOperation.c   
GBScalingDialog.o: $(GBDIR)/GBScalingDialog.c  
	$(CROSS)$(CC) -shared -c -o GBScalingDialog.o $(DEFINES) $(INCDIR) -I./$(GBDIR)/ $(GBDIR)/GBScalingDialog.c   
GBLoadWaveDialog.o: $(GBDIR)/GBLoadWaveDialog.c  
	$(CROSS)$(CC) -shared -c -o GBLoadWaveDialog.o $(DEFINES) $(INCDIR) -I./$(GBDIR)/ $(GBDIR)/GBLoadWaveDialog.c   
GBLoadWave.xop: GBLoadWave.o GBLoadWaveres.o GBLoadWaveOperation.o GBLoadWaveDialog.o GBScalingDialog.o XOPDialogsWin.o libIgor.a XOPSupport.o XOPFiles.o
	$(CROSS)$(CC) -shared -o GBLoadWave.xop GBLoadWave.o GBLoadWaveOperation.o GBLoadWaveDialog.o GBScalingDialog.o XOPDialogsWin.o GBLoadWaveres.o XOPSupport.o XOPFiles.o $(LIBS) libIgor.a  

	
xopSLOAD.o:xopSLOAD.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) xopSLOAD.c -o xopSLOAD.o  -c 
#xopSLOAD.xop:libIgor.a XOPSupport.o 
#	$(CROSS)$(CC) -shared -o xopSLOAD.xop $(DEFINES) $(INCDIR) xopSLOAD.c XOPSupport.o $(LIBS) libIgor.a  
xopSLOAD.xop: xopSLOAD.o libIgor.a XOPSupport.o 
	$(CROSS)$(CC) -shared -o xopSLOAD.xop xopSLOAD.o XOPSupport.o $(LIBS) libIgor.a  

	
libIgor.a: $(IGORDIR)/XOPSupport/IGOR.lib
	$(CROSS)reimp -c --as $(CROSS)as --dlltool $(CROSS)dlltool $(IGORDIR)/XOPSupport/IGOR.lib

XOPDialogsWin.o:$(IGORDIR)/XOPSupport/XOPDialogsWin.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) $(IGORDIR)/XOPSupport/XOPDialogsWin.c -o XOPDialogsWin.o -c
XOPSupport.o:$(IGORDIR)/XOPSupport/XOPSupport.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) $(IGORDIR)/XOPSupport/XOPSupport.c -o XOPSupport.o -c
XOPFiles.o:$(IGORDIR)/XOPSupport/XOPFiles.c
	$(CROSS)$(CC) $(DEFINES) $(INCDIR) $(IGORDIR)/XOPSupport/XOPFiles.c -o XOPFiles.o -c


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
	
	
	