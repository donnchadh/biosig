####### Compiler, tools and options

CC            = gcc
DEFINES       = 
CFLAGS        = -O2 -Wall $(DEFINES)
INCPATH       = -I"."
LINK          = gcc
LFLAGS        = -Wl,-subsystem,console  # Windows
LIBS          =
DEL_FILE      = del                     # Windows

####### Files

SOURCES       = biosig.c \
		main.c 
OBJECTS       = biosig.o \
		main.o
TARGET        = biosig.exe              # Windows

####### Build rules

first: all
all: $(TARGET)

$(TARGET):  $(OBJECTS) 
	$(LINK) $(LFLAGS) -o "$(TARGET)" $(OBJECTS) $(LIBS)

clean: 
	-$(DEL_FILE) biosig.o main.o

####### Compile

biosig.o: biosig.c biosig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o biosig.o biosig.c

main.o: main.c biosig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o main.o main.c
