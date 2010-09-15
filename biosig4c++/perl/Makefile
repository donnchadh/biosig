####### Makefile for "BioSig for C/C++" #####################
###
###  $Id: Makefile 2526 2010-08-27 21:46:28Z schloegl $
###  Copyright (C) 2010 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository
###  (biosig4c++) at http://biosig.sf.net/
###
##############################################################

# More information on SWIG-Perl interface is available here
# http://www.swig.org/Doc1.3/Perl5.html#Perl5



all:
	swig -c++ -perl biosig.i
	g++ -fPIC -c biosig_wrap.cxx -I/usr/lib/perl/5.10.1/CORE -Dbool=char
	g++ -shared ../libbiosig.so biosig_wrap.o -o biosig.so

clean:
	-rm *.cxx 
	-rm *.o
	-rm *.pm
	-rm *.so
