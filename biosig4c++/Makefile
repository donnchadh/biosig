
####### Makefile for "BioSig for C/C++" ######################
###
###  $Id: Makefile,v 1.12 2007-05-24 10:19:12 schloegl Exp $
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
		t230/sopen_hl7aecg.o 

# TARGET        = scp2gdf demo gdf2scp t5.scp t6.scp
              # Windows
TARGET        = save2gdf save2aecg save2scp demo

first: all
all: $(TARGET)

%.o : %.c %.cpp %.h 
	$(CC) -S $(CFLAGS) -c $(.SOURCE)  


####### Compile and link #########

#$(TARGET):  $(OBJECTS) 
#	$(LINK) $(LFLAGS) -o "$(TARGET)" $(OBJECTS) $(LIBS) $(TARGET)

demo:   demo.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) demo.c -o demo 
scp2gdf: scp2gdf.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) scp2gdf.c -o scp2gdf 
gdf2scp: gdf2scp.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) gdf2scp.c -o gdf2scp 
save2gdf: save2gdf.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) save2gdf.c -o save2gdf 
save2aecg: save2aecg.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) save2aecg.c -o save2aecg 
save2scp: save2scp.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) save2scp.c -o save2scp


clean:
	-$(DEL_FILE) *.o
	-$(DEL_FILE) t210/*.o
	-$(DEL_FILE) t220/*.o
	-$(DEL_FILE) t230/*.o
	-$(DEL_FILE) XMLParser/*.o
	-$(DEL_FILE) demo scp2gdf gdf2scp t5.scp t6.scp save2gdf save2aecg save2scp


####### Testing ########

t5.gdf: PFE103.scp scp2gdf 
	./scp2gdf PFE103.scp t5.gdf
t5.scp: t5.gdf gdf2scp
	./gdf2scp t5.gdf t5.scp
t6.scp: PFE103.scp scp2gdf
	./scp2gdf PFE103.scp t6.scp
aecg: aECG/448b.xml save2gdf
	./save2scp  aECG/448b.xml 448b.scp
	./save2gdf  aECG/448b.xml 448b.gdf
	./save2aecg aECG/448b.xml 448b.hl7
