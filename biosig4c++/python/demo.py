import biosig
import numpy as S


fname = 'gdf2test.gdf'

#def load(fname):
HDR = biosig.constructHDR(0, 0)
HDR = biosig.sopen(fname, "r", HDR);

#	turn off all channels 
#    for i in range(HDR.NS):
#        HDR.CHANNEL[i].OnOff = 0

#	turn on specific channels 
#    HDR.CHANNEL[0].OnOff = 1
#    HDR.CHANNEL[1].OnOff = 1
#    HDR.CHANNEL[HDR.NS-1].OnOff = 1

data = biosig.sread(0, HDR.NRec, HDR)

biosig.sclose(HDR)

biosig.hdr2ascii(HDR,3)


#biosig.destructHDR(HDR)
    
#return data
