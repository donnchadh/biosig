function [status]=seof(HDR)
% SEOF checks for end of signal-file
%    status = seof(HDR)
%
% returns 1 if End-of-EDF-File is reached
% returns 0 otherwise
%
% See also: SOPEN, SREAD, SWRITE, SCLOSE, SSEEK, SREWIND, STELL, SEOF

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

%	$Revision: 1.7 $
%	$Id: seof.m,v 1.7 2004-06-16 18:24:41 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


%status=feof(HDR.FILE.FID);  % does not work properly
%if HDR.FILE.POS~=HDR.AS.startrec+HDR.AS.numrec;
        
if strmatch(HDR.TYPE,{'EDF','BDF','GDF','CTF','RDF','EEG','AVG','SIGIF'}),
	%status=feof(EDF.FILE.FID);  % does not work properly
	%if EDF.FILE.POS~=EDF.AS.startrec+EDF.AS.numrec;
        status = (HDR.FILE.POS >= HDR.NRec);
	
elseif strmatch(HDR.TYPE,{'RG64','LABVIEW','BVbinmul','BVbinvec','BVascii'}),
	status = (HDR.FILE.POS >= (HDR.AS.endpos-HDR.HeadLen));

elseif strmatch(HDR.TYPE,{'BKR','CNT','CTF','MIT','SMA','CFWB','DEMG','EEProbe-CNT','EEProbe-AVR','MFER','alpha','GTEC','SCP'}),
	status = (HDR.FILE.POS >= HDR.SPR*HDR.NRec);

elseif strmatch(HDR.TYPE,{'EGI'}),
        if HDR.FLAG.TRIGGERED,
	        status = (HDR.FILE.POS >= HDR.NRec);
        else        
                status = (HDR.FILE.POS >= HDR.SPR);
        end;

elseif strmatch(HDR.TYPE,{'FIF'}),
        [buf, status] = rawdata('next');
        status = strcmp(status,'eof');
        
else
	status=feof(HDR.FILE.FID);
end;



