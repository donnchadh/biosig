function [HDR]=fltopen(arg1,arg3,arg4,arg5,arg6)
% FLTOPEN opens FLT file
% However, it is recommended to use SOPEN instead .
% For loading whole data files, use SLOAD. 
%
% see also: SOPEN, SREAD, SSEEK, STELL, SCLOSE, SWRITE, SEOF

% HDR=fltopen(HDR);

%	$Id: fltopen.m,v 1.6 2007-06-04 14:04:32 schloegl Exp $
%	Copyright (c) 2006,2007 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

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

if isstruct(arg1),
	HDR=arg1;
else
	FILENAME=arg1;
        HDR.FileName  = FILENAME;
        [pfad,file,FileExt] = fileparts(HDR.FileName);
        HDR.FILE.Name = file;
        HDR.FILE.Path = pfad;
        HDR.FILE.Ext  = FileExt(2:length(FileExt));
	HDR.FILE.PERMISSION = 'r';
	HDR.FILE.stdout = 1; 
	HDR.FILE.stderr = 2; 
end;


if any(HDR.FILE.PERMISSION=='r'),
        %%%%% READ HEADER

	fid = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.hdr']),'rt');
	[r,c] = fread(fid,[1,inf],'char'); 
	fclose(fid); 

	r = char(r); 
	HDR.H1 = r;
	HDR.SampleRate = 1;
	FLAG.BioSig = 0; FLAG.LockBioSig = 0;
	while ~isempty(r),
		[t,r] = strtok(r,[10,13]); 
		[tok,left] = strtok(t,'=');
		num = str2double(left(2:end));
		if 0
		elseif strncmp(tok,'*',1),
		elseif strcmp(tok,'name'),
			if ~FLAG.LockBioSig,
				FLAG.BioSig = ~isempty(strfind(left,'created by BioSig'));
				FLAG.LockBioSig = 1; 
			end;	
		elseif strcmp(tok,'type'),
			type = num(1); 
			if type<10; 
				HDR.Endianity = 'ieee-be'; 
			else	
				HDR.Endianity = 'ieee-le'; 
			end; 	
			switch mod(type,10)
			case 1,
				HDR.GDFTYP = 1;
			case 2,
				HDR.GDFTYP = 3;
			case 3,
				HDR.GDFTYP = 5;
			case 4,
				HDR.GDFTYP = 16;
			case 5,
				HDR.GDFTYP = 17;
			otherwise
				fprintf(HDR.FILE.stderr,'Error SOPEN(FLT): type %i not supported',type); 	
			end; 	

			if ~FLAG.BioSig & ~strcmpi(HDR.FILE.Name(end+[-2:0]),'flt')
				HDR.GDFTYP=3;
			end;
		elseif strcmp(tok,'number_of_samples'),
			HDR.SPR = num;
			HDR.NRec = 1; 
		elseif strcmp(tok,'number_of_channels'),
			HDR.NS = num;
		elseif strcmp(tok,'number_of_groups'),
			number_of_groups = num;
		elseif strcmp(tok,'measurement_day'),
			if any((left=='.'))
				left(left=='.')=' '; 
				[HDR.T0([3,2,1]),v,sa]=str2double(left(2:end));
			elseif any((left=='.'))
				left(left=='.')=' '; 
				[HDR.T0([1:3]),v,sa]=str2double(left(2:end));
			end; 	
		elseif strcmp(tok,'measurement_time'),
			left(left==':')=' '; 
			[HDR.T0(4:6),v,sa]=str2double(left(2:end));
		elseif strcmp(tok,'sampling_unit'),
		elseif strcmp(tok,'sampling_exponent'),
			HDR.SampleRate = HDR.SampleRate * (10^-num); 
		elseif strcmp(tok,'sampling_step'),
			HDR.SampleRate = HDR.SampleRate/num; 
		elseif strcmp(tok,'parameter_of_channels'),
			[tch_channel,r]=strtok(r,'}'); 
		elseif strcmp(tok,'parameter_of_sensors'),
			[tch_sensors,r]=strtok(r,'}'); 
		elseif strcmp(tok,'parameter_of_groups'),
			[tch_groups,r]=strtok(r,'}');
		elseif strcmp(tok,'parameter_of_modules'),
			[tch_modules,r]=strtok(r,'}'); 
		else		
		end;
	end;

	[tline,tch] = strtok(tch_groups,[10,13]); 
	N = 0; 
	while ~isempty(tline),
		if tline(1)>=32,
			[n,v,sa]=str2double(tline);
			PhysDim_Group{n(1)+1} = sa{4};
		end
		[tline,tch] = strtok(tch,[10,13]); 
	end; 	

	[tline,tch] = strtok(tch_channel,[10,13]); 
	while ~isempty(tline),
		[n,v,sa]=str2double(tline);
		if length(n)>8,
			HDR.Label{n(1)+1} = sa{4}; 
			HDR.PhysDim{n(1)+1} = PhysDim_Group{n(8)+1}; 
		end;
		[tline,tch] = strtok(tch,[10,13]); 
	end; 	
	N = 0; 
	[tline,tch] = strtok(tch_sensors,[10,13]); 
	while ~isempty(tline),
		[n1,v1,sa1] = str2double(tline);
		if strncmp(sa1{2},'A',1),
			[tline,tch] = strtok(tch,[10,13]); 
			[n2,v2,sa2] = str2double(tline);
			N = N + 1; 
			if strcmp([sa1{2},'S'], sa2{2});
				HDR.ELEC.XYZ(N,:) = (n1(5:7)+n2(5:7))/2;
			else
				HDR.ELEC.XYZ(N,:) = n1(5:7);
			end; 	
		else
			N = N + 1; 
			HDR.ELEC.XYZ(N,:) = n1(5:7);
		end;
		[tline,tch] = strtok(tch,[10,13]); 
	end;
	if HDR.GDFTYP < 10,
		FLAG = HDR.FLAG; % backup
		HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
		HDR.FLAG.UCAL = 1; % default: no calibration information 
		% read scaling information
		ix  = strfind([HDR.FILE.Name,'.'],'.');
		fid = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.calib.txt']),'r'); 
		if fid < 0,
			fid = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name(1:ix(1)-1),'.calib.txt']),'r'); 
		end; 
		if fid>0,
			c   = fread(fid,[1,inf],'char=>char'); fclose(fid);
			[n1,v1,sa1] = str2double(c,9,[10,13]);
			%fid = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name(1:ix-1),'.calib2.txt']),'r');
			%c  = fread(fid,[1,inf],'char'); fclose(fid); 
			%[n2,v2,sa2] = str2double(c); 
			%HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,(n2(4:131,5).*n1(4:131,3)./n2(4:131,4))./100);
			HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,n1(4:131,3)*1e-13*(2^-12));
			HDR.FLAG = FLAG; % restore
		end
	else 	
		HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
	end
	if ~isfield(HDR,'PhysDim') & ~isfield(HDR,'PhysDimCode') 
		HDR.PhysDimCode = zeros(HDR.NS,1); 
	end; 	
		
	% read data
	HDR.FILE.FID = fopen(fullfile(HDR.FILE.Path,HDR.FILE.Name),'rb',HDR.Endianity);
	if HDR.FILE.FID>0,
		HDR.FILE.OPEN= 1; 
	end;
	HDR.HeadLen  = 0; 
	HDR.FILE.POS = 0; 

	fseek(HDR.FILE.FID,0,'eof'); 
	HDR.FILE.size = ftell(HDR.FILE.FID); 
	fseek(HDR.FILE.FID,0,'bof'); 
        [datatyp,limits,datatypes,numbits,GDFTYP]=gdfdatatype(HDR.GDFTYP);
	HDR.AS.bpb    = HDR.NS*numbits/8;
	HDR.AS.endpos = HDR.FILE.size/HDR.AS.bpb;
        
        if (HDR.AS.bpb*HDR.NRec*HDR.SPR) ~= HDR.FILE.size,
        	fprintf(HDR.FILE.stderr,'Warning SOPEN(ET-MEG): size of file does not fit to header information\n');
        	fprintf(HDR.FILE.stderr,'\tFile:\t%s\n',fullfile(HDR.FILE.Path,HDR.FILE.Name));
        	fprintf(HDR.FILE.stderr,'\tFilesize:\t%i is not %i bytes\n',HDR.FILE.size,HDR.AS.bpb*HDR.NRec*HDR.SPR);
        	fprintf(HDR.FILE.stderr,'\tSamples:\t%i\n',HDR.NRec*HDR.SPR);
        	fprintf(HDR.FILE.stderr,'\tChannels:\t%i\n',HDR.NS);
        	fprintf(HDR.FILE.stderr,'\tDatatype:\t%s\n',datatyp);
        	HDR.SPR = floor(HDR.AS.endpos/HDR.NRec);
        end;	

else
	fid = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.hdr']),'wt');
	if 0, isfield(HDR,'H1') 
		% copy header
		fwrite(fid,HDR.H1,'char');
	else	

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

		%%%%%%%%% FIXED HEADER %%%%%%%%%%%%%%		
		fprintf(fid,'[Header]\n'); 
		fprintf(fid,'Version=2.1\nid=1\n'); 
		fprintf(fid,'name=created by BioSig for Octave and Matlab http://biosig.sf.net/\n'); 
		fprintf(fid,'comment=just for testing, some features are not supported, yet\n'); 

		fprintf(fid,'\n[Dataformat]\n'); 
		fprintf(fid,'* data types : HP-UX data\n');
		fprintf(fid,'*               1=(1 byte int)    2=(2 byte int)    3=(4 byte int)\n');
		fprintf(fid,'*               4=(4 byte float)  5=(8 byte float)  6=(ASCII)\n');
		fprintf(fid,'*              LINUX data\n');
		fprintf(fid,'*              11=(1 byte int)   12=(2 byte int)   13=(4 byte int)\n');
		fprintf(fid,'*              14=(4 byte float) 15=(8 byte float) 16=(ASCII)\n');
		fprintf(fid,'version=1.0\nid=1\nname=ET-MEG double data format\n'); 
		fprintf(fid,'type=14\n'); HDR.GDFTYP=16; % float32
		if isfield(HDR,'data')
			[HDR.SPR,HDR.NS]=size(HDR.data);
			HDR.NRec = 1;
		end;	
		fprintf(fid,'number_of_samples=%i\n',HDR.NRec*HDR.SPR); 

		fprintf(fid,'\n[Measurement]\nversion=0.0\nlaboratory_name=ET_HvH\n'); 
		fprintf(fid,'*\n* Antialiasing-Filter\n*\n');
		for k = 0:7,
			fprintf(fid,'* Unit %i (Channels %3i-%3i): not available \n',k,k*16+[0,15]);
		end;

		fprintf(fid,'*\nmeasurement_day=%i.%i.%i\n',HDR.T0([3,2,1])); 
		fprintf(fid,'measurement_time=%i:%i:%i\n',HDR.T0(4:6));
		fprintf(fid,'sampling_unit=s\n'); 
		e = floor(log10(1/HDR.SampleRate));
		fprintf(fid,'sampling_exponent=%i\n',e); 
		fprintf(fid,'sampling_step=%f\n',10^-e/HDR.SampleRate); 
		 
		fprintf(fid,'\n[System]\nversion=0.0\n'); 
		fprintf(fid,'number_of_channels=%i\n',HDR.NS); 
		%fprintf(fid,'SamplingRate=%i\n',HDR.SampleRate); 


		fprintf(fid,'*---------------------------------------------------------------\n');
		fprintf(fid,'*seq id   u name              calib grd grd_name  grp  n_sensors\n');
		fprintf(fid,'*---------------------------------------------------------------\n');
		fprintf(fid,'parameter_of_channels={\n');
		for k=1:HDR.NS,
			fprintf(fid,'%04i  %04i 1 %-16s 1.000  1  ####      0000   1\n',k-1,k-1,HDR.Label{k});
			fprintf(fid,'        %04i 1.000000 * CH%i\n',k-1,k-1);
		end;
		
		fprintf(fid,'}\n\nnumber_of_sensors=%i\n',HDR.NS);
		fprintf(fid,'*----------------------------------------------------------------------------------------\n');
		fprintf(fid,'*id  name     type mod    x         y         z         a         b         c        area\n');
		fprintf(fid,'*----------------------------------------------------------------------------------------\n');
		fprintf(fid,'parameter_of_sensors={\n');
		for k=1:HDR.NS,
			fprintf(fid,'%04i CH%-08i  1 0000  %8.5f  %8.5f  %8.5f  %8.5f  %8.5f  %8.5f  %11.9f\n',k-1,k-1,HDR.ELEC.XYZ(k,1:3),[NaN,NaN,NaN,NaN]);
		end;
		
		fprintf(fid,'}\n\nnumber_of_groups= 11\n');
		fprintf(fid,'*----------------------------------------\n');
		fprintf(fid,'*id  u name             unit   exp  calib\n');
		fprintf(fid,'*----------------------------------------\n');
		fprintf(fid,'parameter_of_groups={\n');
		fprintf(fid,'0001 1 ET-Mag_80WH      T      0    1.000\n');
		fprintf(fid,'0002 1 ET-AxGrd_80WH    T      0    1.000\n');
		fprintf(fid,'0003 1 ET-PlGrd_80WH    T      0    1.000\n');
		fprintf(fid,'0004 1 ET-Mag_RefCh     T      0    1.000\n');
		fprintf(fid,'0005 1 ET-AxGrd_RefCh   T      0    1.000\n');
		fprintf(fid,'0006 1 ET-PlGrd_RefCh   T      0    1.000\n');
		fprintf(fid,'0007 1 Trigger          V      0    1.000\n');
		fprintf(fid,'0008 1 EEG              V      0    1.000\n');
		fprintf(fid,'0009 1 ECG              V      0    1.000\n');
		fprintf(fid,'0010 1 Etc              V      0    1.000\n');
		fprintf(fid,'0011 0 Null_Channel     V      0    1.000\n}\n');
		fprintf(fid,'\nnumber_of_modules=2\n');
		fprintf(fid,'*-------------------------------------------------------------------------\n');
		fprintf(fid,'*id  name          x      y      z      a      b      c      unit exp name\n');
		fprintf(fid,'*-------------------------------------------------------------------------\n');
		fprintf(fid,'parameter_of_modules={\n');
		fprintf(fid,'0000 Magnetic      0.000  0.000  0.000  0.000  0.000  0.000  1.000 0 m\n');
		fprintf(fid,'0007 Electric      0.000  0.000  0.000  0.000  0.000  0.000  1.000 0 m\n}\n');

	end;
	fclose(fid);

	HDR.FILE.FID  = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name]),'wb','ieee-le');
	HDR.FILE.OPEN = 2;
	HDR.HeadLen   = 0; 

end;

