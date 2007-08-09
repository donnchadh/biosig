
####### Makefile for "BioSig for C/C++" ######################
###
###  $Id: Makefile,v 1.24 2007-08-09 19:26:30 schloegl Exp $
###  Copyright (C) 2006,2007 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository 
###  (biosig4c++) at http://biosig.sf.net/ 
###
##############################################################

#CC            = gcc
CC            = g++
DEFINES       =
CFLAGS        = -O2 -Wall $(DEFINES)
INCPATH       =
#LINK          = gcc
LINK          = g++
#LFLAGS        = -Wl,-subsystem,console  # Windows
LFLAGS        =
LIBS          =
#DEL_FILE      = del                     # Windows
DEL_FILE      = rm


SOURCES       = biosig.h \
		biosig.c \
		XMLParser/tinyxml.h \
		XMLParser/tinyxml.cpp \
		XMLParser/tinyxmlparser.cpp \
		XMLParser/tinyxmlerror \
		XMLParser/tinystr.h \
		XMLParser/tinystr.cpp \
		XMLParser/Tokenizer.h \
		t210/sopen_scp_read.c \
		t210/scp-decode.cpp \
		t220/sopen_scp_write.c \
		t220/crc4scp.c  \
		t230/sopen_hl7aecg.c \
		scp2gdf.c \
		demo.c \
		gdf2scp.c \
		PFE103.scp

OBJECTS       = biosig.o \
		XMLParser/tinyxml.o \
		XMLParser/tinyxmlparser.o \
		XMLParser/tinyxmlerror.o \
		XMLParser/tinystr.o \
		t210/sopen_scp_read.o \
		t210/scp-decode.o \
		t220/sopen_scp_write.o \
		t220/crc4scp.o \
		t230/sopen_hl7aecg.o \
		/usr/lib/libz.so

TARGET        = save2gdf demo save2aecg save2scp 

first: all
all: $(TARGET)

%.o: %.c %.cpp %.h biosig.h
	$(CC) -S $(CFLAGS) -c $(.SOURCE)  


####### Compile and link #########

#$(TARGET):  $(OBJECTS) 
#	$(LINK) $(LFLAGS) -o "$(TARGET)" $(OBJECTS) $(LIBS) $(TARGET)

demo:   demo.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) demo.c -o demo 
save2gdf: save2gdf.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) save2gdf.c -o save2gdf 
test_scp_decode: test0/test_scp_decode.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) test0/test_scp_decode.c -o test_scp_decode 
gztest: test0/gztest.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) test0/gztest.c -o gztest
mex:  mexBiosig.c $(OBJECTS) 
	mkoctfile --mex mexBiosig.c $(OBJECTS)
oct:  octBiosig.c $(OBJECTS) 
	mkoctfile octBiosig.c $(OBJECTS)

# for backward compatibility 
save2scp: save2gdf
save2aecg: save2gdf


clean:
	-$(DEL_FILE) *.o
	-$(DEL_FILE) *.mex
	-$(DEL_FILE) *.oct
	-$(DEL_FILE) t210/*.o
	-$(DEL_FILE) t220/*.o
	-$(DEL_FILE) t230/*.o
	-$(DEL_FILE) XMLParser/*.o
	-$(DEL_FILE) demo t5.scp t6.scp save2gdf gztest
	-$(DEL_FILE) t?.[bge]df* t?.hl7* t?.scp* t?.cfw* t?.*.gz *.fil


####### Testing ########
test: PFE103.scp save2scp save2aecg save2gdf
#	cp PFE103.scp   t0.xxx
	cp Exa01.xml   t0.xxx
	./save2gdf      t0.xxx t3.gdf
	./save2scp   	t0.xxx t3.scp
	./save2aecg  	t0.xxx t3.hl7
	./save2gdf 	t3.gdf t3.gdf.gdf
	./save2gdf 	t3.scp t3.scp.gdf
	./save2gdf 	t3.hl7 t3.hl7.gdf
	./save2scp 	t3.gdf t3.gdf.scp
	./save2scp 	t3.scp t3.scp.scp
	./save2scp 	t3.hl7 t3.hl7.scp
	./save2aecg 	t3.gdf t3.gdf.hl7
	./save2aecg 	t3.scp t3.scp.hl7
	./save2aecg 	t3.hl7 t3.hl7.hl7
	# test for on-the-fly compression and decompression 
	./save2gdf   -z t0.xxx t4.gdf
	./save2scp   -z t0.xxx t4.scp
	./save2aecg  -z	t0.xxx t4.hl7
	./save2gdf 	t4.gdf.gz t4.gdf.gdf
	./save2gdf 	t4.scp.gz t4.scp.gdf
	./save2scp 	t4.gdf.gz t4.gdf.scp
	./save2scp 	t4.scp.gz t4.scp.scp
	./save2aecg 	t4.gdf.gz t4.gdf.hl7
	./save2aecg 	t4.scp.gz t4.scp.hl7

zip: PFE103.scp save2gdf
	# test for on-the-fly compression and decompression 
	cp PFE103.scp   t0.xxx
	# on-the-fly compression of output file 
	./save2gdf -z -f=GDF t0.xxx t1.gdf
	./save2gdf -z -f=EDF t0.xxx t1.edf
	./save2gdf -z -f=BDF t0.xxx t1.bdf
	./save2gdf -z -f=SCP t0.xxx t1.scp
	./save2gdf -z -f=HL7 t0.xxx t1.hl7
	./save2gdf -z -f=CFW t0.xxx t1.cfw
	gzip t0.xxx
	# on-the-fly decompression of input file 
	./save2gdf -f=GDF t0.xxx.gz t1.gdf
	./save2gdf -f=EDF t0.xxx.gz t1.edf
	./save2gdf -f=BDF t0.xxx.gz t1.bdf
	./save2gdf -f=SCP t0.xxx.gz t1.scp
	./save2gdf -f=HL7 t0.xxx.gz t1.hl7
	./save2gdf -f=CFW t0.xxx.gz t1.cfw

test6: PFE103.scp save2gdf
#	cp PFE103.scp   t0.xxx
	cp Exa01.xml   t0.xxx
	./save2gdf -z -f=GDF t0.xxx t1.gdf
	./save2gdf -z -f=EDF t0.xxx t1.edf
	./save2gdf -z -f=BDF t0.xxx t1.bdf
	./save2gdf -z -f=SCP t0.xxx t1.scp
	./save2gdf -z -f=HL7 t0.xxx t1.hl7   # -z not supported for HL7
	./save2gdf -z -f=CFW t0.xxx t1.cfw
	./save2gdf -f=GDF t1.gdf.gz t2.gdf.gdf
	./save2gdf -f=EDF t1.gdf.gz t2.gdf.edf
	./save2gdf -f=BDF t1.gdf.gz t2.gdf.bdf
	./save2gdf -f=SCP t1.gdf.gz t2.gdf.scp
	./save2gdf -f=HL7 t1.gdf.gz t2.gdf.hl7
	./save2gdf -f=CFW t1.gdf.gz t2.gdf.cfw
	./save2gdf -f=GDF t1.edf.gz t2.edf.gdf
	./save2gdf -f=EDF t1.edf.gz t2.edf.edf
	./save2gdf -f=BDF t1.edf.gz t2.edf.bdf
	./save2gdf -f=SCP t1.edf.gz t2.edf.scp
	./save2gdf -f=HL7 t1.edf.gz t2.edf.hl7
	./save2gdf -f=CFW t1.edf.gz t2.edf.cfw
	./save2gdf -f=GDF t1.bdf.gz t2.bdf.gdf
	./save2gdf -f=EDF t1.bdf.gz t2.bdf.edf
	./save2gdf -f=BDF t1.bdf.gz t2.bdf.bdf
	./save2gdf -f=SCP t1.bdf.gz t2.bdf.scp
	./save2gdf -f=HL7 t1.bdf.gz t2.bdf.hl7
	./save2gdf -f=CFW t1.bdf.gz t2.bdf.cfw
	./save2gdf -f=GDF t1.scp.gz t2.scp.gdf
	./save2gdf -f=EDF t1.scp.gz t2.scp.edf
	./save2gdf -f=BDF t1.scp.gz t2.scp.bdf
	./save2gdf -f=SCP t1.scp.gz t2.scp.scp
	./save2gdf -f=HL7 t1.scp.gz t2.scp.hl7
	./save2gdf -f=CFW t1.scp.gz t2.scp.cfw
	./save2gdf -f=GDF t1.hl7 t2.hl7.gdf
	./save2gdf -f=EDF t1.hl7 t2.hl7.edf
	./save2gdf -f=BDF t1.hl7 t2.hl7.bdf
	./save2gdf -f=SCP t1.hl7 t2.hl7.scp
	./save2gdf -f=HL7 t1.hl7 t2.hl7.hl7
	./save2gdf -f=CFW t1.hl7 t2.hl7.cfw
	./save2gdf -f=GDF t1.cfw.gz t2.cfw.gdf
	./save2gdf -f=EDF t1.cfw.gz t2.cfw.edf
	./save2gdf -f=BDF t1.cfw.gz t2.cfw.bdf
	./save2gdf -f=SCP t1.cfw.gz t2.cfw.scp
	./save2gdf -f=HL7 t1.cfw.gz t2.cfw.hl7

