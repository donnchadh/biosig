####### Makefile for "BioSig for C/C++" #####################
###
###  $Id: Makefile 2600 2011-02-03 00:13:06Z schloegl $
###  Copyright (C) 2006,2007,2008,2009 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository
###  (biosig4c++) at http://biosig.sf.net/
###
##############################################################


PathToMinGW   = $(HOME)/src/mingw-cross-env
PathToMinGW64 = $(HOME)/src/mce-w64
### modify directories according to your needs 
INCOCTAVE = -I /usr/include/octave 
MATLABDIR = /usr/local/MATLAB/R2010b/
# comment the following line if you use MATLAB on 32-bit operating system
MEX_OPTION += -largeArrayDims
# LIBEXT. Setting it to 'a' links statically, 'so' links dynamically
LIBEXT        = a
#LIBEXT	      = so
# 
# WIN32 settings: 
# Mingw crosscompiler: available at http://www.nongnu.org/mingw-cross-env/
# whether dynamic or static linking is used, can be controlled with 
CROSS   = $(PathToMinGW)/usr/bin/i686-pc-mingw32-
CROSS64 = $(PathToMinGW64)/usr/bin/x86_64-w64-mingw32-
# include directory for Win32-Matlab include
W32MAT_INC = $(HOME)/bin/win32/Matlab/R2010b/extern/include/
W64MAT_INC = $(HOME)/bin/win64/Matlab/R2010b/extern/include/
# path to GNUMEX libraries, available from here http://sourceforge.net/projects/gnumex/
GNUMEX   = $(HOME)/bin/win32/gnumex
GNUMEX64 = $(HOME)/bin/win64/gnumex
###############################


### User-specified options: its likely you want to change this
MEX_OPTION    = -largeArrayDims # turn on for 64 bit Matlab, otherwise empty
LIBS          = -lbiosig -lz -lcholmod
LFLAGS        = -L..
COPY          = cp

mex: mex4m mex4o 
win32 win mexw32: mexSLOAD.mexw32 mexSOPEN.mexw32 
win64 mexw64: mexSLOAD.mexw64 mexSOPEN.mexw64 
all: mex4m mex4o win32 win64

clean:
	-$(RM) *.o *.obj *.o64 core octave-core *.oct *~ *.mex* 

##########################################################
## set Matlab and Octave variables
ifneq ($(shell which matlab), )
  MATLABDIR  := $(dir $(shell readlink -f $(shell which matlab) ) )
  MEX         = $(MATLABDIR)mex
  MEX_EXT    := $(shell $(MATLABDIR)mexext)
endif

# Octave - local install (e.g. compiled from sources) 
OCTAVE_VERSION 	= -3.4.0
INCOCTAVE 	= -I /usr/local/include/octave$(OCTAVE_VERSION)/octave 
# Octave - global install  (e.g. from debian package)
#OCTAVE_VERSION 	= 
#INCOCTAVE 	= -I /usr/include/octave 

OCT           	= mkoctfile$(OCTAVE_VERSION)
##########################################################

../libbiosig.$(LIBEXT):
	make -C .. libbiosig.$(LIBEXT)

../win32/libbiosig.a:
	make -C .. win32/libbiosig.a

#############################################################
#	MEX-Interface for Octave and Matlab
#############################################################
mex: mex4m mex4o
ifneq ($(shell which matlab), )
mex4m: mexSLOAD.$(MEX_EXT) mexSOPEN.$(MEX_EXT)
mexSLOAD: mexSLOAD.$(MEX_EXT) 
mexSOPEN: mexSOPEN.$(MEX_EXT)
mexSLOAD.$(MEX_EXT): mexSLOAD.cpp ../libbiosig.$(LIBEXT) Makefile
	$(MEX) $(MEX_OPTION) $(DEFINES) mexSLOAD.cpp $(LFLAGS) $(LIBS)
	-$(COPY) mexSLOAD.$(MEX_EXT) ../../biosig/t200_FileAccess/
mexSOPEN.$(MEX_EXT): mexSLOAD.cpp ../libbiosig.$(LIBEXT) Makefile
	$(MEX) $(MEX_OPTION) -DmexSOPEN $(DEFINES) mexSLOAD.cpp $(LFLAGS) $(LIBS) -output mexSOPEN
	-$(COPY) mexSOPEN.$(MEX_EXT) ../../biosig/t200_FileAccess/
endif



### MEX-files for Octave
mex4o:  mexSLOAD.mex mexSOPEN.mex
mexSLOAD.mex:  mexSLOAD.cpp ../libbiosig.$(LIBEXT) Makefile
	$(OCT)  $(DEFINES) -v -g --mex mexSLOAD.cpp $(LFLAGS) $(LIBS)
	-$(COPY) mexSLOAD.mex ../../biosig/t200_FileAccess/
mexSOPEN.mex:  mexSLOAD.cpp ../libbiosig.$(LIBEXT) Makefile
	$(OCT) -D=mexSOPEN $(DEFINES) -v -g --mex mexSLOAD.cpp $(LFLAGS) $(LIBS) -o mexSOPEN.mex
	-$(COPY) mexSOPEN.mex ../../biosig/t200_FileAccess/
#physicalunits.mex:  physicalunits.cpp libbiosig.$(LIBEXT)
#	$(OCT) -v -g --mex physicalunits.cpp $(LFLAGS) $(LIBS)

oct:  mexSLOAD.oct
mexSLOAD.oct:  mexSLOAD.cpp ../libbiosig.$(LIBEXT) Makefile
	$(OCT)  $(DEFINES) mexSLOAD.cpp $(LFLAGS) $(LIBS)
	$(OCT) -D=mexSOPEN $(DEFINES) mexSLOAD.cpp $(LFLAGS) $(LIBS) -o mexSOPEN.oct
	-$(COPY) mexSOPEN.oct ../../biosig/t200_FileAccess/
	-$(COPY) mexSOPEN.oct ../../biosig/t200_FileAccess/


#########################################################
#	MATLAB/WIN32
#########################################################
%.obj: %.cpp
	$(CROSS)$(CXX) -c -DMATLAB_MEX_FILE -x c++ -o $@ -I$(W32MAT_INC) -O3 -DMX_COMPAT_32 $< 
%.obj: %.c
	$(CROSS)$(CXX) -c -DMATLAB_MEX_FILE -x c++ -o $@ -I$(W32MAT_INC) -O3 -DMX_COMPAT_32 $< 

mexSOPEN.obj: mexSLOAD.cpp
	$(CROSS)$(CXX) -c -DMATLAB_MEX_FILE -D=mexSOPEN -x c++ -o $@ -I$(W32MAT_INC) -O3 -DMX_COMPAT_32 $< 

%.mexw32: %.obj ../win32/libbiosig.a
	$(CROSS)$(CXX) -shared $(GNUMEX)/mex.def -o $@ -L$(GNUMEX) -s $< -llibmx -llibmex ../win32/libbiosig.a -lz -lcholmod -lws2_32


#########################################################
#	MATLAB/WIN64
#########################################################
%.o64: %.cpp
	$(CROSS64)$(CXX) -c -DMATLAB_MEX_FILE -x c++ -o $@ -I$(W64MAT_INC) -O3 -DMX_COMPAT_64 $< 
%.o64: %.c
	$(CROSS64)$(CXX) -c -DMATLAB_MEX_FILE -x c++ -o $@ -I$(W64MAT_INC) -O3 -DMX_COMPAT_64 $< 

mexSOPEN.o64: mexSLOAD.cpp
	$(CROSS64)$(CXX) -c -DMATLAB_MEX_FILE -D=mexSOPEN -x c++ -o $@ -I$(W64MAT_INC) -O3 -DMX_COMPAT_64 $< 

%.mexw64: %.o64 ../win64/libbiosig.a
	$(CROSS64)$(CXX) -shared $(GNUMEX64)/mex.def -o $@ -L$(GNUMEX64) -s $< -llibmx -llibmex ../win64/libbiosig.a -lz -lcholmod -lws2_32
