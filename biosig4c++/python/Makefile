####### Makefile for "BioSig for C/C++" #####################
###
###  $Id: Makefile,v 1.72 2009/03/03 11:46:57 schloegl Exp $
###  Copyright (C) 2006,2007,2008,2009 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository 
###  (biosig4c++) at http://biosig.sf.net/ 
###
##############################################################

CXX           = g++
DEFINES       = -D=WITH_ZLIB -D=WITH_CHOLMOD -D=__4HAERTEL__ -D=WITH_FAMOS #-D=WITH_FEF -D=WITH_DICOM #-D=WITH_GDCM #-D=WITH_GSL #-D=WITH_EEPROBE #-D=SOFTCODED_PHYSDIMTABLE # if you have zlib
CXXFLAGS      =  -pipe -fPIC -Wall -O2 -Wextra $(DEFINES) # -I cntopenlib/include/
LIBS          = ../libbiosig.a -lz -lcholmod # t240/libfef.a #-lgdcmDSED -lgsl -lgslcblas -lm	# static
SWIG          = swig
PYTHONVER     = $(shell python -c "import sys; print sys.version[:3]")
DEL_FILE      = rm
COPY          = cp -f

TARGET = biosig4python

first: $(TARGET)

../libbiosig.a:
	make -C .. libbiosig.a

biosig4python : _biosig.so biosig.py
biosig.py swig_wrap.cxx: ../libbiosig.a swig.i 
	$(SWIG) -c++ -python -I/usr/include -o swig_wrap.cxx swig.i

_biosig.so : swig_wrap.cxx 
	$(CXX) -c $(CXXFLAGS) swig_wrap.cxx -o swig_wrap.o -I/usr/include/python$(PYTHONVER) && \
 	$(CXX) -shared swig_wrap.o $(LIBS) -o _biosig.so

testpybdf : /tmp/BDFtestfiles.zip
	unzip -u /tmp/BDFtestfiles.zip -d /tmp && \
	python example.py /tmp/*-256.bdf 256 && \
	python example.py /tmp/*-2048.bdf 2048

/tmp/BDFtestfiles.zip :
	wget -P/tmp http://www.biosemi.com/download/BDFtestfiles.zip


clean:
	-$(DEL_FILE) biosig.py*
	-$(DEL_FILE) swig_wrap.*
	 
