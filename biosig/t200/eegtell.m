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
% STELL replaces EEGTELL. 
%
% See also: FTELL, EEGOPEN, EEGREAD, EEGWRITE, EEGCLOSE, EEGREWIND, EEGTELL, EEGSEEK, EEGEOF


%	$Revision: 1.11 $
%	$Id: eegtell.m,v 1.11 2003-09-06 18:31:08 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


[POS,HDR] = stell(HDR);

