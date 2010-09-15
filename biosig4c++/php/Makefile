####### Makefile for "BioSig for C/C++" #####################
###
###  $Id: Makefile 2526 2010-08-27 21:46:28Z schloegl $
###  Copyright (C) 2010 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository
###  (biosig4c++) at http://biosig.sf.net/
###
##############################################################

# More information on SWIG-PHP interface is available here
# http://www.swig.org/Doc1.3/Php.html#Php

all:
	swig -c++ -php biosig.i
	gcc `php-config --includes` -fpic -c biosig_wrap.cpp
	gcc -shared biosig_wrap.o -o php_biosig.so

clean:
	-rm *.h 
	-rm *.o 
	-rm *.cpp 
	-rm *.php 
	-rm *.so
