# make SigViewer from biosig4c++
# rm -rf ../sigviewer/extern
# ln -s ../biosig4c++ ../sigviewer/extern 

qmake -o QMakefile
make -f QMakefile

# symbolic link to latest biosig4c
rm -rf ../sigviewer/extern/biosig.h
ln -s ../../biosig4c++/biosig.h ../sigviewer/extern/biosig.h 
rm ../sigviewer/extern/libbiosig.a
ln -s ../../biosig4c++/libbiosig.a ../sigviewer/extern/libbiosig.a

# copy latest table of event codes 
cp ../biosig/doc/eventcodes.txt ../sigviewer/bin/
cp ../biosig/doc/units.csv      ../sigviewer/bin/

# build sigviewer
cd ../sigviewer/src/
qmake
make

cd ../../biosig4c++