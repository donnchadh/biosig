%%% Octave script for testing BioSig4C++:SAVE2GDF 

%	$Id: test01.m,v 1.1 2007-08-02 18:42:53 schloegl Exp $
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


clear s HDR
fn = dir('t1.*.gz');  
%fn = [dir('t1.*'); dir('t2.*');dir('PFE103.scp')]; 
fn = [dir('t1.*'); dir('PFE103.scp')]; 
for k=1:length(fn), 
%try
	%[s{k},H]=sload(fn(k).name,0,'UCAL','ON'); 
	[H]=sopen(fn(k).name,'r',0);
	%[H]=sopen(fn(k).name,'r',0);
	%H.FLAG.UCAL=1;
	H.FLAG.OVERFLOWDETECTION=0; 
	[s{k},H]=sread(H);
	H = sclose(H); 
	HDR{k}=H; 
	[t,scale]=physicalunits(H.PhysDimCode);
	scale=1;
	r = rms(s{k})*diag(scale);
	r1 = std(s{k})*diag(scale);
	R(k,:)=r1;
	fprintf(1,'%02i %s: [%s] %e %e %e %e\n',k,H.FileName,H.PhysDim{1},r(1:4));  
%catch ; end; 
end; 

for k=1:length(HDR)
try
	fprintf(1,'%02i %12s: [%s] %e %e %e %e\n',k, HDR{k}.FileName,HDR{k}.PhysDim{1},R(k,1:4));  
catch
end;
end; 

