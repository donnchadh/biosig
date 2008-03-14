# make SigViewer from biosig4c++
# rm -rf ../sigviewer/extern
# ln -s ../biosig4c++ ../sigviewer/extern 

rm -rf ../sigviewer/extern/biosig.h
ln -s ../../biosig4c++/biosig.h ../sigviewer/extern/biosig.h 
rm ../sigviewer/extern/libbiosig.a
ln -s ../../biosig4c++/libbiosig.a ../sigviewer/extern/libbiosig.a

cd ../sigviewer/src/
qmake
make
