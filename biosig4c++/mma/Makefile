# Makefile to compile sload.exe for loading data supported by libbiosig into Mathematica
# Copyright (C) 2010, 2011 Alois Schloegl

###########################################
# user-defined variables 
########################################### 
##########################################################
## set Mathematica variables
MLINKDIR  = /usr/local/Wolfram/gridMathematicaServer/7.0/SystemFiles/Links/MathLink/DeveloperKit
ifneq ($(shell which mathematica), )
  MLINKDIR  := $(dir $(shell readlink -f $(shell which mathematica) ) )../SystemFiles/Links/MathLink/DeveloperKit
endif
SYS       = Linux-x86-64# Set this value with the result of evaluating $SystemID
MLLIB     = ML64i3 # ML32i3 # Set this to ML64i3 if using a 64-bit system
#
# Windows
#MLINKDIR  = /usr/local/Wolfram/gridMathematicaServer/7.0/SystemFiles/Links/MathLink/DeveloperKit
SYSwin       = Windows# Set this value with the result of evaluating $SystemID
MLLIBwin     = ml32i3m# Set this to ML64i3 if using a 64-bit system
###########################################

CADDSDIR  = ${MLINKDIR}/${SYS}/CompilerAdditions
INCDIR    = ${CADDSDIR}
LIBDIR    = ${CADDSDIR}

EXTRALIBS = -lm -lpthread -lrt -lstdc++ -lcholmod -lz -L.. -lbiosig# Set these with appropriate libs for your system.

MPREP     = ${CADDSDIR}/mprep

all : sload sload.exe
win win32 : sload.exe

../libbiosig.a:
	make -C .. libbiosig.a

../win32/libbiosig.a: 
	make -C .. win32/libbiosig.a

.c.o :
	${CC} -c -I${INCDIR} $<

sloadtm.c : sload.tm
	${MPREP} $? -o $@

sload : sload.o sloadtm.o ../libbiosig.a
	${CC} sloadtm.o sload.o -L${LIBDIR} -l${MLLIB} ${EXTRALIBS} -o $@


###########################################
#   Windows - cross-compilation 
###########################################


CCwin        = $(HOME)/R/src/mingw-cross-env/usr/bin/i686-pc-mingw32-gcc
CADDSDIRwin  = ${MLINKDIR}/${SYSwin}/CompilerAdditions/mldev32
INCDIRwin    = ${CADDSDIRwin}/include
LIBDIRwin    = ${CADDSDIRwin}/lib

EXTRALIBSwin =  ../win32/libbiosig.a -lgdi32 -lm -lpthread -lstdc++ -lws2_32 -lcholmod -lz  # Set these with appropriate libs for your system.

MPREPwin     = ${CADDSDIRwin}/bin/mprep

sloadtm.win32.c : sload.tm
	wine ${MPREPwin} $? -o $@

sload.obj : sload.c
	${CCwin} -c -I${INCDIRwin} $< -o $@
sloadtm.obj : sloadtm.win32.c
	${CCwin} -c -I${INCDIRwin} $< -o $@

sload.exe : sload.obj sloadtm.obj ../win32/libbiosig.a
	${CCwin} sload.obj sloadtm.obj -static-libgcc -static-libstdc++ -L${LIBDIRwin} -l${MLLIBwin} ${EXTRALIBSwin} -o $@

	
###########################################
clean:
	-$(RM) *.o
	-$(RM) *.obj
	-$(RM) *tm.c
	-$(RM) sload.exe
	-$(RM) sload 
	
