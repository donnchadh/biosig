: Conversion of ASCII-Header and binary channels into REC(GDF,EDF) data format
:
:    $Id: bin2rec.bat,v 1.1 2008-08-11 07:55:24 schloegl Exp $	
:    Copyright (C) 2008 Alois Schloegl <a.schloegl@ieee.org>
:    This file is part of the "BioSig for C/C++" repository 
:    (biosig4c++) at http://biosig.sf.net/ 
echo off
echo REC2BIN is part of BioSig http://biosig.sf.net and licensed with GNU GPL v3.
echo BIN2REC Copyright 2008 Alois Schloegl <a.schoegl@ieee.org>
echo BIN2REC converts BIN data into other biosignal data formats (default GDF)
echo usage: bin2rec source dest
echo usage: bin2rec -f=FMT source dest
echo		FMT can be BDF,BIN,CFWB,EDF,GDF,HL7aECG,SCP_ECG
save2gdf -f=GDF %1 %2 %3 %4 %5 %6 %7
