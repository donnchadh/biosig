function [HDR] = artifact_selection(fn,t1,t2)
% ARTIFACT_SELECTION returns the selected triggered trials with 
% artifacts. The length of the trial is defined by t1 and t2 in 
% seconds.  
%
% [HDR] = getselfile(filename,[t1,t2])
% [HDR] = getselfile(HDR,[t1,t2])
%    uses EVENT information in filename or HDR.EVENT. 
%
% The artifact selection is available in HDR.ArtifactSelection. 
% HDR.ArtifactSelection is vector with the same length than the list
% of trigger points (event 0x0300). A value of 1 indicates an
% artifact, a value of 0 means the trial is free from any artifacts.
% 	
% In case the trigger information and the artifact scoring is 
% stored in separate files, the information of several files can be
% merged.
%
% [HDR] = selected trials({sourcefile,eventfile1,eventfile2,...},[t1,t2])
%
% All files can be defined by their filename, or by the BIOSIG HDR-struct.
% The header of the first file is merged with the Event information of 
%    all other files. 
%
% see also: TRIGG, SOPEN, SCLOSE

%	$Revision: 1.1 $
% 	$Id: artifact_selection.m,v 1.1 2004-12-02 20:13:26 schloegl Exp $
%	Copyright (c) 2004 by Alois Schloegl <a.schloegl@ieee.org>
%

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


fprintf(1,'Artifact_Selection: currently, this is only for testing purposes.\n');

if ischar(fn)
	HDR = sopen(fn,'r'); HDR = sclose(HDR);
elseif iscell(fn)
	for k = 1:length(fn)
		if ischar(fn{k})
			h = sopen(fn{k},'r'); h = sclose(h);
		elseif isstruct(fn{k}) & isfield(fn{k},'EVENT')	
			h = fn{k};
		end	
		if k==1,
			HDR = h; 
		else	
			if isfield(h,'SampleRate')
				if isfield(HDR,'SampleRate')
					if HDR.SampleRate ~= h.SampleRate,
						h.EVENT.POS = h.EVENT.POS*HDR.SampleRate/h.SampleRate;
						h.EVENT.DUR = h.EVENT.DUR*HDR.SampleRate/h.SampleRate;
					end	
				else
					HDR.SampleRate = h.SampleRate;
				end;	 
			end; 
			HDR.EVENT.TYP = [HDR.EVENT.TYP; h.EVENT.TYP];
			HDR.EVENT.POS = [HDR.EVENT.POS; h.EVENT.POS];
			HDR.EVENT.CHN = [HDR.EVENT.CHN; h.EVENT.CHN];
			HDR.EVENT.DUR = [HDR.EVENT.DUR; h.EVENT.DUR];
		end;	
	end;	
end;

% define interval
if prod(size(t1))==1,
    ti = [t1,t2];
else
    ti = t1;
end;    
ti = ti*HDR.SampleRate;

% prepare trigger information 
if ~isfield(HDR,'TRIG')
	HDR.TRIG = HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('0300'));
end;
SEL = zeros(length(HDR.TRIG),1);

% prepare artifact information 
ix  = find(bitand(HDR.EVENT.TYP,hex2dec('FFF0'))==hex2dec('0100'));
A.EVENT.POS = [0; HDR.EVENT.POS(ix); HDR.EVENT.POS(ix) + HDR.EVENT.DUR(ix); inf];   % onset and offset 
A.EVENT.TYP = [0; ones(length(ix),1);-ones(length(ix),1); 0];			% onset = +1, offset = -1;
[A.EVENT.POS, ix2]=sort(A.EVENT.POS);		%  sort the positions
A.EVENT.TYP = cumsum(A.EVENT.TYP(ix2));		%  count the number of artifacts

% check each trial for an artifact. 
ix1 = 1; 
ix2 = 1; 
for k = 1:length(HDR.TRIG)
	if 0, 

	elseif (HDR.TRIG(ix1)+ti(1) <= A.EVENT.POS(ix2)) & (HDR.TRIG(ix1)+ti(2)>A.EVENT.POS(ix2))
	    SEL(ix1) = SEL(ix1) | A.EVENT.TYP(ix2)>0; 
	    ix2 = ix2+1;     

	elseif (HDR.TRIG(ix1)+ti(1) > A.EVENT.POS(ix2)) 
	    ix2 = ix2+1;     
	
	elseif (HDR.TRIG(ix1)+ti(2) <= A.EVENT.POS(ix2))
	    ix1 = ix1+1;     

	end;
end; 

HDR.ArtifactSelection = SEL; 
