
####### Makefile for "BioSig for C/C++" ######################
###
###  $Id: Makefile,v 1.9 2006-05-26 14:13:20 schloegl Exp $
###  Copyright (C) 2006 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository 
###  (biosig4c++) at http://biosig.sf.net/ 
###
##############################################################

#CC            = gcc
CC            = g++
DEFINES       = 
CFLAGS        = -O2 -Wall $(DEFINES)
INCPATH       = 
#'xml2-config --cflags'
#LINK          = gcc
LINK          = g++
#LFLAGS        = -Wl,-subsystem,console  # Windows
LFLAGS        = 
#'xml2-config --cflags'
LIBS          = 
#DEL_FILE      = del                     # Windows
DEL_FILE      = rm


SOURCES       = biosig.h \
		biosig.c \
		sopen_hl7aecg.c \
		t210/sopen_scp_read.c \
		t210/scp-decode.cpp \
		t220/sopen_scp_write.c \
		t220/crc4scp.c  \
		scp2gdf.c \
		demo.c \
		gdf2scp.c \
		PFE103.scp

OBJECTS       = biosig.o \
		sopen_hl7aecg.o \
		t210/sopen_scp_read.o \
		t210/scp-decode.o \
		t220/sopen_scp_write.o \
		t220/crc4scp.o  

TARGET        = scp2gdf demo gdf2scp t5.scp t6.scp
              # Windows


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


clean: 
	-$(DEL_FILE) *.o 
	-$(DEL_FILE) t210/*.o 
	-$(DEL_FILE) t220/*.o 
	-$(DEL_FILE) demo scp2gdf gdf2scp t5.scp t6.scp


####### Testing ########

t5.gdf: PFE103.scp scp2gdf 
	./scp2gdf PFE103.scp t5.gdf
t5.scp: t5.gdf gdf2scp
	./gdf2scp t5.gdf t5.scp
t6.scp: PFE103.scp scp2gdf
	./scp2gdf PFE103.scp t6.scp

 