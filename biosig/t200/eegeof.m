function [status]=eegeof(HDR)
% EEGEOF checks for end of EEG-file
%    status=eegeof(HDR)
%
% returns 1 if End-of-EDF-File is reached
% returns 0 otherwise
%
% See also: feof, EEGREAD, EEGWRITE, EEGCLOSE, EEGSEEK, EEGREWIND, EEGTELL, EEGEOF

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the  License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

%	$Revision: 1.1 $
%	$Id: eegeof.m,v 1.1 2003-02-01 15:03:45 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


%status=feof(EDF.FILE.FID);  % does not work properly
%if EDF.FILE.POS~=EDF.AS.startrec+EDF.AS.numrec;
        
if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'BDF') | strcmp(HDR.TYPE,'GDF'),
	%status=feof(EDF.FILE.FID);  % does not work properly
	%if EDF.FILE.POS~=EDF.AS.startrec+EDF.AS.numrec;
        status=(HDR.FILE.POS>=HDR.NRec);
elseif strcmp(HDR.TYPE,'BKR'),
	status = (HDR.FILE.POS*HDR.NS >= (HDR.AS.endpos-HDR.HeadLen));
elseif strcmp(HDR.TYPE,'CNT'),
	status = (HDR.FILE.POS*HDR.NS >= (HDR.AS.endpos-HDR.HeadLen));
else
	status=feof(HDR.FILE.FID);
end;



