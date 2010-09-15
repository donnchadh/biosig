####### Makefile for "BioSig for C/C++" #####################
###
###  $Id: Makefile 2526 2010-08-27 21:46:28Z schloegl $
###  Copyright (C) 2010 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository
###  (biosig4c++) at http://biosig.sf.net/
###
##############################################################

# More information on SWIG-Ruby interface is available here
# http://www.swig.org/Doc1.3/Ruby.html#Ruby


### INC must be the directory containing ruby.h
INC 	= /usr/lib/ruby/1.8/x86_64-linux/

all:
	swig -c++ -ruby biosig.i
	g++ -fPIC -c biosig_wrap.cxx -I$(INC) 
	g++ -fPIC -shared biosig_wrap.o ../libbiosig.so -o biosig.so
	

clean: 
	-rm *.o
	-rm *.so
	-rm *.c*
	