: Converts data into ascii-Header and each channel into a separate binary data file
:
:    $Id: rec2bin.bat,v 1.1 2008-08-11 07:55:24 schloegl Exp $	
:    Copyright (C) 2008 Alois Schloegl <a.schloegl@ieee.org>
:    This file is part of the "BioSig for C/C++" repository 
:    (biosig4c++) at http://biosig.sf.net/ 
echo off
echo REC2BIN is part of BioSig http://biosig.sf.net and licensed with GNU GPL v3.
echo REC2BIN Copyright 2008 Alois Schloegl <a.schoegl@ieee.org>
echo REC2BIN converts different biosignal data formats into the BIN format
echo usage: rec2bin source dest
echo usage: rec2bin -f=FMT source dest
echo		FMT can be BDF,BIN,CFWB,EDF,GDF,HL7aECG,SCP_ECG
save2gdf -f=BIN %1 %2 %3 %4 %5 %6 %7


