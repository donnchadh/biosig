
####### Compiler, tools and options

#CC            = gcc
CC            = g++
DEFINES       = 
CFLAGS        = -O2 -Wall $(DEFINES)
INCPATH       = -I"."
#LINK          = gcc
LINK          = g++
#LFLAGS        = -Wl,-subsystem,console  # Windows
LFLAGS        = -lm
LIBS          =
#DEL_FILE      = del                     # Windows
DEL_FILE      = rm

####### Files

SOURCES       = biosig.c \
		t210/scp_hl7aecg.cpp \
		t210/scp-decode.cpp \
		demo.c \
		scp2gdf.c 
OBJECTS       = biosig.o \
		t210/scp_hl7aecg.o \
		t210/scp-decode.o \
		scp2gdf.o \
		demo.o 
TARGET        = scp2gdf
              # Windows

####### Build rules

#demo:
#	g++ -o demo biosig.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp demo.c
#scp2gdf:
#	g++ -o scp2gdf biosig.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp scp2gdf.c



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
#biosig.o: biosig.c scp2gdf.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp 
#	$(CC) $(CFLAGS) $(INCPATH) -o biosig.o biosig.c

demo: biosig.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp demo.c
	$(CC) $(CFLAGS) -o demo biosig.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp demo.c
	
scp2gdf: biosig.c scp2gdf.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp 
	$(CC) $(CFLAGS) -o scp2gdf biosig.c t210/scp_hl7aecg.cpp t210/scp-decode.cpp scp2gdf.c
	
	
