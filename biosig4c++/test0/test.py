import pylab
import numpy
import biosig
HDR=biosig.sopen('/home/schloegl/data/test/gdf/sample.gdf','r',biosig.constructHDR(0,0));
#for i in range(HDR.NS):
#    HDR.CHANNEL[i].OnOff = 0
#HDR.CHANNEL[0].OnOff = 1
data = biosig.sread(0, HDR.NRec, HDR)
biosig.sclose(HDR)
#biosig.destructHDR(HDR)


pylab.ion();
pylab.plot(numpy.transpose(data))
pylab.show();

