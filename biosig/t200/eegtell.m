function [HDR] = eegtell(HDR)
% EEGTELL returns file position of EEG data files
% EEG=sdftell(EEG)
% returns the location of the EEEG_file position indicator in the specified file.  
% Position is indicated in Blocks from the beginning of the file.  If -1 is returned, 
% it indicates that the query was unsuccessful; 
% EEG_Struct is a struct obtained by EEGOPEN.
%
% EEG.FILE.POS contains the position of the EDF-Identifier in Blocks
%
% See also: FTELL, EEGOPEN, EEGREAD, EEGWRITE, EEGCLOSE, EEGREWIND, EEGTELL, EEGSEEK, EEGEOF


%	$Revision: 1.1 $
%	$Id: eegtell.m,v 1.1 2003-02-01 15:03:46 schloegl Exp $
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

elseif strcmp(HDR.TYPE,'BKR') | strcmp(HDR.TYPE,'CNT') | strcmp(HDR.TYPE,'EEG'),
	POS = (POS-HDR.HeadLen)/HDR.AS.bpb;

else
    	fprintf(2,'Error EEGTELL: format %s not supported',HDR.TYPE);    
end;        

if HDR.FILE.POS~=POS,
        fprintf(2,'Warning EEGTELL: %s File position error  %i  %i\n', HDR.FileName, POS, HDR.FILE.POS);
end;        
HDR.FILE.POS=POS;
