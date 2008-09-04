TEMPLATE = lib
TARGET = biosig
QMAKE_CC = $(CXX)
INCLUDEPATH += ./ \
               ./t210 \
               ./t220 \
               ./t230 \
               ./XMLParser
LIBS += -lz
CONFIG +=     warn_on \
    staticlib \
    release

HEADERS += *.h \
  t210\*.h \
  XMLParser\*.h  
SOURCES +=  *.c \
  t210\*.c \
  t210\*.cpp \
  t220\*.c \
  t230\*.c \
  test0\sandbox.c \
  XMLParser\*.cpp
