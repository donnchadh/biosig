####### Batch to Make SigViewer from biosig4c++ ######################
###
###  $Id: makeSV.sh,v 1.5 2008-09-03 08:34:26 schloegl Exp $
###  Copyright (C) 2008 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository 
###  (biosig4c++) at http://biosig.sf.net/ 
###
##############################################################

# make SigViewer from biosig4c++
# rm -rf ../sigviewer/extern
# ln -s ../biosig4c++ ../sigviewer/extern 

qmake -o QMakefile
make -f QMakefile

# copy latest table of event codes 
cp ../biosig/doc/eventcodes.txt ../sigviewer/bin/
#cp ../biosig/doc/units.csv      ../sigviewer/bin/

# build sigviewer
cd ../sigviewer/src/
qmake
make

cd ../../biosig4c++