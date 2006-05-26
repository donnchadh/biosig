
####### Compiler, tools and options

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

#OBJECTS       = biosig.o \ 

TARGET        = scp2gdf demo gdf2scp t5.scp t6.scp
              # Windows


first: all
all: $(TARGET)

####### Compile #########

demo:    biosig.c demo.c biosig.h sopen_hl7aecg.c t210/sopen_scp_read.c t220/sopen_scp_write.c t210/scp-decode.cpp t220/crc4scp.c  
	$(CC) $(CFLAGS) -o demo biosig.c sopen_hl7aecg.c t210/sopen_scp_read.c t210/scp-decode.cpp t220/sopen_scp_write.c t220/crc4scp.c  demo.c
scp2gdf: biosig.c scp2gdf.c biosig.h sopen_hl7aecg.c t210/sopen_scp_read.c t220/sopen_scp_write.c t210/scp-decode.cpp t220/crc4scp.c  
	$(CC) $(CFLAGS) -o scp2gdf  biosig.c sopen_hl7aecg.c t210/sopen_scp_read.c t210/scp-decode.cpp t220/sopen_scp_write.c t220/crc4scp.c  scp2gdf.c
gdf2scp: biosig.c gdf2scp.c biosig.h sopen_hl7aecg.c t210/sopen_scp_read.c t220/sopen_scp_write.c t210/scp-decode.cpp t220/crc4scp.c 
	$(CC) $(CFLAGS) -o gdf2scp biosig.c sopen_hl7aecg.c t210/sopen_scp_read.c t210/scp-decode.cpp t220/sopen_scp_write.c t220/crc4scp.c gdf2scp.c

###	$(CC) $(CFLAGS) -I $(INCPATH) -o biosig.o biosig.c


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
	./scp2gdf PFE103.scp t5.scp
 