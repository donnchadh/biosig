
####### Makefile for "BioSig for C/C++" ######################
###
###  $Id: Makefile,v 1.31 2008-01-25 19:26:01 schloegl Exp $
###  Copyright (C) 2006,2007 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository 
###  (biosig4c++) at http://biosig.sf.net/ 
###
##############################################################


CC            = g++
CXX           = g++
#DEFINES      = -D=WITHOUT_SCP_DECODE
DEFINES       =
CFLAGS        = -O2 -Wall $(DEFINES)
INCPATH       =
#LINK          = gcc
LINK          = g++
#LFLAGS        = -Wl,-subsystem,console  # Windows
LFLAGS        = -lz 			# zlib
LIBS          = 
#DEL_FILE      = del                     # Windows
DEL_FILE      = rm


SOURCES       = biosig.c \
		XMLParser/tinyxml.cpp \
		XMLParser/tinyxmlparser.cpp \
		XMLParser/tinyxmlerror.cpp \
		XMLParser/tinystr.cpp \
		t210/sopen_scp_read.c \
		t210/scp-decode.cpp \
		t220/sopen_scp_write.c \
		t220/crc4scp.c  \
		t230/sopen_hl7aecg.c \
		demo.c \

OBJECTS       = biosig.o \
		XMLParser/tinyxml.o \
		XMLParser/tinyxmlparser.o \
		XMLParser/tinyxmlerror.o \
		XMLParser/tinystr.o \
		t210/sopen_scp_read.o \
		t210/scp-decode.o \
		t220/sopen_scp_write.o \
		t220/crc4scp.o \
		t230/sopen_hl7aecg.o

TARGET        = save2gdf demo save2aecg save2scp 

first: all
all: $(TARGET)

biosig.o t210/sopen_scp_read.o t210/scp-decode.o t220/sopen_scp_write.o t230/sopen_hl7aecg.o save2gdf.o: biosig.h

%.o:  %.c %.cpp  %.h 
	$(CXX) -shared -S $(CFLAGS)  -c $(.SOURCE)  

biosig4python _biosig.so: swig.i $(SOURCES)
	swig -c++ -python -includeall -I/usr/include swig.i
	$(CXX) -fpic -c -I/usr/include/python2.4 -I/usr/lib/python2.4/config $(.SOURCE) swig_wrap.cxx
	$(CXX) -shared -lz $(OBJECTS) swig_wrap.o -o _biosig.so


####### Compile and link #########

#$(TARGET):  $(OBJECTS) 
#	$(LINK) $(LFLAGS) -o "$(TARGET)" $(OBJECTS) $(LIBS) $(TARGET)

demo:   demo.c $(OBJECTS)
	$(CXX) $(CFLAGS) $(OBJECTS) $(LFLAGS) demo.c -o demo 
save2gdf: save2gdf.c $(OBJECTS)
	$(CXX) $(CFLAGS) $(OBJECTS) $(LFLAGS) save2gdf.c -o save2gdf 
mex:  mexSLOAD.cpp $(OBJECTS) 
#	mex mexSLOAD.cpp $(OBJECTS) $(LFLAGS)
#	mex mexSLOAD.cpp biosig.o XMLParser/tinyxml.o XMLParser/tinyxmlparser.o XMLParser/tinyxmlerror.o XMLParser/tinystr.o t210/sopen_scp_read.o t210/scp-decode.o t220/sopen_scp_write.o t220/crc4scp.o t230/sopen_hl7aecg.o  -lz
#	mkoctfile --mex mexSLOAD.cpp $(OBJECTS)
#	mkoctfile2.9  $(LFLAGS) -v -g --mex mexSLOAD.cpp $(OBJECTS)
	mkoctfile2.9 -lz -v -g --mex mexSLOAD.cpp biosig.o XMLParser/tinyxml.o XMLParser/tinyxmlparser.o XMLParser/tinyxmlerror.o XMLParser/tinystr.o t210/sopen_scp_read.o t210/scp-decode.o t220/sopen_scp_write.o t220/crc4scp.o t230/sopen_hl7aecg.o
oct:  octBiosig.c $(OBJECTS) 
	mkoctfile2.9 mexSLOAD.cpp $(OBJECTS) $(LFLAGS)

# for backward compatibility 
save2scp: save2gdf
save2aecg: save2gdf



distclean:
	-$(DEL_FILE) *.o
	-$(DEL_FILE) *.mex
	-$(DEL_FILE) *.oct
	-$(DEL_FILE) t210/*.o
	-$(DEL_FILE) t220/*.o
	-$(DEL_FILE) t230/*.o
	-$(DEL_FILE) XMLParser/*.o

clean:
	-$(DEL_FILE) *.o
	-$(DEL_FILE) *.so
	-$(DEL_FILE) *.mex
	-$(DEL_FILE) *.oct
	-$(DEL_FILE) t210/*.o
	-$(DEL_FILE) t220/*.o
	-$(DEL_FILE) t230/*.o
	-$(DEL_FILE) XMLParser/*.o
	-$(DEL_FILE) demo t5.scp t6.scp save2gdf gztest test_scp_decode
	-$(DEL_FILE) t?.[bge]df* t?.hl7* t?.scp* t?.cfw* t?.*.gz *.fil /tmp/t1.*


####### Testing ########

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
	./save2gdf -z -f=EDF /tmp/t1.scp t1.edf
	./save2gdf -z -f=BDF /tmp/t1.scp t1.bdf
	./save2gdf -z -f=SCP /tmp/t1.scp t1.scp
	./save2gdf -z -f=HL7 /tmp/t1.scp t1.hl7
	./save2gdf -z -f=CFW /tmp/t1.scp t1.cfw
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
	./save2gdf -z -f=GDF1 t0.xxx t1.gd1
	./save2gdf -z -f=GDF t0.xxx t1.gdf
	./save2gdf -z -f=EDF t0.xxx t1.edf
	./save2gdf -z -f=BDF t0.xxx t1.bdf
	./save2gdf -z -f=SCP t0.xxx t1.scp
	./save2gdf -z -f=HL7 t0.xxx t1.hl7   # -z not supported for HL7
	./save2gdf -z -f=CFW t0.xxx t1.cfw
	./save2gdf -f=GDF1 t1.gd1.gz t2.gd1.gd1
	./save2gdf -f=GDF t1.gd1.gz t2.gd1.gdf
	./save2gdf -f=EDF t1.gd1.gz t2.gd1.edf
	./save2gdf -f=BDF t1.gd1.gz t2.gd1.bdf
	./save2gdf -f=SCP t1.gd1.gz t2.gd1.scp
	./save2gdf -f=HL7 t1.gd1.gz t2.gd1.hl7
	./save2gdf -f=CFW t1.gd1.gz t2.gd1.cfw
	./save2gdf -f=GDF1 t1.gdf.gz t2.gdf.gd1
	./save2gdf -f=GDF t1.gdf.gz t2.gdf.gdf
	./save2gdf -f=EDF t1.gdf.gz t2.gdf.edf
	./save2gdf -f=BDF t1.gdf.gz t2.gdf.bdf
	./save2gdf -f=SCP t1.gdf.gz t2.gdf.scp
	./save2gdf -f=HL7 t1.gdf.gz t2.gdf.hl7
	./save2gdf -f=CFW t1.gdf.gz t2.gdf.cfw
	./save2gdf -f=GDF1 t1.edf.gz t2.edf.gd1
	./save2gdf -f=GDF t1.edf.gz t2.edf.gdf
	./save2gdf -f=EDF t1.edf.gz t2.edf.edf
	./save2gdf -f=BDF t1.edf.gz t2.edf.bdf
	./save2gdf -f=SCP t1.edf.gz t2.edf.scp
	./save2gdf -f=HL7 t1.edf.gz t2.edf.hl7
	./save2gdf -f=CFW t1.edf.gz t2.edf.cfw
	./save2gdf -f=GDF1 t1.bdf.gz t2.bdf.gd1
	./save2gdf -f=GDF t1.bdf.gz t2.bdf.gdf
	./save2gdf -f=EDF t1.bdf.gz t2.bdf.edf
	./save2gdf -f=BDF t1.bdf.gz t2.bdf.bdf
	./save2gdf -f=SCP t1.bdf.gz t2.bdf.scp
	./save2gdf -f=HL7 t1.bdf.gz t2.bdf.hl7
	./save2gdf -f=CFW t1.bdf.gz t2.bdf.cfw
	./save2gdf -f=GDF1 t1.scp.gz t2.scp.gd1
	./save2gdf -f=GDF t1.scp.gz t2.scp.gdf
	./save2gdf -f=EDF t1.scp.gz t2.scp.edf
	./save2gdf -f=BDF t1.scp.gz t2.scp.bdf
	./save2gdf -f=SCP t1.scp.gz t2.scp.scp
	./save2gdf -f=HL7 t1.scp.gz t2.scp.hl7
	./save2gdf -f=CFW t1.scp.gz t2.scp.cfw
	./save2gdf -f=GDF1 t1.hl7    t2.hl7.gd1
	./save2gdf -f=GDF t1.hl7    t2.hl7.gdf
	./save2gdf -f=EDF t1.hl7    t2.hl7.edf
	./save2gdf -f=BDF t1.hl7    t2.hl7.bdf
	./save2gdf -f=SCP t1.hl7    t2.hl7.scp
	./save2gdf -f=HL7 t1.hl7    t2.hl7.hl7
	./save2gdf -f=CFW t1.hl7    t2.hl7.cfw
	./save2gdf -f=GDF1 t1.cfw.gz t2.cfw.gd1
	./save2gdf -f=GDF t1.cfw.gz t2.cfw.gdf
	./save2gdf -f=EDF t1.cfw.gz t2.cfw.edf
	./save2gdf -f=BDF t1.cfw.gz t2.cfw.bdf
	./save2gdf -f=SCP t1.cfw.gz t2.cfw.scp
	./save2gdf -f=HL7 t1.cfw.gz t2.cfw.hl7

