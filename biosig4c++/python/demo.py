####### Demo for Python interface to BioSig" #####################
###
###  $Id$
###  Copyright (C) 2009 Alois Schloegl <a.schloegl@ieee.org>
###  This file is part of the "BioSig for C/C++" repository 
###  (biosig4c++) at http://biosig.sf.net/ 
###
##############################################################

# download and extract 
#   http://www.biosemi.com/download/BDFtestfiles.zip 
# into /tmp/
# then run this demo 
#
# on linux you can run instead  
#   make test 

import biosig
import numpy as S

#def load(fname):
HDR = biosig.constructHDR(0, 0)
#HDR = biosig.sopen('/scratch/schloegl/R/data/test/HekaPatchMaster/AP100427b.dat' , 'r', HDR)
#HDR = biosig.sopen('/scratch/schloegl/R/data/test/HekaPatchMaster/AP100429a.dat' , 'r', HDR)
HDR = biosig.sopen('/scratch/schloegl/R/data/test/CFS/example_6channels.dat' , 'r', HDR)

# show header information 
biosig.hdr2ascii(HDR,4)  


#	turn off all channels 
#    for i in range(HDR.NS):
#        HDR.CHANNEL[i].OnOff = 0

#	turn on specific channels 
#    HDR.CHANNEL[0].OnOff = 1
#    HDR.CHANNEL[1].OnOff = 1
#    HDR.CHANNEL[HDR.NS-1].OnOff = 1

# read data 
data = biosig.sread(0, HDR.NRec, HDR)

# close file
biosig.sclose(HDR)

# release allocated memory
biosig.destructHDR(HDR)
    
#return data
