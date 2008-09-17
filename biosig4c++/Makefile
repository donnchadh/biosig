####### Makefile for "BioSig for C/C++" ######################
###
###  $Id: Makefile,v 1.58 2008-09-17 20:46:29 schloegl Exp $
###  Copyright (C) 2006,2007,2008 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository 
###  (biosig4c++) at http://biosig.sf.net/ 
###
##############################################################



CC            = $(CXX)
CXX           = g++
MinGWCXX      = i586-mingw32msvc-g++
MEX	      = /opt/MatlabR2008a/bin/mex  # add full path to .../matlab/bin/mex $(MATLABPATH)/bin/mex	
#DEFINES      = -D=WITHOUT_SCP_DECODE
DEFINES       = -D=WITH_ZLIB #-D=WITH_ANS1 #-D=WITH_EEPROBE -D=WITH_ANS1 #-D=SOFTCODED_PHYSDIMTABLE # if you have zlib
CFLAGS        = -pipe -g -fPIC -Wall -O2 -W -D_REENTRANT $(DEFINES) # -I cntopenlib/include/
CXXFLAGS      = -pipe -g -fPIC -Wall -O2 -W -D_REENTRANT $(DEFINES) # -I cntopenlib/include/
AR            = ar crs
INCPATH       = 
#LINK          = gcc
LINK          = g++
MinGWLINK     = i586-mingw32msvc-g++
#LFLAGS        = -Wl,-subsystem,console # Windows
LFLAGS        =
#LIBS          = -lz -L. -lbiosig  	# dynamic
LIBS          = libbiosig.a -lz 	# static
MinGWLIBS     = win32/libbiosig.a 	# static
#DEL_FILE      = del                    # Windows
DEL_FILE      = rm

SWIG          = swig
PYTHONVER     = $(shell python -c "import sys; print sys.version[:3]")
TASN1_DIR     = /home/schloegl/src/libtasn1-1.5/lib/
LIBTASN1      = $(TASN1_DIR).libs/libtasn1.a 
 
####### Output directory

OBJECTS_DIR   = ./
BIN	      = $(DESTDIR)/usr/bin
LIB	      = $(DESTDIR)/usr/lib

####### Files
SOURCES       = biosig.c \
		physicalunits.c \
		XMLParser/tinyxml.cpp \
		XMLParser/tinyxmlparser.cpp \
		XMLParser/tinyxmlerror.cpp \
		XMLParser/tinystr.cpp \
		t210/sopen_scp_read.c \
		t210/scp-decode.cpp \
		t220/sopen_scp_write.c \
		t220/crc4scp.c \
		t230/sopen_hl7aecg.c \
		test0/sandbox.c 
		
OBJECTS       = biosig.o \
		tinyxml.o \
		tinyxmlparser.o \
		tinyxmlerror.o \
		tinystr.o \
		sopen_scp_read.o \
		scp-decode.o \
		sopen_scp_write.o \
		crc4scp.o \
		sopen_hl7aecg.o \
		sandbox.o
		 
		
MinGWOBJECTS  = win32/biosig.obj \
		win32/getlogin_r.obj \
		win32/tinyxml.obj \
		win32/tinyxmlparser.obj \
		win32/tinyxmlerror.obj \
		win32/tinystr.obj \
		win32/sopen_scp_read.obj \
		win32/scp-decode.obj \
		win32/sopen_scp_write.obj \
		win32/crc4scp.obj \
		win32/sopen_hl7aecg.obj \
		win32/sandbox.obj \
		win32/zlib/lib/zdll.lib		

TARGET        = save2gdf save2aecg save2scp libbiosig.a libbiosig.so #sigviewer

first: $(TARGET)
all: $(TARGET) mex4o sigviewer exe biosig4python

####### Implicit rules

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
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"


####### Compile

biosig.o: biosig.c biosig-dev.h biosig.h 
	$(CC) -c $(CFLAGS) $(INCPATH) -o biosig.o biosig.c

physicalunits.o: physicalunits.c biosig.h 
	$(CC) -c $(CFLAGS) $(INCPATH) -o physicalunits.o physicalunits.c

sandbox.o: test0/sandbox.c biosig.h 
	$(CC) -c $(CFLAGS) $(INCPATH) -o sandbox.o test0/sandbox.c

save2gdf.o: save2gdf.c biosig-dev.h biosig.h 
	$(CC) -c $(CFLAGS) $(INCPATH) -o save2gdf.o save2gdf.c

sopen_scp_read.o: t210/sopen_scp_read.c biosig-dev.h \
		biosig.h t210/structures.h t210/types.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o sopen_scp_read.o t210/sopen_scp_read.c

scp-decode.o: t210/scp-decode.cpp biosig-dev.h biosig.h \
		t210/types.h t210/structures.h t210/codes.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o scp-decode.o t210/scp-decode.cpp

crc4scp.o: t220/crc4scp.c 
	$(CC) -c $(CFLAGS) $(INCPATH) -o crc4scp.o t220/crc4scp.c

sopen_scp_write.o: t220/sopen_scp_write.c biosig-dev.h biosig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o sopen_scp_write.o t220/sopen_scp_write.c

sopen_hl7aecg.o: t230/sopen_hl7aecg.c biosig-dev.h biosig.h \
		XMLParser/tinyxml.h XMLParser/tinystr.h XMLParser/Tokenizer.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o sopen_hl7aecg.o t230/sopen_hl7aecg.c

tinystr.o: XMLParser/tinystr.cpp XMLParser/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinystr.o XMLParser/tinystr.cpp

tinyxml.o: XMLParser/tinyxml.cpp XMLParser/tinyxml.h XMLParser/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxml.o XMLParser/tinyxml.cpp

tinyxmlerror.o: XMLParser/tinyxmlerror.cpp XMLParser/tinyxml.h XMLParser/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxmlerror.o XMLParser/tinyxmlerror.cpp

tinyxmlparser.o: XMLParser/tinyxmlparser.cpp XMLParser/tinyxml.h XMLParser/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxmlparser.o XMLParser/tinyxmlparser.cpp


####### Build rules

biosig4python : _biosig.so biosig.py
biosig.py swig_wrap.cxx : $(SOURCES) libbiosig.a swig.i 
	$(SWIG) -c++ -python -I/usr/include -o swig_wrap.cxx swig.i
_biosig.so : swig_wrap.cxx 
	$(CXX) -c $(CXXFLAGS) swig_wrap.cxx -o swig_wrap.o -I/usr/include/python$(PYTHONVER) && \
 	$(CXX) -shared swig_wrap.o $(LIBS) -o _biosig.so
 

#$(TARGET):  libbiosig.a 
#	$(LINK) $(LFLAGS) -o "$(TARGET)" $(LIBS) $(TARGET)

libbiosig.a: $(OBJECTS)
	$(AR) libbiosig.a $(OBJECTS) 
	
libbiosig.so: $(OBJECTS)
	$(CXX) -shared -fPIC -o libbiosig.so $(OBJECTS) 
	#$(CXX) -shared -Wl,-soname,libbiosig.so.1 -o libbiosig.so.1.0.1  $(OBJECTS) 


####### Windows executable

win32/zlib123-dll.zip:
	wget -q -Pwin32 http://www.zlib.net/zlib123-dll.zip

win32/zlib: win32/zlib123-dll.zip
	mkdir -p win32/zlib
	unzip -u win32/zlib123-dll.zip -d win32/zlib

win32/zlib/lib/zdll.lib: win32/zlib
	touch --reference=win32/zlib win32/zlib/lib/zdll.lib

win32/zlib/include/zlib.h: win32/zlib
	touch --reference=win32/zlib win32/zlib/include/zlib.h

win32/biosig.obj: biosig.c biosig-dev.h biosig.h  win32/zlib/include/zlib.h
	$(MinGWCXX) -c $(CFLAGS) $(INCPATH) -o win32/biosig.obj biosig.c

win32/libbiosig.a: $(MinGWOBJECTS)
	i586-mingw32msvc-$(AR) win32/libbiosig.a $(MinGWOBJECTS) 
	
win32/physicalunits.obj: physicalunits.c biosig.h  win32/zlib/include/zlib.h
	$(MinGWCXX) -c $(CFLAGS) $(INCPATH) -o win32/physicalunits.obj physicalunits.c

win32/sandbox.obj: test0/sandbox.c biosig.h  win32/zlib/include/zlib.h
	$(MinGWCXX) -c $(CFLAGS) $(INCPATH) -o win32/sandbox.obj test0/sandbox.c

win32/save2gdf.obj: save2gdf.c biosig-dev.h biosig.h  win32/zlib/include/zlib.h
	$(MinGWCXX) -c $(CFLAGS) $(INCPATH) -o win32/save2gdf.obj save2gdf.c

win32/sopen_scp_read.obj: t210/sopen_scp_read.c biosig-dev.h  win32/zlib/include/zlib.h \
		biosig.h t210/structures.h t210/types.h
	$(MinGWCXX) -c $(CFLAGS) $(INCPATH) -o win32/sopen_scp_read.obj t210/sopen_scp_read.c

win32/scp-decode.obj: t210/scp-decode.cpp biosig-dev.h biosig.h win32/zlib/include/zlib.h \
		t210/types.h t210/structures.h t210/codes.h
	$(MinGWCXX) -c $(CXXFLAGS) $(INCPATH) -o win32/scp-decode.obj t210/scp-decode.cpp

win32/crc4scp.obj: t220/crc4scp.c 
	$(MinGWCXX) -c $(CFLAGS) $(INCPATH) -o win32/crc4scp.obj t220/crc4scp.c

win32/sopen_scp_write.obj: t220/sopen_scp_write.c biosig-dev.h biosig.h win32/zlib/include/zlib.h
	$(MinGWCXX) -c $(CFLAGS) $(INCPATH) -o win32/sopen_scp_write.obj t220/sopen_scp_write.c

win32/sopen_hl7aecg.obj: t230/sopen_hl7aecg.c biosig-dev.h biosig.h win32/zlib/include/zlib.h \
		XMLParser/tinyxml.h XMLParser/tinystr.h XMLParser/Tokenizer.h
	$(MinGWCXX) -c $(CFLAGS) $(INCPATH) -o win32/sopen_hl7aecg.obj t230/sopen_hl7aecg.c

win32/tinystr.obj: XMLParser/tinystr.cpp XMLParser/tinystr.h
	$(MinGWCXX) -c $(CXXFLAGS) $(INCPATH) -o win32/tinystr.obj XMLParser/tinystr.cpp

win32/tinyxml.obj: XMLParser/tinyxml.cpp XMLParser/tinyxml.h XMLParser/tinystr.h
	$(MinGWCXX) -c $(CXXFLAGS) $(INCPATH) -o win32/tinyxml.obj XMLParser/tinyxml.cpp

win32/tinyxmlerror.obj: XMLParser/tinyxmlerror.cpp XMLParser/tinyxml.h XMLParser/tinystr.h
	$(MinGWCXX) -c $(CXXFLAGS) $(INCPATH) -o win32/tinyxmlerror.obj XMLParser/tinyxmlerror.cpp

win32/tinyxmlparser.obj: XMLParser/tinyxmlparser.cpp XMLParser/tinyxml.h XMLParser/tinystr.h
	$(MinGWCXX) -c $(CXXFLAGS) $(INCPATH) -o win32/tinyxmlparser.obj XMLParser/tinyxmlparser.cpp

win32/getlogin_r.obj: win32/getlogin_r.c
	$(MinGWCXX) -c $(CXXFLAGS) $(INCPATH) -o win32/getlogin_r.obj win32/getlogin_r.c

win32/save2gdf.exe: save2gdf.c $(MinGWLIBS)
	$(MinGWCXX) $(CXXFLAGS) save2gdf.c $(LFLAGS) $(MinGWOBJECTS) -lws2_32 -o win32/save2gdf.exe 

win32 exe: win32/save2gdf.exe
 


####### application files 
 
save2gdf: save2gdf.c libbiosig.a libbiosig.so $(LIBTASN1)
	$(CXX) $(CXXFLAGS) save2gdf.c $(LFLAGS) $(LIBS) $(TASN1_DIR).libs/libtasn1.a -o save2gdf 

physicalunits: physicalunits.c libbiosig.a
	$(CXX) $(CXXFLAGS) physicalunits.c $(LFLAGS) $(LIBS) -o physicalunits

win32 exe: win32/save2gdf.exe 

mex: mex4m mex4o
mex4m: mexSLOAD.mexglx mexSOPEN.mexglx
mexSLOAD.mexglx: mexSLOAD.cpp  libbiosig.a libbiosig.so
	$(MEX) mexSLOAD.cpp  $(LIBS) 
mexSOPEN.mexglx: mexSLOAD.cpp  libbiosig.a libbiosig.so
	$(MEX) -DmexSOPEN mexSLOAD.cpp  $(LIBS) -output mexSOPEN

mex4o:  mexSLOAD.mex mexSOPEN.mex 
mexSLOAD.mex:  mexSLOAD.cpp libbiosig.a  libbiosig.so 
	mkoctfile  -v -g --mex mexSLOAD.cpp  $(LIBS) 
mexSOPEN.mex:  mexSLOAD.cpp libbiosig.a  libbiosig.so 
	mkoctfile  -D=mexSOPEN -v -g --mex mexSLOAD.cpp  $(LIBS) -o mexSOPEN.mex 
#physicalunits.mex:  physicalunits.cpp libbiosig.a  libbiosig.so 
#	mkoctfile  -v -g --mex physicalunits.cpp  $(LIBS) 

oct:  mexSLOAD.oct
mexSLOAD.oct:  mexSLOAD.cpp libbiosig.a libbiosig.so
	mkoctfile mexSLOAD.cpp $(LFLAGS) $(LIBS) 

sigviewer: libbiosig.a 
	# symbolic link to latest biosig4c
	rm -rf ../sigviewer/extern/biosig.h
	ln -s ../../biosig4c++/biosig.h ../sigviewer/extern/biosig.h 
	rm ../sigviewer/extern/libbiosig.a
	ln -s ../../biosig4c++/libbiosig.a ../sigviewer/extern/libbiosig.a
	touch ../sigviewer/src/log_dialog.cpp
	sh makeSV.sh
	
docs: 	docs/save2gdf.txt  docs/mexSLOAD.txt
	asciidoc -d manpage docs/save2gdf.txt   
	asciidoc -d manpage docs/mexSLOAD.txt 


# for backward compatibility 
save2scp: save2gdf
save2aecg: save2gdf

distclean:
	-$(DEL_FILE) *.a
	-$(DEL_FILE) *.o
	-$(DEL_FILE) *.so
	-$(DEL_FILE) *.mex*
	-$(DEL_FILE) *.oct
	-$(DEL_FILE) t5.scp t6.scp save2gdf gztest test_scp_decode
	-$(DEL_FILE) t?.[bge]df* t?.hl7* t?.scp* t?.cfw* t?.gd1* t?.*.gz *.fil /tmp/t1.*
	-$(DEL_FILE) swig_wrap.* biosig.py 
	-$(DEL_FILE) QMakefile
	-$(DEL_FILE) win32/*.a
	-$(DEL_FILE) win32/*.obj
	-$(DEL_FILE) win32/*.exe
	-$(DEL_FILE) -rf win32/zlib	

clean:		
	-$(DEL_FILE) *~
	-$(DEL_FILE) *.a
	-$(DEL_FILE) *.o
	-$(DEL_FILE) *.so
	-$(DEL_FILE) *.mex*
	-$(DEL_FILE) *.oct
	-$(DEL_FILE) win32/*.a
	-$(DEL_FILE) win32/*.o
	-$(DEL_FILE) win32/*.obj
	-$(DEL_FILE) win32/*.exe


####### INSTALL ########

install: #save2gdf mexSLOAD.mex
	chmod 0755 	save2gdf
	install -d $(BIN)
	install save2gdf $(BIN)
	install save2aecg $(BIN)
	install save2scp  	$(BIN)
	#install rec2bin  	$(BIN)
	#install bin2rec  	$(BIN)
	#chmod 0755  	../sigviewer/bin/sigviewer
	#mv ../sigviewer/bin/sigviewer $(DESTDIR)/usr/local/bin
	install biosig.h    	$(DESTDIR)/usr/include
	mv libbiosig.so 	$(LIB)
	mv libbiosig.a 		$(LIB)
	mkdir -p  	$(DESTDIR)/usr/share/man/man1
	install doc/*.1  	$(DESTDIR)/usr/share/man/man1
	#install libbiosig.man $(DESTDIR)/usr/share/
	mkdir -p  	$(DESTDIR)/share/octave
	install mexSLOAD.mex $(DESTDIR)/share/octave

remove: 
	rm $(BIN)save2gdf
	rm $(BIN)save2scp
	rm $(BIN)save2aecg
	rm $(BIN)rec2bin
	rm $(BIN)bin2rec
	rm $(BIN)sigviewer
	rm $(DESTDIR)/usr/include/biosig.h
	rm $(LIB)libbiosig.*
	rm $(DESTDIR)/usr/share/man/man1/save2gdf.1
	rm $(DESTDIR)/usr/share/man/man1/mexSLOAD.1
	rm $(DESTDIR)/usr/share/man/man1/sigviewer.1
	rm $(DESTDIR)/usr/share/man/man1/rec2bin.1
	rm $(DESTDIR)/usr/share/man/man1/bin2rec.1
	


####### Testing ########
#	./save2gdf -V8 ~/data/test/tms32/small_test.float32.log	
	./save2gdf -f=TMSi t0.scp t2.log
	cat t2.log	

/tmp/t1.scp: 
	# scp example data sets
	wget  -q -P/tmp http://www.openecg.net/ECGsamples.zip
	wget  -q -P/tmp http://www.openecg.net/ECGsamplesc.zip
	unzip -u /tmp/ECGsamples.zip "scp*.zip" -d /tmp
	unzip -u /tmp/ECGsamplesc.zip "scp*.zip" -d /tmp
	mkdir -p /tmp/scp/high
	mkdir -p /tmp/scp/highc
	mkdir -p /tmp/scp/redred
	mkdir -p /tmp/scp/redredc
	unzip -u /tmp/scp_high.zip -d /tmp/scp/high
	unzip -u /tmp/scp_highc.zip -d /tmp/scp/highc
	unzip -u /tmp/scp_redred.zip -d /tmp/scp/redred
	unzip -u /tmp/scp_redredc.zip -d /tmp/scp/redredc
	rm -rf /tmp/ECGsamples*.zip 
	rm -rf /tmp/scp*.zip 
	cp /tmp/scp/redred/PFE103.scp /tmp/t1.scp
	touch /tmp/t1.scp

/tmp/t1.hl7:
	# HL7aECG example data set
	wget -q -P/tmp http://hl7.org/library/committees/rcrim/annecg/aECG%20Release%201%20Schema%20and%20Example%2Ezip
	unzip -u "/tmp/aECG Release 1 Schema and Example.zip"  -d /tmp
	cp "/tmp/2003-12 Schema/example/Example aECG.xml" /tmp/t1.hl7
	rm -rf "/tmp/aECG Release 1 Schema and Example.zip"
	rm -rf "/tmp/2003-12 Schema"
	touch /tmp/t1.hl7

/tmp/t1.edf:
	# EDF+ example data set
#	wget -q -P/home/schloegl/data/EDF/ http://www.edfplus.info/downloads/files/osas.zip
#	unzip -u "/tmp/osas.zip"  -d /tmp
	unzip -u /home/schloegl/data/EDF/osas.zip  -d /tmp
	cp "/tmp/Osas2002plusQRS.edf" /tmp/t1.edf
	touch /tmp/t1.edf

asc: save2gdf 
	./save2gdf -f=ASCII t0.xxx t1.asc

bin: save2gdf 
	./save2gdf -f=BIN t0.xxx t1.bin

testbin: save2gdf /tmp/t1.edf
	./save2gdf -f=BIN /tmp/t1.edf /tmp/t1.hdr
	./save2gdf -f=BIN /tmp/t1.hdr /tmp/t2.hdr
	./save2gdf -f=GDF /tmp/t2.hdr /tmp/t2.gdf

testscp: save2gdf /tmp/t1.scp
	# test converting SCP data
	./save2gdf -f=HL7 /tmp/t1.scp /tmp/t1.scp.hl7
	./save2gdf -f=GDF /tmp/t1.scp.hl7 /tmp/t1.scp.hl7.gdf
	./save2gdf -f=SCP /tmp/t1.scp.hl7.gdf /tmp/t1.scp.hl7.gdf.scp
	./save2gdf -f=GDF /tmp/t1.scp.hl7.gdf.scp /tmp/t1.scp.hl7.gdf.scp.gdf
	./save2gdf -f=HL7 /tmp/t1.scp.hl7.gdf.scp.gdf /tmp/t1.scp.hl7.gdf.scp.gdf.hl7
	./save2gdf -f=SCP /tmp/t1.scp.hl7.gdf.scp.gdf.hl7 /tmp/t1.scp.hl7.gdf.scp.gdf.hl7.scp
	./save2gdf -f=GDF /tmp/t1.scp /tmp/t1.scp.gdf
	./save2gdf -f=HL7 /tmp/t1.scp.gdf /tmp/t1.scp.gdf.hl7
	./save2gdf -f=SCP /tmp/t1.scp.gdf.hl7 /tmp/t1.scp.gdf.hl7.scp
	./save2gdf -f=HL7 /tmp/t1.scp.gdf.hl7.scp /tmp/t1.scp.gdf.hl7.scp.hl7
	./save2gdf -f=GDF /tmp/t1.scp.gdf.hl7.scp.hl7 /tmp/t1.scp.gdf.hl7.scp.hl7.gdf
	./save2gdf -f=SCP /tmp/t1.scp.gdf.hl7.scp.hl7.gdf /tmp/t1.scp.gdf.hl7.scp.hl7.gdf.scp

testhl7: save2gdf /tmp/t1.hl7
	# test converting HL7aECG data
	./save2gdf -f=GDF /tmp/t1.hl7 /tmp/t1.hl7.gdf
	./save2gdf -f=SCP /tmp/t1.hl7.gdf /tmp/t1.hl7.gdf.scp
	./save2gdf -f=HL7 /tmp/t1.hl7.gdf.scp /tmp/t1.hl7.gdf.scp.hl7
	./save2gdf -f=SCP /tmp/t1.hl7.gdf.scp.hl7 /tmp/t1.hl7.gdf.scp.hl7.scp
	./save2gdf -f=GDF /tmp/t1.hl7.gdf.scp.hl7.scp /tmp/t1.hl7.gdf.scp.hl7.scp.gdf
	./save2gdf -f=HL7 /tmp/t1.hl7.gdf.scp.hl7.scp.gdf /tmp/t1.hl7.gdf.scp.hl7.scp.gdf.hl7
	./save2gdf -f=SCP /tmp/t1.hl7 /tmp/t1.hl7.scp
	./save2gdf -f=GDF /tmp/t1.hl7.scp /tmp/t1.hl7.scp.gdf
	./save2gdf -f=HL7 /tmp/t1.hl7.scp.gdf /tmp/t1.hl7.scp.gdf.hl7
	./save2gdf -f=GDF /tmp/t1.hl7.scp.gdf.hl7 /tmp/t1.hl7.scp.gdf.hl7.gdf
	./save2gdf -f=SCP /tmp/t1.hl7.scp.gdf.hl7.gdf /tmp/t1.hl7.scp.gdf.hl7.gdf.scp
	./save2gdf -f=HL7 /tmp/t1.hl7.scp.gdf.hl7.gdf.scp /tmp/t1.hl7.scp.gdf.hl7.gdf.scp.hl7

test: /tmp/t1.scp save2scp save2aecg save2gdf 
	# biosig4python
	# includes test for on-the-fly compression and decompression 
	./save2gdf  -z  /tmp/t1.scp        /tmp/t1.scp.gdf
	./save2scp  -z 	/tmp/t1.scp        /tmp/t1.scp.scp
	./save2aecg  	/tmp/t1.scp        /tmp/t1.scp.hl7
	./save2gdf 	/tmp/t1.scp.gdf.gz /tmp/t1.scp.gdf.gdf
	./save2gdf 	/tmp/t1.scp.scp.gz /tmp/t1.scp.scp.gdf  
	./save2gdf 	/tmp/t1.scp.hl7    /tmp/t1.scp.hl7.gdf
	./save2scp 	/tmp/t1.scp.gdf.gz /tmp/t1.scp.gdf.scp
	./save2scp 	/tmp/t1.scp.scp.gz /tmp/t1.scp.scp.scp  
	./save2scp 	/tmp/t1.scp.hl7    /tmp/t1.scp.hl7.scp
	./save2aecg 	/tmp/t1.scp.gdf.gz /tmp/t1.scp.gdf.hl7
	./save2aecg	/tmp/t1.scp.scp.gz /tmp/t1.scp.scp.hl7  
	./save2aecg	/tmp/t1.scp.hl7    /tmp/t1.scp.hl7.hl7
	# python test.py

zip: /tmp/t1.scp save2gdf
	# test for on-the-fly compression and decompression 
	# on-the-fly compression of output file 
	./save2gdf -z -f=GDF /tmp/t1.scp t1.gdf
	./save2gdf -z -f=GDF1 /tmp/t1.scp t1.gd1
	./save2gdf -z -f=EDF /tmp/t1.scp t1.edf
	./save2gdf -z -f=BDF /tmp/t1.scp t1.bdf
	./save2gdf -z -f=SCP /tmp/t1.scp t1.scp
	./save2gdf -z -f=HL7 /tmp/t1.scp t1.hl7
	./save2gdf -z -f=CFW /tmp/t1.scp t1.cfw
	./save2gdf -z -f=MFER /tmp/t1.scp t1.mwf

	gzip -c /tmp/t1.scp >/tmp/t1.scp.gz 
	# on-the-fly decompression of input file 
	./save2gdf -f=GDF /tmp/t1.scp.gz t1.gdf
	./save2gdf -f=EDF /tmp/t1.scp.gz t1.edf
	./save2gdf -f=BDF /tmp/t1.scp.gz t1.bdf
	./save2gdf -f=SCP /tmp/t1.scp.gz t1.scp
	./save2gdf -f=HL7 /tmp/t1.scp.gz t1.hl7
	./save2gdf -f=CFW /tmp/t1.scp.gz t1.cfw

test6: /tmp/t1.scp save2gdf
	cp /tmp/t1.scp t0.xxx
test7: /tmp/t1.edf save2gdf
	cp /tmp/t1.edf t0.xxx
	./save2gdf -z -f=GDF1 t0.xxx t1.gd1
	./save2gdf -z -f=GDF t0.xxx t1.gdf
	./save2gdf -z -f=EDF t0.xxx t1.edf
	./save2gdf -z -f=BDF t0.xxx t1.bdf
	./save2gdf -z -f=SCP t0.xxx t1.scp
	./save2gdf -z -f=HL7 t0.xxx t1.hl7   # -z not supported for HL7
	./save2gdf -z -f=CFWB t0.xxx t1.cfw
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
	./save2gdf -f=GDF1 t1.hl7    t2.hl7.gd1
	./save2gdf -f=GDF t1.hl7    t2.hl7.gdf
	./save2gdf -f=EDF t1.hl7    t2.hl7.edf
	./save2gdf -f=BDF t1.hl7    t2.hl7.bdf
	./save2gdf -f=SCP t1.hl7    t2.hl7.scp
	./save2gdf -f=HL7 t1.hl7    t2.hl7.hl7
	./save2gdf -f=CFWB t1.hl7    t2.hl7.cfw
	./save2gdf -f=GDF1 t1.cfw.gz t2.cfw.gd1
	./save2gdf -f=GDF t1.cfw.gz t2.cfw.gdf
	./save2gdf -f=EDF t1.cfw.gz t2.cfw.edf
	./save2gdf -f=BDF t1.cfw.gz t2.cfw.bdf
	./save2gdf -f=SCP t1.cfw.gz t2.cfw.scp
	./save2gdf -f=HL7 t1.cfw.gz t2.cfw.hl7

testpybdf : /tmp/BDFtestfiles.zip
	unzip -u /tmp/BDFtestfiles.zip -d /tmp && \
	python example.py /tmp/*-256.bdf 256 && \
	python example.py /tmp/*-2048.bdf 2048

/tmp/BDFtestfiles.zip :
	wget -P/tmp http://www.biosemi.com/download/BDFtestfiles.zip
