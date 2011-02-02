# Makefile to compile sload1.exe for loading data supported by libbiosig into Mathematica
# Copyright (C) 2010, 2011 Alois Schloegl

###########################################
# user-defined variables 
########################################### 
MLINKDIR  = /usr/local/Wolfram/gridMathematicaServer/7.0/SystemFiles/Links/MathLink/DeveloperKit
SYS       = Linux-x86-64# Set this value with the result of evaluating $SystemID
MLLIB     = ML64i3 # ML32i3 # Set this to ML64i3 if using a 64-bit system
###########################################


CADDSDIR  = ${MLINKDIR}/${SYS}/CompilerAdditions
INCDIR    = ${CADDSDIR}
LIBDIR    = ${CADDSDIR}

EXTRALIBS = -lm -lpthread -lrt -lstdc++ -lcholmod -lz -L.. -lbiosig# Set these with appropriate libs for your system.

MPREP     = ${CADDSDIR}/mprep

all: sload

win, win32:
	make -f Makefile.win32

.c.o :
	${CC} -c -I${INCDIR} $<

sloadtm.c : sload.tm
	${MPREP} $? -o $@

sload: sload.o sloadtm.o ../libbiosig.a
	${CC} sloadtm.o sload.o -L${LIBDIR} -l${MLLIB} ${EXTRALIBS} -o $@

clean: 
	-rm *.exe
	-rm *.o
	-rm *.obj
	-rm *tm.c
	-rm sload
