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

%	$Revision: 1.5 $
%	$Id: eegeof.m,v 1.5 2003-05-26 17:17:24 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


%status=feof(EDF.FILE.FID);  % does not work properly
%if EDF.FILE.POS~=EDF.AS.startrec+EDF.AS.numrec;
        
if strmatch(HDR.TYPE,{'EDF','BDF','GDF','RDF','EEG'}),
	%status=feof(EDF.FILE.FID);  % does not work properly
	%if EDF.FILE.POS~=EDF.AS.startrec+EDF.AS.numrec;
        status = (HDR.FILE.POS >= HDR.NRec);
	
elseif strmatch(HDR.TYPE,{'BKR','CNT','MIT','RG64','LABVIEW','SMA'}),
	status = (HDR.FILE.POS >= (HDR.AS.endpos-HDR.HeadLen));

elseif strmatch(HDR.TYPE,{'EGI'}),
        if HDR.FLAG.TRIGGERED,
	        status = (HDR.FILE.POS >= HDR.NRec);
        else        
                status = (HDR.FILE.POS >= HDR.SPR);
        end;
else
	status=feof(HDR.FILE.FID);
end;



