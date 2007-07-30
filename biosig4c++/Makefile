
####### Makefile for "BioSig for C/C++" ######################
###
###  $Id: Makefile,v 1.20 2007-07-30 23:24:14 schloegl Exp $
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

TARGET        = save2gdf demo

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
mex:  mexBiosig.c $(OBJECTS) 
	mkoctfile --mex mexBiosig.c $(OBJECTS)
oct:  octBiosig.c $(OBJECTS) 
	mkoctfile octBiosig.c $(OBJECTS)


clean:
	-$(DEL_FILE) *.o
	-$(DEL_FILE) *.mex
	-$(DEL_FILE) *.oct
	-$(DEL_FILE) t210/*.o
	-$(DEL_FILE) t220/*.o
	-$(DEL_FILE) t230/*.o
	-$(DEL_FILE) XMLParser/*.o
	-$(DEL_FILE) demo t5.scp t6.scp save2gdf

	-$(DEL_FILE) t?.[bge]df* t?.hl7* t?.scp*


####### Testing ########
test: PFE103.scp save2scp save2aecg save2gdf
	./save2gdf     	PFE103.scp t3.gdf
	./save2scp     	PFE103.scp t3.scp
	./save2aecg 	PFE103.scp t3.hl7
	./save2gdf 	t3.gdf t3.gdf.gdf
	./save2gdf 	t3.scp t3.scp.gdf
	./save2gdf 	t3.hl7 t3.hl7.gdf
	./save2scp 	t3.gdf t3.gdf.scp
	./save2scp 	t3.scp t3.scp.scp
	./save2scp 	t3.hl7 t3.hl7.scp
	./save2aecg 	t3.gdf t3.gdf.hl7
	./save2aecg 	t3.scp t3.scp.hl7
	./save2aecg 	t3.hl7 t3.hl7.hl7

test3: PFE103.scp save2gdf
	./save2gdf -f=GDF PFE103.scp t1.gdf
	./save2gdf -f=EDF PFE103.scp t1.edf
	./save2gdf -f=BDF PFE103.scp t1.bdf
	./save2gdf -f=SCP PFE103.scp t1.scp
	./save2gdf -f=HL7 PFE103.scp t1.hl7
	./save2gdf -f=CFWB PFE103.scp t1.cfw

	./save2gdf -f=GDF t1.gdf t2.gdf.gdf
	./save2gdf -f=EDF t1.gdf t2.gdf.edf
	./save2gdf -f=BDF t1.gdf t2.gdf.bdf
	./save2gdf -f=SCP t1.gdf t2.gdf.scp
	./save2gdf -f=HL7 t1.gdf t2.gdf.hl7
	./save2gdf -f=CFWB t1.gdf t2.gdf.cfw
	./save2gdf -f=GDF t1.edf t2.edf.gdf
	./save2gdf -f=EDF t1.edf t2.edf.edf
	./save2gdf -f=BDF t1.edf t2.edf.bdf
	./save2gdf -f=SCP t1.edf t2.edf.scp
	./save2gdf -f=HL7 t1.edf t2.edf.hl7
	./save2gdf -f=CFWB t1.edf t2.edf.cfw
	./save2gdf -f=GDF t1.bdf t2.bdf.gdf
	./save2gdf -f=EDF t1.bdf t2.bdf.edf
	./save2gdf -f=BDF t1.bdf t2.bdf.bdf
	./save2gdf -f=SCP t1.bdf t2.bdf.scp
	./save2gdf -f=HL7 t1.bdf t2.bdf.hl7
	./save2gdf -f=CFWB t1.bdf t2.bdf.cfw
	./save2gdf -f=GDF t1.scp t2.scp.gdf
	./save2gdf -f=EDF t1.scp t2.scp.edf
	./save2gdf -f=BDF t1.scp t2.scp.bdf
	./save2gdf -f=SCP t1.scp t2.scp.scp
	./save2gdf -f=HL7 t1.scp t2.scp.hl7
	./save2gdf -f=CFWB t1.scp t2.scp.cfw
	./save2gdf -f=GDF t1.hl7 t2.hl7.gdf
	./save2gdf -f=EDF t1.hl7 t2.hl7.edf
	./save2gdf -f=BDF t1.hl7 t2.hl7.bdf
	./save2gdf -f=SCP t1.hl7 t2.hl7.scp
	./save2gdf -f=HL7 t1.hl7 t2.hl7.hl7
	./save2gdf -f=CFWB t1.hl7 t2.hl7.cfw
	./save2gdf -f=GDF t1.cfw t2.cfw.gdf
	./save2gdf -f=EDF t1.cfw t2.cfw.edf
	./save2gdf -f=BDF t1.cfw t2.cfw.bdf
	./save2gdf -f=SCP t1.cfw t2.cfw.scp
	./save2gdf -f=HL7 t1.cfw t2.cfw.hl7

