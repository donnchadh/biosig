function [argout,H1,h2] = hdr2ascii(source,dest)
% HDR2ASCII converts the header information into ASCII text. 
%    The output is displayed to standard output
%
%   HDR2ASCII(HDR [, ...]);
%	converts file header HDR 
%   HDR2ASCII(file [, ...]);
%	converts header of file 
%   HDR2ASCII(arg,dest_file);
%	converts file header HDR and writes it into dest_file
%   HDR=HDR2ASCII(...);
%	returns header HDR 
%  
% see also: SLOAD, SOPEN


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

%	$Id: hdr2ascii.m,v 1.1 2007-01-24 17:02:07 schloegl Exp $
%	(C) 2007 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

if nargin<2,
	if isstruct(source) 
		HDR = source; 
	elseif ischar(source)
		HDR = sopen(source); 
		HDR = sclose(HDR); 
	else
		'not implemented yet',
	end;	
	dest = tempname; 
elseif isstruct(source) & ischar(dest); 
	HDR = source; 
elseif ischar(source) & ischar(dest); 
	HDR = sopen(source); 
	HDR = sclose(HDR); 
else
	'not implemented yet',
end; 


if nargin>1,
	fid = fopen(dest,'wt'); 
else
	fid = 1; 
end; 


%%%%%%%%% FIXED HEADER %%%%%%%%%%%%%%		
fprintf(fid,'[BioSig Header]\n\n'); 

fprintf(fid,'Filename=%s\n',HDR.FileName); 
fprintf(fid,'NumberOfChannels=%i\n',HDR.NS); 
fprintf(fid,'SamplingRate=%i\n',HDR.SampleRate); 
fprintf(fid,'Number_of_Samples=%i\n',HDR.NRec*HDR.SPR); 
fprintf(fid,'RecordingDateTime=%i-%i-%i %i:%i:%i\n',HDR.T0); 


%%%%%%%% CHANNEL DATA %%%%%%%%%%%%%%%
if ~isfield(HDR.AS,'SampleRate')
	HDR.AS.SampleRate = repmat(HDR.SampleRate,HDR.NS,1); 
end;
if ~isfield(HDR,'THRESHOLD')
	HDR.THRESHOLD = repmat(NaN,HDR.NS,2); 
end;
if ~isfield(HDR.Filter,'Notch')
	HDR.Filter.Notch = repmat(NaN,HDR.NS,1); 
end;
if ~isfield(HDR,'PhysDimCode')
	HDR.PhysDimCode = physicalunits(HDR.PhysDim); 
end;
if ~isfield(HDR,'REC')
	HDR.REC.Impedance = repmat(NaN,HDR.NS,1); 
end;
if ~isfield(HDR.REC,'Impedance')
	HDR.REC.Impedance = repmat(NaN,HDR.NS,1); 
end;
if length(HDR.Filter.HighPass)==1,
	HDR.Filter.HighPass = repmat(HDR.Filter.HighPass,HDR.NS,1); 
end;
if length(HDR.Filter.LowPass)==1,
	HDR.Filter.LowPass = repmat(HDR.Filter.LowPass,HDR.NS,1); 
end;

PhysDim = physicalunits(HDR.PhysDimCode); 
fprintf(fid,'\n[Channel Header]\nNo\tLabel    \tfs [Hz]\tdtype\tTH-\tTH+\tOffset     \tCalib    \tPhys.Dim.\tHP [Hz]\tLP [Hz]\tNotch\tImpedance[kOhm]\t       X\t      Y\t       Z\n'); 
for k = 1:HDR.NS,
	Label = HDR.Label{k};
	Z = HDR.REC.Impedance(k)/1000; 
	gdftyp = gdfdatatype(HDR.GDFTYP(min(length(HDR.GDFTYP),k))); 
	Label(Label==9)=' '; % replace TAB's because TAB's are used as field delimiter
	fprintf(fid,'#%3i\t%9s\t%6.1f\t%s\t%i\t%i\t%6e\t%6e\t%s\t%6.6f\t%5.1f\t%i\t%5.1f\t%f\t%f\t%f\n',k,Label,HDR.AS.SampleRate(k),gdftyp,HDR.THRESHOLD(k,1:2),full(HDR.Calib([1,k+1],k)),PhysDim{k},HDR.Filter.HighPass(k),HDR.Filter.LowPass(k),HDR.Filter.Notch(k),Z,HDR.ELEC.XYZ(k,:)); 
end;


%%%%%%%%%% EVENTTABLE %%%%%%%%%%%%%%%55
fprintf(fid,'\n[Event Table]\nTYP\tPOS'); 
if isfield(HDR.EVENT,'CHN')
	fprintf(fid,'\tCHN\tDUR'); 
end; 
fprintf(fid,'\nNumberOfEvents=%i\n',length(HDR.EVENT.POS)); 

if length(HDR.EVENT.POS),
	if ~isfield(HDR.EVENT,'CHN')
		fprintf(fid,'0x%04x\t%7i\t%i\t%i\n',[HDR.EVENT.TYP,HDR.EVENT.POS]'); 
	elseif isempty(HDR.EVENT.CHN)
		fprintf(fid,'0x%04x\t%7i\t%i\t%i\n',[HDR.EVENT.TYP,HDR.EVENT.POS]'); 
	else
		fprintf(fid,'0x%04x\t%7i\t%i\t%i\n',[HDR.EVENT.TYP,HDR.EVENT.POS,HDR.EVENT.CHN,HDR.EVENT.DUR]'); 
	end; 
end; 

if fid>2,
	fclose(fid); 
end;
if nargout>0,
	argout=HDR;
end;	

