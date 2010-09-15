####### Makefile for "BioSig for C/C++" #####################
###
###  $Id: Makefile 2526 2010-08-27 21:46:28Z schloegl $
###  Copyright (C) 2010 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository
###  (biosig4c++) at http://biosig.sf.net/
###
##############################################################

# More information on SWIG-Tcl interface is available here
# http://www.swig.org/Doc1.3/Tcl.html#Tcl

all:
	swig -c++ -tcl biosig.i
	gcc -fPIC -c biosig_wrap.cxx -I/usr/include/tcl
	gcc -shared biosig_wrap.o ../libbiosig.so -o biosig.so

clean:
	-rm *.cxx
	-rm *.o 
	-rm *.so
