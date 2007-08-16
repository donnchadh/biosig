%%% Octave script for testing BioSig4C++:SAVE2GDF 

%	$Id: test01.m,v 1.2 2007-08-16 13:13:53 schloegl Exp $
%	Copyright (c) 2007 by Alois Schloegl <a.schloegl@ieee.org>	
%       This file is part of the biosig project http://biosig.sf.net/

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

pfad=pwd;
cd ..

[s,m]=unix('make'); %% compile save2gdf
if (s>0), 
	fprintf(stdout,'this script must be started from within .../biosig4c++/test0/ directory!\n'); 
end; 	
[s,m]=unix('make testhl7');  %% run test for HL7 file 
[s,m]=unix('make testscp');  %% run test for SCP file


fn = dir('/tmp/t1.*');  
clear s HDR
for k=1:length(fn), 
try
	%[s{k},H]=sload(fn(k).name,0,'UCAL','ON'); 
	[H]=sopen(fullfile('/tmp',fn(k).name),'r',0);
	%[H]=sopen(fn(k).name,'r',0);
	%H.FLAG.UCAL=1;
	H.FLAG.OVERFLOWDETECTION=0; 
	[s{k},H]=sread(H);
	H = sclose(H); 
	HDR{k}=H; 
	[t,scale]=physicalunits(H.PhysDimCode);
	s{k} = s{k}*diag(scale);

	r = rms(s{k});
	r1 = std(s{k});
	R(k,1:length(r1))=r1;
	fprintf(1,'%02i %s: [%s] %e %e %e %e\n',k,H.FileName,H.PhysDim{1},r(1:4));  
catch ; end; 
end; 

for k=1:length(HDR)
try
	fprintf(1,'%02i %-32s: [%s] %e %e %e %e\n',k, HDR{k}.FileName,HDR{k}.PhysDim{1},R(k,1:4));  
catch
end;
end; 

cd(pfad); 
