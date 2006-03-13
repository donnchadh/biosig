
####### Compiler, tools and options

#CC            = gcc
CC            = g++
DEFINES       = 
CFLAGS        = -O2 -Wall $(DEFINES)
INCPATH       = 'xml2-config --cflags'
#LINK          = gcc
LINK          = g++
#LFLAGS        = -Wl,-subsystem,console  # Windows
LFLAGS        = 'xml2-config --cflags'
LIBS          = 
#DEL_FILE      = del                     # Windows
DEL_FILE      = rm

####### Files

SOURCES       = biosig.c \
		t210/scp_hl7aecg.cpp \
		t210/scp-decode.cpp \
		sopen_hl7aecg.c \
		demo.c \
		scp2gdf.c 
OBJECTS       = biosig.o \
		t210/scp_hl7aecg.o \
		t210/scp-decode.o \
		sopen_hl7aecg.o \
		scp2gdf.o \
		demo.o 
TARGET        = scp2gdf demo
              # Windows

####### Build rules

#demo:
#	g++ -I /usr/include/libxml2/ biosig.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp sopen_hl7aecg.c scp2gdf.c -L/usr/lib -lxml2 -lz -lm -o scp2gdf 
#scp2gdf:
#	g++ -I /usr/include/libxml2/ biosig.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp sopen_hl7aecg.c demo.c -L/usr/lib -lxml2 -lz -lm -o demo



first: all
all: $(TARGET)

$(TARGET):  $(OBJECTS) 
	$(LINK) $(LFLAGS) -o "$(TARGET)" $(OBJECTS) $(LIBS)

clean: 
	-$(DEL_FILE) *.o 
	-$(DEL_FILE) t210/*.o 
	-$(DEL_FILE) demo scp2gdf

####### Compile

#biosig.o: biosig.c scp_hl7aecg.c biosig.h 
#biosig.o: biosig.c scp2gdf.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp  sopen_hl7aecg.c 
#	$(CC) $(CFLAGS) -I $(INCPATH) -o biosig.o biosig.c
	
	
