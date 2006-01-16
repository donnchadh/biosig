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
		scp_hl7aecg.c \
		main.c 
OBJECTS       = biosig.o \
		scp_hl7aecg.o \
		main.o
TARGET        = biosig
              # Windows

####### Build rules

first: all
all: $(TARGET)

$(TARGET):  $(OBJECTS) 
	$(LINK) $(LFLAGS) -o "$(TARGET)" $(OBJECTS) $(LIBS)

clean: 
	-$(DEL_FILE) biosig.o scp_hl7aecg.o main.o

####### Compile

biosig.o: biosig.c scp_hl7aecg.c biosig.h 
	$(CC) -c $(CFLAGS) $(INCPATH) -o biosig.o biosig.c

main.o: main.c scp_hl7aecg.c biosig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o main.o main.c
