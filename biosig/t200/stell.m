function [POS,HDR] = stell(HDR)
% STELL returns file position of signal data files
% HDR = stell(HDR)
% returns the location of the HDR-signal file position indicator in the specified file.  
% Position is indicated in Blocks from the beginning of the file.  If -1 is returned, 
% it indicates that the query was unsuccessful; 
% HDR-struct is a struct obtained by SOPEN.
%
% HDR.FILE.POS contains the position of the HDR-Identifier in Blocks
%
% See also: SOPEN, SREAD, SWRITE, SCLOSE, SSEEK, SREWIND, STELL, SEOF


%	$Revision: 1.12 $
%	$Id: stell.m,v 1.12 2005-02-19 21:45:08 schloegl Exp $
%	Copyright (c) 1997-2005 by Alois Schloegl
%	a.schloegl@ieee.org	


POS = ftell(HDR.FILE.FID);
if POS<0,
        [HDR.ERROR,HDR.ErrNo] = ferror(HDR.FILE.FID);
        return; 
end;

if strmatch(HDR.TYPE,{'EDF','BDF','GDF','CTF'}),
	POS = (POS-HDR.HeadLen)/HDR.AS.bpb;
	HDR.ERROR = [];
	HDR.ErrNo = 0;

	if (HDR.AS.startrec+HDR.AS.numrec)~=POS,
        	fprintf(2,'Warning SDFTELL: File postion error in EDF/GDF/BDF-toolbox.\n')
                HDR.AS.startrec = POS;
        end;

elseif strmatch(HDR.TYPE,{'ACQ','BKR','ISHNE','CNT','EEG','AVG','MIT','RG64','LABVIEW','Nicolet','EGI','SMA','SND','WAV','AIF','CFWB','DEMG','alpha','BCI2000'}),
	POS = (POS-HDR.HeadLen)/HDR.AS.bpb;

elseif strmatch(HDR.TYPE,{'RDF','SIGIF','BVbinmul','BVbinvec','BVascii','EEProbe-CNT','EEProbe-AVR','FIF','native','MFER','WG1'}),
	POS = HDR.FILE.POS;
	
else
    	fprintf(2,'Error STELL: format %s not supported',HDR.TYPE);    
end;        

if HDR.FILE.POS~=POS,
        fprintf(2,'Warning STELL: %s File position error  %i  %i\n', HDR.FileName, POS, HDR.FILE.POS);
end;        
HDR.FILE.POS=POS;	
