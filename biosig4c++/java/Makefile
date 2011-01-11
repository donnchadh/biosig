####### Makefile for "BioSig for C/C++" #####################
###
###  $Id: Makefile 2526 2010-08-27 21:46:28Z schloegl $
###  Copyright (C) 2010 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository
###  (biosig4c++) at http://biosig.sf.net/
###
##############################################################

# More information on SWIG-Java interface is available here
# http://www.swig.org/Doc1.3/Java.html

all:
	swig -c++ -java biosig.i
	g++ -fPIC -c biosig_wrap.cxx  -I/usr/lib/gcc/x86_64-linux-gnu/4.4/include/ -I/usr/lib/jvm/java-1.5.0-gcj-4.4/include
	ld -G biosig_wrap.o  -o libbiosig4java.so

clean: 
	-rm *.java
	-rm *.o 
	-rm *.cxx 
	-rm *.so

