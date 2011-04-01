: Conversion of HEKA/Patchmaster data into ITX (Igor Text) format 
:
:    $Id$	
:    Copyright (C) 2008, 2011 Alois Schloegl <a.schloegl@ieee.org>
:    This file is part of the "BioSig for C/C++" repository 
:    (biosig4c++) at http://biosig.sf.net/ 
echo off
echo HEKA2ITX is part of BioSig http://biosig.sf.net and licensed with GNU GPL v3.
echo HEKA2ITX Copyright 2008,2011 Alois Schloegl <a.schoegl@ieee.org>
echo HEKA2ITX converts BIN data into other biosignal data formats (default GDF)
echo usage: heka2itx source dest
echo usage: heka2itx -SWEEP=ne,ng,ns source dest
echo          selects sweep ns from group ng from experiment ne. 
echo          use 0 as wildcard selecting all sweeps fullfilling the criteria	
save2gdf.exe -f=ITX %1 %2 %3 %4 %5 %6 %7
