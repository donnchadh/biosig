function [POS,HDR] = eegtell(HDR)
% EEGTELL returns file position of EEG data files
% EEG=eegtell(EEG)
% returns the location of the EEG_file position indicator in the specified file.  
% Position is indicated in Blocks from the beginning of the file.  If -1 is returned, 
% it indicates that the query was unsuccessful; 
% EEG_Struct is a struct obtained by EEGOPEN.
%
% EEG.FILE.POS contains the position of the EDF-Identifier in Blocks
%
% See also: FTELL, EEGOPEN, EEGREAD, EEGWRITE, EEGCLOSE, EEGREWIND, EEGTELL, EEGSEEK, EEGEOF


%	$Revision: 1.6 $
%	$Id: eegtell.m,v 1.6 2003-05-27 13:53:16 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


POS = ftell(HDR.FILE.FID);
if POS<0,
        [HDR.ERROR,HDR.ErrNo] = ferror(HDR.FILE.FID);
        return; 
end;

if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'BDF') | strcmp(HDR.TYPE,'GDF'),
	POS = (POS-HDR.HeadLen)/HDR.AS.bpb;
	HDR.ERROR = [];
	HDR.ErrNo = 0;

	if (HDR.AS.startrec+HDR.AS.numrec)~=POS,
        	fprintf(2,'Warning SDFTELL: File postion error in EDF/GDF/BDF-toolbox.\n')
                HDR.AS.startrec = POS;
        end;

elseif strmatch(HDR.TYPE,{'BKR','ISHNE','CNT','EEG','MIT','RG64','LABVIEW','EGI','SMA'}),
	POS = (POS-HDR.HeadLen)/HDR.AS.bpb;

elseif strmatch(HDR.TYPE,{'RDF'}),
	POS = HDR.FILE.POS;
	
else
    	fprintf(2,'Error EEGTELL: format %s not supported',HDR.TYPE);    
end;        

if HDR.FILE.POS~=POS,
        fprintf(2,'Warning EEGTELL: %s File position error  %i  %i\n', HDR.FileName, POS, HDR.FILE.POS);
end;        
HDR.FILE.POS=POS;	
