####### Makefile for "BioSig for C/C++" #####################
###
###  $Id$
###  Copyright (C) 2006,2007,2008,2009 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository 
###  (biosig4c++) at http://biosig.sf.net/ 
###
##############################################################


## make save2gdf   - makes converter 
## make mex4o      - makes mexSLOAD, mexSOPEN for Octave (requires Octave-headers)
## make mex4m      - makes mexSLOAD, mexSOPEN for Matlab (requires Matlab, mex -setup must be configured)
## make mex        - mex4o and mex4m combined 
## make libbiosig  - makes libbiosig
## make biosig4python - makes python interface (requires Python) 
## make win32      - makes save2gdf.exe,libbiosig.lib,libbiosig.dll, for MSWindows, requires MinGW


CC            = gcc
CXX           = g++
MEX	      = mex   # add full path to .../matlab/bin/mex $(MATLABPATH)/bin/mex	
octVER	      = # default
OCT	      =	mkoctfile$(octVER)
#DEFINES      = -D=WITHOUT_SCP_DECODE
DEFINES       = -D=WITH_ZLIB -D=__4HAERTEL__ -D=WITH_FAMOS -D=WITH_CHOLMOD #-D=WITH_FEF -D=WITH_DICOM #-D=WITH_GDCM #-D=WITH_GSL #-D=WITH_EEPROBE #-D=SOFTCODED_PHYSDIMTABLE # if you have zlib
CFLAGS        =  -pipe -fPIC -Wall -O2 -Wextra $(DEFINES) # -I cntopenlib/include/
CXXFLAGS      =  -pipe -fPIC -Wall -O2 -Wextra $(DEFINES) # -I cntopenlib/include/
AR            = ar crs
INCPATH       = -I /usr/local/include/gdcm-2.0/
#		../gdcm-2.0.10/Source/DataStructureAndEncodingDefinition \
#		../gdcm-2.0.10/Source/InformationObjectDefinition  	\
#		../gdcm-2.0.10/Source/MediaStorageAndFileFormat 	\
#		../gdcm-2.0.10/Source/DataDictionary 			\
#		../gdcm-2.0.10/Source/Common
#LINK          = gcc
LINK          = $(CXX)
#LFLAGS        = -Wl,-subsystem,console # Windows
LFLAGS        = -L /usr/local/lib/
#LIBS          = -lz  -lbiosig  	# dynamic
LIBS          = libbiosig.a -lz -lcholmod # t240/libfef.a #-lgdcmDSED -lgsl -lgslcblas -lm	# static
#LIBS         = libbiosig.a -lws2_32 win32/zlib/lib/zdll.lib	# static, for windows 
#DELETE        = del                    # Windows
DELETE        = rm
COPY          = cp -f
TEMP_DIR      = /tmp/

SWIG          = swig
PYTHONVER     = $(shell python -c "import sys; print sys.version[:3]")

# MinGW Crosscompiler: compile on linux binaries for windows  
MinGWCC      = i586-mingw32msvc-gcc 
MinGWCXX     = i586-mingw32msvc-g++ 
MinGWDEF     = -D=WITH_ZLIB -D=__4HAERTEL__ -D=WITH_FAMOS #-D=WITH_CHOLMOD -D=WITH_FEF -D=WITH_DICOM #-D=WITH_GDCM #-D=WITH_GSL #-D=WITH_EEPROBE #-D=SOFTCODED_PHYSDIMTABLE # if you have zlib
MinGWCFLAGS  = -shared -pipe -fPIC -Wall -O2 -W -D_REENTRANT -D=BUILD_DLL $(MinGWDEF)
#MinGWLIBS   = win32/libbiosig.lib win32/libfef.a win32/zlib/lib/zdll.lib -lws2_32  # static
MinGWLIBS    = win32/libbiosig.lib win32/zlib/lib/zdll.lib -lws2_32 #-lcholmod # stati
#MinGWLIBS   = -Lwin32 -l biosig -l zlib1 -lws2_32  # dynamic
MinGWLINK    = i586-mingw32msvc-g++

####### Output directory

OBJ	      = ./obj
BIN	      = $(DESTDIR)/usr/bin
LIB	      = $(DESTDIR)/usr/lib

####### Files
SOURCES       = biosig.c \
		XMLParser/tinyxml.cpp \
		XMLParser/tinyxmlparser.cpp \
		XMLParser/tinyxmlerror.cpp \
		XMLParser/tinystr.cpp \
		t210/sopen_scp_read.c \
		t210/sopen_famos_read.c \
		t210/sopen_alpha_read.c \
		t210/scp-decode.cpp \
		t220/sopen_scp_write.c \
		t220/crc4scp.c \
		t230/sopen_hl7aecg.c \
		t240/sopen_fef_read.c \
		win32/getlogin_r.c \
		test0/sandbox.c \
		physicalunits.c \
		biosig-network.c \
		save2gdf.c \
		biosig_client.c \
		biosig_server.c
		
OBJECTS       = \
		crc4scp.o \
		biosig.o \
		tinyxml.o \
		tinyxmlparser.o \
		tinyxmlerror.o \
		tinystr.o \
		sopen_scp_read.o \
		sopen_famos_read.o \
		sopen_alpha_read.o \
		sopen_fef_read.o \
		scp-decode.o \
		sopen_scp_write.o \
		sopen_fef_read.o \
		sopen_hl7aecg.o \
		getlogin_r.o \
		biosig-network.o \
		sandbox.o 
		 
		
MinGWOBJECTS  = \
		win32/crc4scp.obj \
		win32/biosig.obj \
		win32/getlogin_r.obj \
		win32/tinyxml.obj \
		win32/tinyxmlparser.obj \
		win32/tinyxmlerror.obj \
		win32/tinystr.obj \
		win32/sopen_scp_read.obj \
		win32/sopen_famos_read.obj \
		win32/sopen_alpha_read.obj \
		win32/scp-decode.obj \
		win32/sopen_scp_write.obj \
		win32/sopen_fef_read.obj \
		win32/sopen_hl7aecg.obj \
		win32/biosig-network.obj \
		win32/sandbox.obj 

TARGET        = save2gdf save2aecg save2scp libbiosig.a mex4o pdp2gdf #libbiosig.so 

first: $(TARGET)
all:   $(TARGET) mex4o biosig4python win32 sigviewer biosig_client biosig_server


#############################################################
#	Compilation: Implicit, default rules
#############################################################

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"
.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"
.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"
.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"
.c.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

%.o: %.c biosig.h biosig-dev.h 
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"
%.o: t210/%.c biosig.h biosig-dev.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"
%.o: t220/%.c biosig.h biosig-dev.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"
%.o: t230/%.c biosig.h biosig-dev.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"
%.o: t240/%.c biosig.h biosig-dev.h
	$(CC) -c $(CFLAGS) -I t240 $(INCPATH) -o "$@" "$<"
%.o: test0/%.c biosig.h biosig-dev.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"
%.o: XMLParser/%.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

win32/%.obj: %.c  biosig-dev.h biosig.h 
	$(MinGWCC) -c $(MinGWCFLAGS) $(INCPATH) -o "$@" "$<"
win32/%.obj: t210/%.c  biosig-dev.h biosig.h 
	$(MinGWCC) -c $(MinGWCFLAGS) $(INCPATH) -o "$@" "$<"
win32/%.obj: t220/%.c  biosig-dev.h biosig.h 
	$(MinGWCC) -c $(MinGWCFLAGS) $(INCPATH) -o "$@" "$<"
win32/%.obj: t230/%.c  biosig-dev.h biosig.h 
	$(MinGWCC) -c $(MinGWCFLAGS) $(INCPATH) -o "$@" "$<"
win32/%.obj: t240/%.c  biosig-dev.h biosig.h 
	$(MinGWCC) -c $(MinGWCFLAGS) -I t240 $(INCPATH) -o "$@" "$<"
win32/%.obj: XMLParser/%.cpp win32/zlib/include/zlib.h
	$(MinGWCXX) -c $(MinGWCXXFLAGS) $(INCPATH) -o "$@" "$<"


#############################################################
#	Compilation: exceptions, explicit rules
#############################################################

sandbox.o: test0/sandbox.c biosig.h 
	$(CXX) -c $(CFLAGS) $(INCPATH) -o sandbox.o test0/sandbox.c

sopen_fef_read.o: t240/sopen_fef_read.c biosig-dev.h biosig.h
	cd t240
	#asn1c -fcompound-names -pdu=SessionArchiveSection -S /home/schloegl/cvs/asn1c/skeletons fef.asn1
	asn1c -fcompound-names -pdu=SessionArchiveSection fef.asn1
	#asn1c -fcompound-names iso11073-10471.asn1
	#asn1c -fcompound-names iso11073-20601.asn1
	cd ..
	make -C t240 -f Makefile.am.sample 
#	$(CC) -c $(CFLAGS) -I t240 $(INCPATH) -o sopen_fef_read.o t240/sopen_fef_read.c

sopen_fef_read.o: t240/sopen_fef_read.c biosig-dev.h biosig.h 
	$(CC) -c $(CFLAGS) -I t240 $(INCPATH) -o sopen_fef_read.o t240/sopen_fef_read.c
	
sopen_hl7aecg.o: t230/sopen_hl7aecg.c biosig-dev.h biosig.h \
		XMLParser/tinyxml.h XMLParser/tinystr.h # XMLParser/Tokenizer.h
	$(CXX) -c $(CFLAGS) $(INCPATH) -o sopen_hl7aecg.o t230/sopen_hl7aecg.c
	
sopen_scp_read.o: t210/sopen_scp_read.c biosig-dev.h biosig.h\
		biosig.h t210/structures.h t210/types.h
	$(CXX) -c $(CFLAGS) $(INCPATH) -o sopen_scp_read.o t210/sopen_scp_read.c

scp-decode.o: t210/scp-decode.cpp biosig-dev.h biosig.h \
		t210/types.h t210/structures.h t210/codes.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o scp-decode.o t210/scp-decode.cpp

biosig-dev.h biosig.h getlogin_r.o: win32/getlogin_r.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o getlogin_r.o win32/getlogin_r.c

#strtod.o: gnulib/strtod.c
#	$(CC) -c $(CFLAGS) $(INCPATH) -o strtod.o gnulib/strtod.c

win32/sandbox.obj: test0/sandbox.c biosig.h  win32/zlib/include/zlib.h
	$(MinGWCXX) -c $(MinGWCFLAGS) $(INCPATH) -o win32/sandbox.obj test0/sandbox.c

win32/sopen_scp_read.obj: t210/sopen_scp_read.c biosig-dev.h  win32/zlib/include/zlib.h \
		biosig.h t210/structures.h t210/types.h
	$(MinGWCXX) -c $(MinGWCFLAGS) $(INCPATH) -o win32/sopen_scp_read.obj t210/sopen_scp_read.c

win32/sopen_alpha_read.obj: t210/sopen_alpha_read.c biosig-dev.h 
	$(MinGWCC) -c $(MinGWCFLAGS) $(INCPATH) -o win32/sopen_alpha_read.obj t210/sopen_alpha_read.c

win32/scp-decode.obj: t210/scp-decode.cpp biosig-dev.h biosig.h win32/zlib/include/zlib.h \
		t210/types.h t210/structures.h t210/codes.h
	$(MinGWCXX) -c $(MinGWCFLAGS) $(INCPATH) -o win32/scp-decode.obj t210/scp-decode.cpp

#win32/sopen_fef_read.obj: t240/sopen_fef_read.c biosig-dev.h biosig.h 
#	$(MinGWCC) -c $(MinGWCFLAGS) -I t240 $(INCPATH) -o win32/sopen_fef_read.obj t240/sopen_fef_read.c
	
win32/sopen_hl7aecg.obj: t230/sopen_hl7aecg.c biosig-dev.h biosig.h win32/zlib/include/zlib.h \
		XMLParser/tinyxml.h XMLParser/tinystr.h # XMLParser/Tokenizer.h
	$(MinGWCXX) -c $(MinGWCFLAGS) $(INCPATH) -o win32/sopen_hl7aecg.obj t230/sopen_hl7aecg.c

win32/getlogin_r.obj: win32/getlogin_r.c
	$(MinGWCC) -c $(MinGWCFLAGS) $(INCPATH) -o win32/getlogin_r.obj win32/getlogin_r.c


#############################################################
#	Python (Linux)
#############################################################

python: 
	make -C python biosig4python	

biosig4python : python/_biosig.so python/biosig.py

python/biosig.py python/swig_wrap.cxx: libbiosig.a python/swig.i 
	$(SWIG) -c++ -python -I/usr/include -o python/swig_wrap.cxx python/swig.i

python/_biosig.so : python/swig_wrap.cxx 
	$(CXX) -c $(CXXFLAGS) python/swig_wrap.cxx -o python/swig_wrap.o -I/usr/include/python$(PYTHONVER) && \
 	$(CXX) -shared python/swig_wrap.o $(LIBS) -o python/_biosig.so
 

#############################################################
# 	Download zlib-dll
#############################################################

win32/zlib123-dll.zip:
	wget -q -Pwin32 http://www.zlib.net/zlib123-dll.zip

win32/zlib win32/zlib/lib/zdll.lib win32/zlib/zlib1.dll win32/zlib/include/zlib.h win32/libzdll.a: win32/zlib123-dll.zip
	mkdir -p win32/zlib
	unzip -u win32/zlib123-dll.zip -d win32/zlib
	$(COPY) win32/zlib/lib/zdll.lib win32/libzdll.a

win32/libzdll.a: win32/zlib/lib/zdll.lib
	$(COPY) win32/zlib/lib/zdll.lib win32/libzdll.a

#############################################################
#	WIN32 - BUILD
#############################################################

win32/libbiosig.lib: $(MinGWOBJECTS)  
	-$(DELETE) win32/libbiosig.lib
	i586-mingw32msvc-$(AR) win32/libbiosig.lib   $(MinGWOBJECTS)  

win32/libfef.a:	
		
win32/libbiosig.dll: $(OBJECTS) win32/zlib/zlib1.dll
	$(MinGWCXX) -shared -fPIC  $(MinGWOBJECTS) win32/zlib/zlib1.dll -lws2_32 -o win32/libbiosig.dll -Wl,--output-def,win32/libbiosig.def,--out-implib,libbiosig.dll
	-$(COPY) win32/libbiosig.dll ../sigviewer/extern/

win32/save2gdf.exe: save2gdf.c win32/libbiosig.lib
	$(MinGWCXX) $(MinGWCXXFLAGS) save2gdf.c $(MinGWOBJECTS) $(MinGWLIBS) -o win32/save2gdf.exe 

win32/pdp2gdf.exe: pdp2gdf.c win32/libbiosig.lib 
	$(MinGWLINK) $(MinGWCFLAGS) -o win32/pdp2gdf.exe pdp2gdf.c win32/libbiosig.lib $(MinGWLIBS)  

win32 exe: win32/save2gdf.exe win32/libbiosig.dll win32/libbiosig.lib 
 

#############################################################
#	GNU/Linux - BUILD 
#############################################################
 
libbiosig.a: $(OBJECTS)
	-$(DELETE) libbiosig.a
	$(AR) libbiosig.a $(OBJECTS) 
	
libbiosig.so: $(OBJECTS)
	$(CXX) -shared -fPIC -o libbiosig.so $(OBJECTS) 
	#$(CXX) -shared -Wl,-soname,libbiosig.so.1 -o libbiosig.so.1.0.1  $(OBJECTS) 

save2gdf: save2gdf.o libbiosig.a #t240/libfef.a #libbiosig.so 
	$(LINK) $(CXXFLAGS) save2gdf.o $(LFLAGS) $(LIBS) -o save2gdf 

pdp2gdf: pdp2gdf.o libbiosig.a #libbiosig.so 
	$(LINK) $(CXXFLAGS) pdp2gdf.o $(LFLAGS) $(LIBS) -o pdp2gdf 

physicalunits: physicalunits.o libbiosig.a
	$(LINK) $(CXXFLAGS) physicalunits.o $(LFLAGS) $(LIBS) -o physicalunits

bscs: biosig_client biosig_server sandbox.o biosig.o 
biosig_client: biosig_client.c libbiosig.a biosig-network.o 
	$(LINK) $(CXXFLAGS) biosig_client.c  biosig-network.o $(LFLAGS) $(LIBS) -o biosig_client

biosig_server: biosig_server.c libbiosig.a biosig-network.o 
	$(LINK) $(CXXFLAGS) biosig_server.c  biosig-network.o $(LFLAGS) $(LIBS) -o biosig_server


#############################################################
#	MEX-files for Octave and Matlab
#############################################################
 
mex: mex4m mex4o
mex4m: mexSLOAD.mexa64 mexSOPEN.mexa64
mexSLOAD: mexSLOAD.mexa64
mexSOPEN: mexSOPEN.mexa64
mexSLOAD.mexa64: mexSLOAD.cpp libbiosig.a
	$(MEX) $(DEFINES) -largeArrayDims mexSLOAD.cpp $(LIBS) 
	-$(COPY) mexSLOAD.mexa64 ../biosig/t200_FileAccess/
mexSOPEN.mexa64: mexSLOAD.cpp libbiosig.a
	$(MEX) -DmexSOPEN $(DEFINES) -largeArrayDims mexSLOAD.cpp $(LIBS) -output mexSOPEN
	-$(COPY) mexSOPEN.mexa64 ../biosig/t200_FileAccess/

### MEX-files for Octave
mex4o:  mexSLOAD.mex mexSOPEN.mex 
mexSLOAD.mex:  mexSLOAD.cpp libbiosig.a 
	$(OCT)  $(DEFINES) -v -g --mex mexSLOAD.cpp $(LIBS) 
	$(COPY) mexSLOAD.mex ../biosig/t200_FileAccess/
mexSOPEN.mex:  mexSLOAD.cpp libbiosig.a 
	$(OCT) -D=mexSOPEN $(DEFINES) -v -g --mex mexSLOAD.cpp $(LIBS) -o mexSOPEN.mex 
	-$(COPY) mexSOPEN.mex ../biosig/t200_FileAccess/
#physicalunits.mex:  physicalunits.cpp libbiosig.a  libbiosig.so 
#	$(OCT) -v -g --mex physicalunits.cpp  $(LIBS) 

oct:  mexSLOAD.oct
mexSLOAD.oct:  mexSLOAD.cpp libbiosig.a
	$(OCT)  $(DEFINES) mexSLOAD.cpp $(LFLAGS) $(LIBS) 
	$(OCT) -D=mexSOPEN $(DEFINES) mexSLOAD.cpp $(LFLAGS) $(LIBS) -o mexSOPEN.oct 
	-$(COPY) mexSOPEN.oct ../biosig/t200_FileAccess/
	-$(COPY) mexSOPEN.oct ../biosig/t200_FileAccess/


#############################################################
#	SigViewer
#############################################################

sigviewer: ../sigviewer/bin/sigviewer 
	make -C ../sigviewer/src  
../sigviewer/bin/sigviewer: libbiosig.a biosig.h
	$(COPY) -lf biosig.h ../sigviewer/extern/
	$(COPY) -lf libbiosig.a ../sigviewer/extern/
	-$(COPY) -lf win32/libbiosig.dll ../sigviewer/extern/
#	$(COPY) -lf t240/libfef.a ../sigviewer/extern/
#	touch ../sigviewer/tmp/basic_header.o
	make -C ../sigviewer/src  
	
docs: 	docs/save2gdf.txt  docs/mexSLOAD.txt
	asciidoc -d manpage docs/save2gdf.txt   
	asciidoc -d manpage docs/mexSLOAD.txt 


# for backward compatibility 
save2scp: save2gdf
save2aecg: save2gdf


#############################################################
#	INSTALL and DE-INSTALL
#############################################################
 
distclean:
	-$(DELETE) *.a
	-$(DELETE) *.o
	-$(DELETE) *.lib
	-$(DELETE) *.so
	-$(DELETE) *.mex*
	-$(DELETE) *.oct
	-$(DELETE) t5.scp t6.scp save2gdf gztest test_scp_decode biosig_server biosig_client
	-$(DELETE) t?.[bge]df* t?.hl7* t?.scp* t?.cfw* t?.gd1* t?.*.gz *.fil $(TEMP_DIR)/t1.*
	-$(DELETE) python/swig_wrap.* python/biosig.py* python/_biosig.so 
	-$(DELETE) QMakefile
	-$(DELETE) win32/*.a
	-$(DELETE) win32/*.lib
	-$(DELETE) win32/*.obj
	-$(DELETE) win32/*.exe
	-$(DELETE) -rf win32/zlib	

clean:		
	-$(DELETE) *~
	-$(DELETE) *.a
	-$(DELETE) *.o
	-$(DELETE) *.so
	-$(DELETE) *.mex*
	-$(DELETE) *.oct
	-$(DELETE) python/biosig.py* _biosig.so
	-$(DELETE) python/swig_wrap.*
	-$(DELETE) win32/*.a
	-$(DELETE) win32/*.lib
	-$(DELETE) win32/*.o
	-$(DELETE) win32/*.obj
	-$(DELETE) win32/*.exe

install_sigviewer: sigviewer
	install ../sigviewer/bin/sigviewer $(BIN)
	install ../biosig/doc/eventcodes.txt $(BIN)

install_octave: mexSLOAD.mex 
	mkdir -p  		$(DESTDIR)/usr/local/share/octave/site-m/
	install mexSLOAD.mex 	$(DESTDIR)/usr/local/share/octave/site-m/
	install mexSOPEN.mex 	$(DESTDIR)/usr/local/share/octave/site-m/

install: save2gdf libbiosig.so libbiosig.a doc/save2gdf.1
	install -d 		$(BIN)
	install save2gdf 	$(BIN)
	install save2aecg 	$(BIN)
	install save2scp  	$(BIN)
	#install rec2bin  	$(BIN)
	#install bin2rec  	$(BIN)
	install biosig.h    	$(DESTDIR)/usr/include
	install libbiosig.so 	$(LIB)
	install libbiosig.a	$(LIB)
	mkdir -p  		$(DESTDIR)/usr/share/man/man1
	install doc/*.1  	$(DESTDIR)/usr/share/man/man1
	#install libbiosig.man $(DESTDIR)/usr/share/
	#make install_sigviewer
	#make install_octave

remove: 
	rm $(BIN)/save2gdf
	rm $(BIN)/save2scp
	rm $(BIN)/save2aecg
	#rm $(BIN)/rec2bin
	#rm $(BIN)/bin2rec
	rm $(BIN)/sigviewer
	rm $(BIN)/eventcodes.txt
	rm $(DESTDIR)/usr/include/biosig.h
	rm $(LIB)/libbiosig.*
	rm $(DESTDIR)/usr/share/man/man1/save2gdf.1
	rm $(DESTDIR)/usr/share/man/man1/mexSLOAD.1
	rm $(DESTDIR)/usr/share/man/man1/sigviewer.1
	#rm $(DESTDIR)/usr/share/man/man1/rec2bin.1
	#rm $(DESTDIR)/usr/share/man/man1/bin2rec.1
	

#############################################################
#	Testing
#############################################################
#	./save2gdf -V8 ~/data/test/tms32/small_test.float32.log	
	./save2gdf -f=TMSi t0.scp t2.log
	cat t2.log	

$(TEMP_DIR)t1.scp: 
	# scp example data sets
	wget  -q -P$(TEMP_DIR) http://www.openecg.net/ECGsamples.zip
	wget  -q -P$(TEMP_DIR) http://www.openecg.net/ECGsamplesc.zip
	unzip -u $(TEMP_DIR)ECGsamples.zip "scp*.zip" -d $(TEMP_DIR)
	unzip -u $(TEMP_DIR)ECGsamplesc.zip "scp*.zip" -d $(TEMP_DIR)
	mkdir -p $(TEMP_DIR)scp/high
	mkdir -p $(TEMP_DIR)scp/highc
	mkdir -p $(TEMP_DIR)scp/redred
	mkdir -p $(TEMP_DIR)scp/redredc
	unzip -u $(TEMP_DIR)scp_high.zip -d $(TEMP_DIR)scp/high
	unzip -u $(TEMP_DIR)scp_highc.zip -d $(TEMP_DIR)scp/highc
	unzip -u $(TEMP_DIR)scp_redred.zip -d $(TEMP_DIR)scp/redred
	unzip -u $(TEMP_DIR)scp_redredc.zip -d $(TEMP_DIR)scp/redredc
	rm -rf $(TEMP_DIR)ECGsamples*.zip 
	rm -rf $(TEMP_DIR)scp*.zip 
	$(COPY) $(TEMP_DIR)scp/redred/PFE103.scp $(TEMP_DIR)t1.scp
	touch $(TEMP_DIR)t1.scp

$(TEMP_DIR)t1.hl7:
	# HL7aECG example data set
	wget -q -P$(TEMP_DIR) http://hl7.org/library/committees/rcrim/annecg/aECG%20Release%201%20Schema%20and%20Example%2Ezip
	unzip -u "$(TEMP_DIR)aECG Release 1 Schema and Example.zip"  -d $(TEMP_DIR)
	$(COPY) "$(TEMP_DIR)2003-12 Schema/example/Example aECG.xml" $(TEMP_DIR)t1.hl7
	rm -rf "$(TEMP_DIR)aECG Release 1 Schema and Example.zip"
	rm -rf "$(TEMP_DIR)2003-12 Schema"
	touch $(TEMP_DIR)t1.hl7

$(TEMP_DIR)t1.edf:
	# EDF+ example data set
	wget -q -P$(TEMP_DIR) http://www.edfplus.info/downloads/files/osas.zip
	unzip -u "$(TEMP_DIR)osas.zip"  -d $(TEMP_DIR)
	cp "$(TEMP_DIR)Osas2002plusQRS.edf" $(TEMP_DIR)t1.edf
	touch $(TEMP_DIR)t1.edf

asc: save2gdf 
	./save2gdf -f=ASCII t0.xxx t1.asc

bin: save2gdf 
	./save2gdf -f=BIN t0.xxx t1.bin

testbin: save2gdf $(TEMP_DIR)t1.edf
	./save2gdf -f=BIN $(TEMP_DIR)t1.edf $(TEMP_DIR)t1.hdr
	./save2gdf -f=BIN $(TEMP_DIR)t1.hdr $(TEMP_DIR)t2.hdr
	./save2gdf -f=GDF $(TEMP_DIR)t2.hdr $(TEMP_DIR)t2.gdf

testscp: save2gdf $(TEMP_DIR)t1.scp
	# test converting SCP data
	./save2gdf -f=HL7 $(TEMP_DIR)/t1.scp $(TEMP_DIR)/t1.scp.hl7
	./save2gdf -f=GDF $(TEMP_DIR)/t1.scp.hl7 $(TEMP_DIR)/t1.scp.hl7.gdf
	./save2gdf -f=SCP $(TEMP_DIR)/t1.scp.hl7.gdf $(TEMP_DIR)/t1.scp.hl7.gdf.scp
	./save2gdf -f=GDF $(TEMP_DIR)/t1.scp.hl7.gdf.scp $(TEMP_DIR)t1.scp.hl7.gdf.scp.gdf
	./save2gdf -f=HL7 $(TEMP_DIR)t1.scp.hl7.gdf.scp.gdf $(TEMP_DIR)t1.scp.hl7.gdf.scp.gdf.hl7
	./save2gdf -f=SCP $(TEMP_DIR)t1.scp.hl7.gdf.scp.gdf.hl7 $(TEMP_DIR)t1.scp.hl7.gdf.scp.gdf.hl7.scp
	./save2gdf -f=GDF $(TEMP_DIR)t1.scp $(TEMP_DIR)t1.scp.gdf
	./save2gdf -f=HL7 $(TEMP_DIR)t1.scp.gdf $(TEMP_DIR)t1.scp.gdf.hl7
	./save2gdf -f=SCP $(TEMP_DIR)t1.scp.gdf.hl7 $(TEMP_DIR)t1.scp.gdf.hl7.scp
	./save2gdf -f=HL7 $(TEMP_DIR)t1.scp.gdf.hl7.scp $(TEMP_DIR)t1.scp.gdf.hl7.scp.hl7
	./save2gdf -f=GDF $(TEMP_DIR)t1.scp.gdf.hl7.scp.hl7 $(TEMP_DIR)t1.scp.gdf.hl7.scp.hl7.gdf
	./save2gdf -f=SCP $(TEMP_DIR)t1.scp.gdf.hl7.scp.hl7.gdf $(TEMP_DIR)t1.scp.gdf.hl7.scp.hl7.gdf.scp

testhl7: save2gdf $(TEMP_DIR)t1.hl7
	# test converting HL7aECG data
	./save2gdf -f=GDF $(TEMP_DIR)t1.hl7 $(TEMP_DIR)t1.hl7.gdf
	./save2gdf -f=SCP $(TEMP_DIR)t1.hl7.gdf $(TEMP_DIR)t1.hl7.gdf.scp
	./save2gdf -f=HL7 $(TEMP_DIR)t1.hl7.gdf.scp $(TEMP_DIR)t1.hl7.gdf.scp.hl7
	./save2gdf -f=SCP $(TEMP_DIR)t1.hl7.gdf.scp.hl7 $(TEMP_DIR)t1.hl7.gdf.scp.hl7.scp
	./save2gdf -f=GDF $(TEMP_DIR)t1.hl7.gdf.scp.hl7.scp $(TEMP_DIR)t1.hl7.gdf.scp.hl7.scp.gdf
	./save2gdf -f=HL7 $(TEMP_DIR)t1.hl7.gdf.scp.hl7.scp.gdf $(TEMP_DIR)t1.hl7.gdf.scp.hl7.scp.gdf.hl7
	./save2gdf -f=SCP $(TEMP_DIR)t1.hl7 $(TEMP_DIR)t1.hl7.scp
	./save2gdf -f=GDF $(TEMP_DIR)t1.hl7.scp $(TEMP_DIR)t1.hl7.scp.gdf
	./save2gdf -f=HL7 $(TEMP_DIR)t1.hl7.scp.gdf $(TEMP_DIR)t1.hl7.scp.gdf.hl7
	./save2gdf -f=GDF $(TEMP_DIR)t1.hl7.scp.gdf.hl7 $(TEMP_DIR)t1.hl7.scp.gdf.hl7.gdf
	./save2gdf -f=SCP $(TEMP_DIR)t1.hl7.scp.gdf.hl7.gdf $(TEMP_DIR)t1.hl7.scp.gdf.hl7.gdf.scp
	./save2gdf -f=HL7 $(TEMP_DIR)t1.hl7.scp.gdf.hl7.gdf.scp $(TEMP_DIR)t1.hl7.scp.gdf.hl7.gdf.scp.hl7

test: $(TEMP_DIR)t1.scp save2scp save2aecg save2gdf 
	# biosig4python
	# includes test for on-the-fly compression and decompression 
	./save2gdf  -z  $(TEMP_DIR)t1.scp        $(TEMP_DIR)t1.scp.gdf
	./save2scp  -z 	$(TEMP_DIR)t1.scp        $(TEMP_DIR)t1.scp.scp
	./save2aecg -z 	$(TEMP_DIR)t1.scp        $(TEMP_DIR)t1.scp.hl7
	./save2gdf 	$(TEMP_DIR)t1.scp.gdf.gz $(TEMP_DIR)t1.scp.gdf.gdf
	./save2gdf 	$(TEMP_DIR)t1.scp.scp.gz $(TEMP_DIR)t1.scp.scp.gdf  
	./save2gdf 	$(TEMP_DIR)t1.scp.hl7.gz $(TEMP_DIR)t1.scp.hl7.gdf
	./save2scp 	$(TEMP_DIR)t1.scp.gdf.gz $(TEMP_DIR)t1.scp.gdf.scp
	./save2scp 	$(TEMP_DIR)t1.scp.scp.gz $(TEMP_DIR)t1.scp.scp.scp  
	./save2scp 	$(TEMP_DIR)t1.scp.hl7.gz $(TEMP_DIR)t1.scp.hl7.scp
	./save2aecg 	$(TEMP_DIR)t1.scp.gdf.gz $(TEMP_DIR)t1.scp.gdf.hl7
	./save2aecg	$(TEMP_DIR)t1.scp.scp.gz $(TEMP_DIR)t1.scp.scp.hl7  
	./save2aecg	$(TEMP_DIR)t1.scp.hl7.gz $(TEMP_DIR)t1.scp.hl7.hl7
	# python test0/test.py

zip: $(TEMP_DIR)t1.scp save2gdf
	# test for on-the-fly compression and decompression 
	# on-the-fly compression of output file 
	./save2gdf -z -f=GDF $(TEMP_DIR)t1.scp t1.gdf
	./save2gdf -z -f=GDF1 $(TEMP_DIR)t1.scp t1.gd1
	./save2gdf -z -f=EDF $(TEMP_DIR)t1.scp t1.edf
	./save2gdf -z -f=BDF $(TEMP_DIR)t1.scp t1.bdf
	./save2gdf -z -f=SCP $(TEMP_DIR)t1.scp t1.scp
	./save2gdf -z -f=CFWB $(TEMP_DIR)t1.scp t1.cfw
	./save2gdf -z -f=MFER $(TEMP_DIR)t1.scp t1.mwf
	./save2gdf -z -f=HL7 $(TEMP_DIR)t1.scp t1.hl7

	gzip -c $(TEMP_DIR)t1.scp >$(TEMP_DIR)t1.scp.gz 
	# on-the-fly decompression of input file 
	./save2gdf -f=GDF $(TEMP_DIR)t1.scp.gz t1.gdf
	./save2gdf -f=EDF $(TEMP_DIR)t1.scp.gz t1.edf
	./save2gdf -f=BDF $(TEMP_DIR)t1.scp.gz t1.bdf
	./save2gdf -f=SCP $(TEMP_DIR)t1.scp.gz t1.scp
	./save2gdf -f=HL7 $(TEMP_DIR)t1.scp.gz t1.hl7
	./save2gdf -f=MFER $(TEMP_DIR)t1.scp.gz t1.mwf
	./save2gdf -f=CFWB $(TEMP_DIR)t1.scp.gz t1.cfw

test6: $(TEMP_DIR)t1.scp save2gdf
	$(COPY) $(TEMP_DIR)t1.scp t0.xxx
	#test7: $(TEMP_DIR)t1.edf save2gdf
	#$(COPY) $(TEMP_DIR)t1.edf t0.xxx
	./save2gdf -z -f=GDF1 t0.xxx t1.gd1
	./save2gdf -z -f=GDF t0.xxx t1.gdf
	./save2gdf -z -f=EDF t0.xxx t1.edf
	./save2gdf -z -f=BDF t0.xxx t1.bdf
	./save2gdf -z -f=SCP t0.xxx t1.scp
	./save2gdf -z -f=HL7 t0.xxx t1.hl7   # -z not supported for HL7
	./save2gdf -z -f=CFWB t0.xxx t1.cfw
	./save2gdf -z -f=MFER t0.xxx t1.mwf
	./save2gdf -z -f=BVA t0.xxx t1.bva
	./save2gdf -f=GDF1 t1.gd1.gz t2.gd1.gd1
	./save2gdf -f=GDF t1.gd1.gz t2.gd1.gdf
	./save2gdf -f=EDF t1.gd1.gz t2.gd1.edf
	./save2gdf -f=BDF t1.gd1.gz t2.gd1.bdf
	./save2gdf -f=SCP t1.gd1.gz t2.gd1.scp
	./save2gdf -f=HL7 t1.gd1.gz t2.gd1.hl7
	./save2gdf -f=CFWB t1.gd1.gz t2.gd1.cfw
	./save2gdf -f=GDF1 t1.gdf.gz t2.gdf.gd1
	./save2gdf -f=GDF t1.gdf.gz t2.gdf.gdf
	./save2gdf -f=EDF t1.gdf.gz t2.gdf.edf
	./save2gdf -f=BDF t1.gdf.gz t2.gdf.bdf
	./save2gdf -f=SCP t1.gdf.gz t2.gdf.scp
	./save2gdf -f=HL7 t1.gdf.gz t2.gdf.hl7
	./save2gdf -f=CFWB t1.gdf.gz t2.gdf.cfw
	./save2gdf -f=GDF1 t1.edf.gz t2.edf.gd1
	./save2gdf -f=GDF t1.edf.gz t2.edf.gdf
	./save2gdf -f=EDF t1.edf.gz t2.edf.edf
	./save2gdf -f=BDF t1.edf.gz t2.edf.bdf
	./save2gdf -f=SCP t1.edf.gz t2.edf.scp
	./save2gdf -f=HL7 t1.edf.gz t2.edf.hl7
	./save2gdf -f=CFWB t1.edf.gz t2.edf.cfw
	./save2gdf -f=GDF1 t1.bdf.gz t2.bdf.gd1
	./save2gdf -f=GDF t1.bdf.gz t2.bdf.gdf
	./save2gdf -f=EDF t1.bdf.gz t2.bdf.edf
	./save2gdf -f=BDF t1.bdf.gz t2.bdf.bdf
	./save2gdf -f=SCP t1.bdf.gz t2.bdf.scp
	./save2gdf -f=HL7 t1.bdf.gz t2.bdf.hl7
	./save2gdf -f=CFWB t1.bdf.gz t2.bdf.cfw
	./save2gdf -f=GDF1 t1.scp.gz t2.scp.gd1
	./save2gdf -f=GDF t1.scp.gz t2.scp.gdf
	./save2gdf -f=EDF t1.scp.gz t2.scp.edf
	./save2gdf -f=BDF t1.scp.gz t2.scp.bdf
#	./save2gdf -f=SCP t1.scp.gz t2.scp.scp
	./save2gdf -f=HL7 t1.scp.gz t2.scp.hl7
	./save2gdf -f=CFWB t1.scp.gz t2.scp.cfw
	./save2gdf -f=GDF1 t1.hl7.gz t2.hl7.gd1
	./save2gdf -f=GDF t1.hl7.gz t2.hl7.gdf
	./save2gdf -f=EDF t1.hl7.gz t2.hl7.edf
	./save2gdf -f=BDF t1.hl7.gz t2.hl7.bdf
	./save2gdf -f=SCP t1.hl7.gz t2.hl7.scp
	./save2gdf -f=HL7 t1.hl7.gz t2.hl7.hl7
	./save2gdf -f=CFWB t1.hl7.gz t2.hl7.cfw
	./save2gdf -f=GDF1 t1.cfw.gz t2.cfw.gd1
	./save2gdf -f=GDF t1.cfw.gz t2.cfw.gdf
	./save2gdf -f=EDF t1.cfw.gz t2.cfw.edf
	./save2gdf -f=BDF t1.cfw.gz t2.cfw.bdf
	./save2gdf -f=SCP t1.cfw.gz t2.cfw.scp
	./save2gdf -f=HL7 t1.cfw.gz t2.cfw.hl7
	./save2gdf -f=GDF1 t1.mwf.gz t2.mwf.gd1
	./save2gdf -f=GDF t1.mwf.gz t2.mwf.gdf
	./save2gdf -f=EDF t1.mwf.gz t2.mwf.edf
	./save2gdf -f=BDF t1.mwf.gz t2.mwf.bdf
	./save2gdf -f=SCP t1.mwf.gz t2.mwf.scp
	./save2gdf -f=HL7 t1.mwf.gz t2.mwf.hl7

testpybdf : $(TEMP_DIR)BDFtestfiles.zip
	unzip -u $(TEMP_DIR)BDFtestfiles.zip -d $(TEMP_DIR) && \
	python python/demo.py $(TEMP_DIR)*-256.bdf
	python python/example.py $(TEMP_DIR)*-256.bdf 256 && \
	python python/example.py $(TEMP_DIR)*-2048.bdf 2048

$(TEMP_DIR)BDFtestfiles.zip :
	wget -P$(TEMP_DIR) http://www.biosemi.com/download/BDFtestfiles.zip
