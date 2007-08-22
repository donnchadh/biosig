function [argout,H1,h2] = hdr2ascii(source,dest)
% HDR2ASCII converts the header information into ASCII text. 
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

%	$Id: hdr2ascii.m,v 1.5 2007-08-22 15:07:44 schloegl Exp $
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
	dest = [tempname,'.dlm']; 
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
	if fid<0,
		fprintf(2,'ERROR HDR2ASCII: could not open file %s\n',dest);
		return; 
	end; 
else
	fid = 1; 
end; 


%%%%%%%%% FIXED HEADER %%%%%%%%%%%%%%		
fprintf(fid,'[BioSig Header]\n\n'); 
fprintf(fid,'Version=0.10\n',dest); 
fprintf(fid,'generated=%04i-%02i-%02i %02i:%02i:%2.0f\n',datevec(now)); 
if fid>2;
	fprintf(fid,'\n;This is a TAB-delimiter file. When you edit this file, make sure not to corrupt the TABs ASCII(9)!\n\n'); 
	fprintf(fid,'ThisFile=%s\n',dest); 
end; 


fprintf(fid,'\n[Fixed Header]\n'); 
fprintf(fid,'Filename\t= %s\n',HDR.FileName); 
fprintf(fid,'Format  \t= %s\n',HDR.TYPE); 
if isfield(HDR.FILE,'size'), fprintf(fid,'SizeOfFile\t= %i\n',HDR.FILE.size); end;
fprintf(fid,'NumberOfChannels\t= %i\n',HDR.NS); 
fprintf(fid,'SamplingRate    \t= %i\n',HDR.SampleRate); 
fprintf(fid,'Number_of_Samples\t= %i\n',HDR.NRec*HDR.SPR); 
fprintf(fid,'RecordingDateTime\t= %04i-%02i-%02i %02i:%02i:%06.3f\n',HDR.T0); 
if isfield(HDR,'Patient')
fprintf(fid,'Patient.\n'); 
if isfield(HDR.Patient,'Name')
	fprintf(fid,'\tName      \t= %s\n',HDR.Patient.Name); 
end;
if isfield(HDR.Patient,'Id')
	fprintf(fid,'\tId\t\t= %s\n',HDR.Patient.Id); 
end;
if isfield(HDR.Patient,'Sex')
	if (HDR.Patient.Sex==1)
		fprintf(fid,'\tGender   \t= male\n'); 
	elseif (HDR.Patient.Sex==2)
		fprintf(fid,'\tGender   \t= female\n'); 
	else
		fprintf(fid,'\tGender   \t= unknown\n');
	end;
end;
if isfield(HDR.Patient,'Birthday')
	fprintf(fid,'\tAge\t\t= %4.1f years\n',(datenum(HDR.T0)-datenum(HDR.Patient.Birthday))/(365.25)); 
	fprintf(fid,'\tBirth\t\t= %04i-%02i-%02i %02i:%02i:%06.3f\n',HDR.Patient.Birthday); 
end;
end;

%%%%%%%% CHANNEL DATA %%%%%%%%%%%%%%%
if ~isfield(HDR,'AS')
	HDR.AS.SampleRate = repmat(HDR.SampleRate,HDR.NS,1); 
end;
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
if ~isfield(HDR,'LeadIdCode')
	HDR = leadidcodexyz(HDR); 
end;
if ~isfield(HDR,'REC')
	HDR.REC.Impedance = repmat(NaN,HDR.NS,1); 
end;
if ~isfield(HDR.REC,'Impedance')
	HDR.REC.Impedance = repmat(NaN,HDR.NS,1); 
end;
if ~isfield(HDR,'Off')
	HDR.Off = zeros(HDR.NS,1); 
end;
if ~isfield(HDR,'Cal')
	HDR.Cal = ones(HDR.NS,1); 
	HDR.Cal(HDR.InChanSelect) = diag(HDR.Calib(2:end,:));
end;
if length(HDR.Filter.HighPass)==1,
	HDR.Filter.HighPass = repmat(HDR.Filter.HighPass,HDR.NS,1); 
end;
if length(HDR.Cal)==1,
	HDR.Cal = repmat(HDR.Cal,HDR.NS,1); 
end;
if length(HDR.Filter.LowPass)==1,
	HDR.Filter.LowPass = repmat(HDR.Filter.LowPass,HDR.NS,1); 
end;
if length(HDR.Filter.Notch)==1,
	HDR.Filter.Notch = repmat(HDR.Filter.Notch,HDR.NS,1); 
end;

PhysDim = physicalunits(HDR.PhysDimCode); 
fprintf(fid,'\n[Channel Header]\n#No  LeadId  Label\tfs [Hz]\tGDFTYP\tTH-  TH+  Offset  Calib  PhysDim  HP[Hz]  LP[Hz]  Notch  R[kOhm]  x  y  z\n'); 
for k = 1:HDR.NS;
	Label = HDR.Label{k};
	Z = HDR.REC.Impedance(k)/1000; 
	gdftyp = HDR.GDFTYP(min(length(HDR.GDFTYP),k)); 
	Label(Label==9)=' '; % replace TAB's because TAB's are used as field delimiter
	fprintf(fid,'%3i  %i\t%9s\t%6.1f %2i  %i\t%i\t%6e\t%6e %5s  %6.4f %5.1f  %i  %5.1f  %f %f %f\n',k,HDR.LeadIdCode(k),Label,HDR.AS.SampleRate(k),gdftyp,HDR.THRESHOLD(k,1:2),HDR.Off(k),HDR.Cal(k),PhysDim{k},HDR.Filter.HighPass(k),HDR.Filter.LowPass(k),HDR.Filter.Notch(k),Z,HDR.ELEC.XYZ(k,:)); 
end;


%%%%%%%%%% EVENTTABLE %%%%%%%%%%%%%%%55
fprintf(fid,'\n[Event Table]\n'); 
fprintf(fid,'NumberOfEvents=%i\n   TYP\t   POS ',length(HDR.EVENT.POS)); 
if isfield(HDR.EVENT,'CHN')
	fprintf(fid,'\tCHN\tDUR/VAL'); 
end; 
fprintf(fid,'\tDescription\n'); 

% use global to improve speed
global BIOSIG_GLOBAL;
if ~isfield(BIOSIG_GLOBAL,'ISLOADED_EVENTCODES')
	BIOSIG_GLOBAL.ISLOADED_EVENTCODES=0;
end; 
if ~BIOSIG_GLOBAL.ISLOADED_EVENTCODES,
	H=sopen('eventcodes.txt'); sclose(H); 
	BIOSIG_GLOBAL.EVENT = H.EVENT;
end;
	
for k = 1:length(HDR.EVENT.POS);
	fprintf(fid,'0x%04x\t%7i',[HDR.EVENT.TYP(k),HDR.EVENT.POS(k)]'); 
	if isfield(HDR.EVENT,'CHN')
		if ~isempty(HDR.EVENT.CHN)
			fprintf(fid,'\t%i\t%i',HDR.EVENT.CHN(k),HDR.EVENT.DUR(k)); 
		end;
	end; 
	if HDR.EVENT.TYP(k)==hex2dec('7fff'),
		ch = HDR.EVENT.CHN(k);
		fprintf(fid,'\t%f %s',[1,HDR.EVENT.DUR(k)]*HDR.Calib([1,ch+1],ch),HDR.PhysDim{ch}); 
	elseif isfield(HDR.EVENT,'CodeDesc');
		fprintf(fid,'\t%s',HDR.EVENT.CodeDesc{HDR.EVENT.TYP(k)});
	else
		ix = find(HDR.EVENT.TYP(k)==BIOSIG_GLOBAL.EVENT.CodeIndex);
		if length(ix)==1,
			fprintf(fid,'\t%s',BIOSIG_GLOBAL.EVENT.CodeDesc{ix});
		end; 
	end; 
	fprintf(fid,'\n');
end; 

if fid>2,
	fclose(fid); 
end;
if nargout>0,
	argout=HDR;
end;	

