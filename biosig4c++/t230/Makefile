####### Compiler, tools and options

CC            = g++
DEFINES       = 
CFLAGS        = -O2 -Wall $(DEFINES)
#INCPATH       = -I"." -I"/usr/include/boost-1_33_1"
INCPATH       = -I"." -I"/usr/include/boost"
LINK          = g++
LFLAGS        = -Wl,-subsystem,console,-L"/usr/local/lib"
LIBS          = -lxqilla -lxerces
DEL_FILE      = rm

####### Files

SOURCES       = sreader.cpp \
		XMLUtil.cpp \
		aecg_parser.cpp 
OBJECTS       = sreader.o \
		XMLUtils.o \
		aecg_parser.o 
TARGET        = sreader

####### Build rules

all: $(TARGET)

$(TARGET):  $(OBJECTS) 
	$(LINK) $(LFLAGS) -o "$(TARGET)" $(OBJECTS) $(LIBS)

clean: 
	-$(DEL_FILE) sreader.o XMLUtils.o aecg_parser.o

####### Compile

sreader.o: sreader.cpp XMLUtils.h aecg_parser.h  
	$(CC) -c $(CFLAGS) $(INCPATH) -o sreader.o sreader.cpp

XMLUtils.o: XMLUtils.cpp XMLUtils.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o XMLUtils.o XMLUtils.cpp

aecg_parser.o: aecg_parser.cpp aecg_parser.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o aecg_parser.o aecg_parser.cpp
