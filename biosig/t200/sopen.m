function [HDR,H1,h2] = sopen(arg1,PERMISSION,CHAN,MODE,arg5,arg6)
% Opens signal files for reading and writing. 
% Many different data formats are supported.
%
% HDR = sopen(Filename,PERMISSION, [, CHAN [, MODE]]);
% [S,HDR] = sread(HDR, NoR, StartPos);
% HDR = sclose(HDR);
%
% PERMISSION is one of the following strings 
%	'r'	read header
%	'w'	write header
%
% CHAN defines the selected Channels
%
% HDR contains the Headerinformation and internal data
% S 	returns the signal data 
%
% see also: SOPEN, SREAD, SSEEK, STELL, SCLOSE, SWRITE, SEOF


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

%	$Revision: 1.2 $
%	$Id: sopen.m,v 1.2 2003-09-07 21:24:23 schloegl Exp $
%	(C) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


if nargin<2, 
        PERMISSION='rb'; 
elseif ~any(PERMISSION=='b');
        PERMISSION = [PERMISSION,'b']; % force binary open. Needed for Octave
end;
if nargin<3, CHAN = 0; end; 
if nargin<4, MODE = ''; end;

if ~isstruct(arg1),
	HDR.FileName = arg1;
else
        HDR = arg1;
end;

if isempty(MODE), MODE=' '; end;	% Make sure MODE is not empty -> FINDSTR

HDR.FLAG.UCAL = ~isempty(findstr(MODE,'UCAL'));   % FLAG for UN-CALIBRATING
HDR.FLAG.FILT = 0; 	% FLAG if any filter is applied; 

[pfad,file,FileExt] = fileparts(HDR.FileName);
HDR.FILE.Name = file;
HDR.FILE.Path = pfad;
HDR.FILE.Ext  = FileExt(2:length(FileExt));
HDR.FILE.OPEN = 0;
if ~isfield(HDR.FILE,'stderr'),
        HDR.FILE.stderr = 2;
end;
if ~isfield(HDR.FILE,'stdout'),
        HDR.FILE.stdout = 1;
end;

%if exist(HDR.FileName)==2,
if any(PERMISSION=='r'),
	fid = fopen(HDR.FileName,'rb','ieee-le');
	if fid>0,
		[s,c] = fread(fid,[1,32],'uchar');
                if c,
                        type_mat4=str2num(char(abs(sprintf('%04i',s(1:4)*[1;10;100;1000]))'));
	                ss = setstr(s);
                        if all(s(1:2)==[207,0]); 
                                HDR.TYPE='BKR';
                        elseif strncmp(ss,'Version ',8); 
                                HDR.TYPE='CNT';
                        elseif strncmp(ss,'0       ',8); 
                                HDR.TYPE='EDF';
                        elseif all(s(1:8)==[255,abs('BIOSEMI')]); 
                                HDR.TYPE='BDF';
                        elseif strncmp(ss,'GDF',3); 
                                HDR.TYPE='GDF';
                        elseif strncmp(ss,'EBS',3); 
                                HDR.TYPE='EBS';
                        elseif strncmp(ss,'CEN',3) & all(s(4:8)==hex2dec(['13';'10';'1A';'04';'84'])'); 
                                HDR.TYPE='CEN';
                                HDR.VERSION   = s(9:16);
                                HDR.Encoding  = s(17:24);
                                if any(s(25:32)),
                                        HDR.Endianity = 'ieee-be';
                                else
                                        HDR.Endianity = 'ieee-le';
                                end;
                        elseif strncmp(ss,'ISHNE1.0',8);	% ISHNE Holter standard output file.
                                HDR.TYPE='ISHNE';
                        elseif strncmp(ss,'POLY_SAM',8);	% Poly5/TMS32 sample file format.
                                HDR.TYPE='TMS32';
                        elseif strncmp(ss,'"Snap-Master Data File"',23);	% Snap-Master Data File .
                                HDR.TYPE='SMA';
                        elseif all(s([1:2,20])==[1,0,0]) & any(s(19)==[2,4]); 
                                HDR.TYPE='TEAM';	% Nicolet TEAM file format
                        elseif strncmp(ss,HDR.FILE.Name,length(HDR.FILE.Name)); 
                                HDR.TYPE='MIT';
                        elseif strncmp(ss,'.snd',4); 
                                HDR.TYPE='SND';
				HDR.Endianity = 'ieee-be';
                        elseif strncmp(ss,'dns.',4); 
                                HDR.TYPE='SND';
				HDR.Endianity = 'ieee-le';
                        elseif strcmp(ss([1:4,9:12]),'RIFFWAVE'); 
                                HDR.TYPE='WAV';
				HDR.Endianity = 'ieee-le';
                        elseif strcmp(ss([1:4,9:11]),'FORMAIF'); 
                                HDR.TYPE='AIF';
				HDR.Endianity = 'ieee-be';
                        elseif strncmp(ss,'RG64',4); 
                                HDR.TYPE='RG64';
                        elseif strncmp(ss,'DTDF',4); 
                                HDR.TYPE='DDF';
                        elseif strncmp(ss,'RSRC',4);
                                HDR.TYPE='LABVIEW';
                        elseif strncmp(ss,'IAvSFo',6); 
                                HDR.TYPE='SIGIF';
                        elseif any(s(4)==(2:7)) & all(s(1:3)==0); % [int32] 2...7
                                HDR.TYPE='EGI';
                        elseif strncmp(ss,'rhdE',4);	% Holter Excel 2 file, not supported yet. 
                                HDR.TYPE='rhdE';          
			elseif any(s(3:6)*(2.^[0;8;16;24]) == (30:40))
				HDR.TYPE='ACQ';
			elseif all(s(1:2)==[hex2dec('55'),hex2dec('AA')]);
				HDR.TYPE='RDF';

			elseif all(s(1:2)==[hex2dec('55'),hex2dec('3A')]); % little endian 
				HDR.TYPE='SEG2';
				HDR.Endianity = 'ieee-le';
			elseif all(s(1:2)==[hex2dec('3A'),hex2dec('55')]); % big endian 
				HDR.TYPE='SEG2 b';
				HDR.Endianity = 'ieee-be';

                        elseif strncmp(ss,'MATLAB Data Acquisition File.',29);% Matlab Data Acquisition File 
		                HDR.TYPE='DAQ';
                        elseif strncmp(ss,'MATLAB 5.0 MAT-file',19); 
                                HDR.TYPE='MAT5';
                                fseek(fid,126,'bof');
                                tmp = fread(fid,1,'uint16');
                                if tmp==(abs('MI').*[256;1])
	                                HDR.Endianity = 'ieee-le';
                                elseif tmp==(abs('IM').*[256;1])
                                        HDR.Endianity = 'ieee-be';
                                end;
                                
                        elseif any(s(1)==[49:51]) & all(s([2:4,6])==[0,50,0,0]) & any(s(5)==[49:50]),
				HDR.TYPE = 'WFT';	% nicolet 	

                        elseif all(s(1:2)==[102,105]); 
                                HDR.TYPE='669';
                        elseif all(s(1:2)==[234,96]); 
                                HDR.TYPE='ARJ';
                        elseif strcmp(ss([1:4,9:12]),'RIFFAVI '); 
                                HDR.TYPE='AVI';
                        elseif s(1)==2; 
                                HDR.TYPE='DB2';
                        elseif any(s(1)==[3,131]); 
                                HDR.TYPE='DB3';
                        elseif strncmp(ss,'DDMF',4); 
                                HDR.TYPE='DMF';
                        elseif strncmp(ss,'DMS',4); 
                                HDR.TYPE='DMS';
                        elseif strncmp(ss,'FAR',3); 
                                HDR.TYPE='FAR';
                        elseif all(ss(5:6)==[175,18]); 
                                HDR.TYPE='FLC';
                        elseif strncmp(ss,'GF1PATCH110',12); 
                                HDR.TYPE='GF1';
                        elseif strncmp(ss,'GIF8',4); 
                                HDR.TYPE='GIF';
                        elseif all(s(1:4)==hex2dec(['FF';'D9';'FF';'E0'])')
                                HDR.TYPE='JPG';
				HDR.Endianity = 'ieee-be';
                        elseif all(s(1:4)==hex2dec(['FF';'D8';'FF';'E0'])')
                                HDR.TYPE='JPG';
				HDR.Endianity = 'ieee-be';
                        elseif all(s(1:4)==hex2dec(['E0';'FF';'D8';'FF'])')
                                HDR.TYPE='JPG';
				HDR.Endianity = 'ieee-le';
			elseif all(s(1:3)==[0,0,1])	
                                HDR.TYPE='JPG';
                        elseif strcmp(ss([3:5,7]),'-lh-'); 
                                HDR.TYPE='LZH';
                        elseif strcmp(ss([3:5,7]),'-lz-'); 
                                HDR.TYPE='LZH';
                        elseif strcmp(ss(1:4),'MThd'); 
                                HDR.TYPE='MIDI';
                        elseif (s(1)==255) & any(s(2)>=224); 
                                HDR.TYPE='MPEG3';
                        elseif strncmp(ss(5:8),'mdat',4); 
                                HDR.TYPE='MOV';
                        elseif all(s(1:2)==[26,63]); 
                                HDR.TYPE='OPT';
                        elseif strncmp(ss,'%PDF',4); 
                                HDR.TYPE='PDF';
                        elseif strncmp(ss,'QLIIFAX',7); 
                                HDR.TYPE='QFX';
                        elseif strncmp(ss,'.RMF',4); 
                                HDR.TYPE='RMF';
                        elseif strncmp(ss,'IREZ',4); 
                                HDR.TYPE='RMF';
                        elseif strncmp(ss,'{/rtf',5); 
                                HDR.TYPE='RTF';
                        elseif strncmp(ss,'II',2); 
                                HDR.TYPE='TIFF';
                                HDR.Endianity = 0;
                        elseif strncmp(ss,'MM',2); 
                                HDR.TYPE='TIFF';
                                HDR.Endianity = 1;
                        elseif all(ss(1:2)==[25,149]); 
                                HDR.TYPE='TWE';
                        elseif all(ss(1:5)==[0,0,2,0,4]); 
                                HDR.TYPE='WKS';
                        elseif all(ss(1:5)==[0,0,2,0,abs('Q')]); 
                                HDR.TYPE='WQ1';
                        elseif all(s(1:8)==hex2dec(['30';'26';'B2';'75';'8E';'66';'CF';'11'])'); 
                                HDR.TYPE='WMV';
                        elseif all(s(1:5)==[80,75,3,4,20]); 
                                HDR.TYPE='ZIP';
                        elseif strncmp(ss,'ZYXEL',5); 
                                HDR.TYPE='ZYXEL';
                                
                        elseif any(~type_mat4),  % should be last, otherwise to many false detections
                                HDR.TYPE='MAT4';
                                if type_mat4(1)==1,
                                        HDR.MAT4.opentyp='ieee-be';
                                elseif type_mat4(1)==2,
                                        HDR.MAT4.opentyp='vaxd';
                                elseif type_mat4(1)==3,
                                        HDR.MAT4.opentyp='vaxg';
                                elseif type_mat4(1)==4,
                                        HDR.MAT4.opentyp='gray';
                                else
                                        HDR.MAT4.opentyp='ieee-le';
                                end;
                        else
                                %TYPE='unknown';
                        end;
                end;
                fclose(fid);
	else
                fprintf(HDR.FILE.stderr,'SOPEN: File %s couldnot be opened\n',HDR.FileName);
                return;
	end;
end;



        % MIT-ECG / Physiobank format
if strcmp(HDR.TYPE,'HEA'), HDR.TYPE='MIT';

elseif strcmp(HDR.TYPE,'ATR'), HDR.TYPE='MIT';

elseif strcmp(HDR.TYPE,'DAT'), 
        tmp = dir(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.hea']));
        if isempty(tmp), 
                HDR.TYPE='DAT';
        else
                HDR.TYPE='MIT';
                [tmp,tmp1,HDR.FILE.Ext] = fileparts(fn(1).name);
        end
end; 


if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'GDF') | strcmp(HDR.TYPE,'BDF'),
        if any(PERMISSION=='w');
                HDR = eegchkhdr(HDR);
        end;
        HDR = sdfopen(HDR,PERMISSION,CHAN);
	HDR.FLAG.TRIGGERED = 0;	% Trigger Flag

        
elseif strcmp(HDR.TYPE,'BKR'),
        if any(PERMISSION=='w');
                HDR = eegchkhdr(HDR);
        end;
        HDR = bkropen(HDR,PERMISSION,CHAN);

        
elseif strmatch(HDR.TYPE,['CNT';'AVG';'EEG']),
        [HDR,H1,h2] = cntopen(HDR,PERMISSION,CHAN);
        

elseif strcmp(HDR.TYPE,'CEN'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
	fseek(HDR.FILE.FID,32,'bof');

	%%%%% (2) Session Archive Section %%%%%
	DS_begin=ftell(fid,0,'bof');
	DS_ID=fread(fid,1,'int16')
	DS_Length=fread(fid,1,TYP_LENGTH);
	fseek(fid,DS_begin+DS_Length,'bof');

	%%%%% (2) Demographic Section %%%%%
	DS_begin=ftell(fid,0,'bof');
	DS_ID=fread(fid,1,'int16')
	DS_Length=fread(fid,1,TYP_LENGTH);
	fseek(fid,DS_begin+DS_Length,'bof');

	%%%%% (3) Medical Device System Presentation Section %%%%%
	MDSP_begin=ftell(fid,0,'bof');
	MDSP_ID=fread(fid,1,'int16')
	MDSP_Length=fread(fid,1,TYP_LENGTH);
	fseek(fid,MDSP_begin+MDSP_Length,'bof');

	%%%%% (4) Manufacture Specific Section %%%%%
	MS_begin=ftell(fid,0,'bof');
	MS_ID=fread(fid,1,'int16')
	MS_Length=fread(fid,1,TYP_LENGTH);
	fseek(fid,MS_begin+MS_Length,'bof');

	%%%%% (5) Image Section %%%%%
	IS_begin=ftell(fid,0,'bof');
	IS_ID=fread(fid,1,'int16')
	IS_Length=fread(fid,1,TYP_LENGTH);
	fseek(fid,IS_begin+IS_Length,'bof');

	%%%%% (6) Archive & Measurement Section %%%%%
	SA_begin=ftell(fid,0,'bof');
	OIDSA_ID=fread(fid,1,'int16')
	O_Length=fread(fid,1,TYP_LENGTH);
	fseek(fid,SA_begin+SA_Length,'bof');

	%%%%% (6.1) Test Sub-Section %%%%%


	fprintf(2,'Warning SOPEN: Implementing CEN/FEF format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
	fclose(fid);


elseif strcmp(HDR.TYPE,'EBS'),
    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
	
	fprintf(2,'Warning SOPEN: Implementing EBS format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');

	%%%%% (1) Fixed Header (32 bytes) %%%%%
	HDR.VERSION = fread(fid,[1,8],'char');	%
	if setstr(HDR.VERSION(1:3)')~='EBS' 
	    	fprintf(2,'Error LOADEBS: %s not an EBS-File',HDR.FileName); 
		if any(HDR.VERSION(4:8)~=hex2dec(['94';'0a';'13';'1a';'0d'])'); 
			fprintf(2,'Warning SOPEN EBS: %s may be corrupted',HDR.FileName); 
		end; 
	end;
	HDR.EncodingId = fread(fid,1,'int32');	%
	HDR.NS  = fread(fid,1,'uint32');	% Number of Channels
	HDR.SPR = fread(fid,2,'uint32');	% Number of Samples
	if HDR.SPR(1)==0,
		 HDR.SPR=HDR.SPR(2)
	else 
		fprintf(2,'Error SOPEN: EBS-FILE %s too large',HDR.FileName); 
	end;
	LenData=fread(fid,1,'int64');	% Data Length

	%%%%% (2) LOAD Variable Header %%%%%
	tag=fread(fid,1,'int32');	% Tag field
	while tag~=0
	        l  =fread(fid,1,'int32');	% length of value field
	        val=setstr(fread(fid,4*l,'char')');	% depends on Tag field
	        if     tag==hex2dec('00000002'),	%IGNORE
	        elseif tag==hex2dec('00000004') HDR.PATIENT_NAME=val;
	        elseif tag==hex2dec('00000006') HDR.PATIENT_ID=val;
	        elseif tag==hex2dec('00000008') HDR.PATIENT_BIRTHDAY=val;
	        elseif tag==hex2dec('0000000a') HDR.PATIENT_SEX=val;
	        elseif tag==hex2dec('0000000c') HDR.SHORT_DESCRIPTION=val;
	        elseif tag==hex2dec('0000000e') HDR.DESCRIPTION=val;
	        elseif tag==hex2dec('00000010') HDR.SAMPLE_RATE=str2num(val);
	        elseif tag==hex2dec('00000012') HDR.INSTITUTION=val;
	        elseif tag==hex2dec('00000014') HDR.PROCESSING_HISTORY=val;
	        elseif tag==hex2dec('00000016') HDR.LOCATION_DIAGRAM=val;
	     
	        elseif tag==hex2dec('00000001') HDR.PREFERRED_INTEGER_RANGE=vec2matx(vec2matx(val,HDR.NS),2);
	        elseif tag==hex2dec('00000003') HDR.PhysDim=val;
	        elseif tag==hex2dec('00000005') HDR.CHANNEL_DESCRIPTION=val;
	        elseif tag==hex2dec('00000007') HDR.CHANNEL_GROUPS=val;
	        elseif tag==hex2dec('00000009') HDR.EVENTS=val;
	        elseif tag==hex2dec('0000000b') HDR.RECORDING_TIME=val;
	        elseif tag==hex2dec('0000000d') HDR.HDR.CHANNEL_LOCATIONS=val;
	        elseif tag==hex2dec('0000000f') HDR.FILTERS=val;
		end;
		tag=fread(fid,1,'int32');	% Tag field
        end; 

	fclose(fid);


elseif strcmp(HDR.TYPE,'ACQ'),
    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');

	%--------    Fixed Header        
        ItemHeaderLen = fread(HDR.FILE.FID,1,'int16');
        HDR.VERSION = fread(HDR.FILE.FID,1,'int32');
        ExtItemHeaderLen = fread(HDR.FILE.FID,1,'int32');
        HDR.NS = fread(HDR.FILE.FID,1,'int16');
        HorizAxisType = fread(HDR.FILE.FID,1,'int16');
        CurChannel = fread(HDR.FILE.FID,1,'int16');
        SampleTime = fread(HDR.FILE.FID,1,'float64');
	%HDR.SampleRate = 1/SampleTime;
        HDR.TimeOffset = fread(HDR.FILE.FID,1,'float64');
        HDR.TimeScale  = fread(HDR.FILE.FID,1,'float64');
        TimeCursor1  = fread(HDR.FILE.FID,1,'float64');
        TimeCursor2  = fread(HDR.FILE.FID,1,'float64');
        rcWindow  = fread(HDR.FILE.FID,1,'float64');
	MeasurementType = fread(HDR.FILE.FID,6,'int16');
	HiLite = fread(HDR.FILE.FID,2,'uint8');
        HDR.FirstTimeOffset = fread(HDR.FILE.FID,1,'float64');

	if HDR.VERSION < 34, offset = 150;
	elseif HDR.VERSION < 35, offset = 164; 
	elseif HDR.VERSION < 36, offset = 326; 
	elseif HDR.VERSION < 38, offset = 886; 
	else offset = 1894; 
	end;
	
	fseek(HDR.FILE.FID,offset,'bof');
	
	%--------   Variable Header        
	HDR.Comment = zeros(HDR.NS,40);
	HDR.Off = zeros(HDR.NS,1);
	HDR.Cal = ones(HDR.NS,1);
	HDR.PhysDim = zeros(HDR.NS,20);
	
	for k = 1:HDR.NS;
	        HDR.ChanHeaderLen  = fread(HDR.FILE.FID,1,'int32');
        	HDR.ChanSel(k) = fread(HDR.FILE.FID,1,'int16');
		HDR.Comment(no,1:40) = fread(HDR.FILE.FID,[1,40],'char');
		rgbColor = fread(HDR.FILE.FID,4,'int8');
		DispChan = fread(HDR.FILE.FID,2,'int8');
		HDR.Off(k) = fread(HDR.FILE.FID,1,'float64');
		HDR.Cal(k) = fread(HDR.FILE.FID,1,'float64');
		HDR.PhysDim(k,1:20) = fread(HDR.FILE.FID,[1,20],'char');
		HDR.SPR(k) = fread(HDR.FILE.FID,1,'int32');
		HDR.AmpGain(k) = fread(HDR.FILE.FID,1,'float64');
		HDR.AmpOff(k) = fread(HDR.FILE.FID,1,'float64');
		ChanOrder = fread(HDR.FILE.FID,1,'int16');
		DispSize = fread(HDR.FILE.FID,1,'int16');

		if HDR.VERSION >= 34,
			fseek(HDR.FILE.FID,10,'cof')	
		end;
		if HDR.VERSION >= 38,
			HDR.Description(k,1:128) = fread(HDR.FILE.FID,[1,128],'char');
			HDR.VarSampleDiv(k) = fread(HDR.FILE.FID,1,'uint16');
		else
			HDR.VarSampleDiv(k) = 1;
		end;
	end;
	HDR.MAXSPR = lcm(HDR.VarSampleDiv);
	HDR.SampleRate = 1./(HDR.VarSampleDiv*SampleTime);
	HDR.Dur = HDR.MAXSPR*SampleTime;
	
	%--------   foreign data section
	ForeignDataLength = fread(HDR.FILE.FID,1,'int16');
	%ID = fread(HDR.FILE.FID,1,'');
	fseek(HDR.FILE.FID,ForeignDataLength+2,'cof');

	%--------   per channel data type section
	offset3 = 0;
	HDR.AS.bpb = 0;	
	HDR.AS.spb = 0;	
	for k = 1:HDR.NS,
        	sz = fread(HDR.FILE.FID,1,'int16');
		HDR.AS.bpb = HDR.AS.bpb + HDR.MAXSPR/HDR.VarSampleDiv(k)*sz; 
		HDR.AS.spb = HDR.AS.spb + HDR.MAXSPR/HDR.VarSampleDiv(k); 
		offset3 = offset3+HDR.SPR(k)*sz;

		typ = fread(HDR.FILE.FID,1,'int16');
		HDR.GDFTYP(k) = typ*14-11;   % 1 = int16; 2 = double
	end;

	HDR.HeadLen = offset + HDR.ChanHeaderLen*HDR.NS + ForeignDataLength + 4*HDR.NS; 
	HDR.AS.endpos = HDR.HeadLen + offset3; 
	fseek(HDR.FILE.FID,HDR.AS.endpos,'bof');	

	%--------  Markers Header section
	len = fread(HDR.FILE.FID,1,'int32');
	HDR.NumEevents = fread(HDR.FILE.FID,1,'int32');
	for k = 1:HDR.NumEvents, 
		HDR.EVENT(k).Sample = fread(HDR.FILE.FID,1,'int32');
		tmp = fread(HDR.FILE.FID,4,'int16');
		HDR.EVENT(k).selected = tmp(1); 
		HDR.EVENT(k).TextLocked = tmp(2); 
		HDR.EVENT(k).PositionLocked = tmp(3); 
		textlen = tmp(4);
		HDR.EVENT(k).Text = fread(HDR.FILE.FID,textlen,'char');
	end;
	fseek(HDR.FILE.FID,HDR.HeadLen,'bof');	
	
        
elseif strcmp(HDR.TYPE,'SND'),
	if ~isfield(HDR,'Endianity'),
		HDR.Endianity = 'ieee-be';
	end;	 
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
	if HDR.FILE.FID < 0,
		return;
        end;
	
	if ~isempty(findstr(PERMISSION,'r')),	%%%%% READ 
		HDR.FILE.OPEN = 1; 
	        fseek(HDR.FILE.FID,4,'bof');
		HDR.HeadLen = fread(HDR.FILE.FID,1,'uint32');
		datlen = fread(HDR.FILE.FID,1,'uint32');
		HDR.FILE.TYPE = fread(HDR.FILE.FID,1,'uint32');
		HDR.SampleRate = fread(HDR.FILE.FID,1,'uint32');
		HDR.NS = fread(HDR.FILE.FID,1,'uint32');
		[tmp,count] = fread(HDR.FILE.FID, [1,HDR.HeadLen-24],'char');
		HDR.INFO = setstr(tmp);
	
	elseif ~isempty(findstr(PERMISSION,'w')),	%%%%% WRITE 
		HDR.FILE.OPEN = 2; 
		if ~isfield(HDR,'NS'),
			HDR.NS = 0;
		end;
		if ~isfield(HDR,'SPR'),
			HDR.SPR = 0;
		end;
		if ~isfinite(HDR.NS)
			HDR.NS = 0;
		end;	
		if ~isfinite(HDR.SPR)
			HDR.SPR = 0;
		end;	
		if any([HDR.SPR,HDR.NS] <= 0);
			HDR.FILE.OPEN = 3; 
		end;
		if ~isfield(HDR,'INFO')
			HDR.INFO = HDR.FileName;
		end;
		len = length(HDR.INFO);
		if len == 0; 
			HDR.INFO = 'INFO';
		else
		    	HDR.INFO = [HDR.INFO,repmat(' ',1,mod(4-len,4))];	
		end;
		HDR.HeadLen = 24+length(HDR.INFO); 
		if ~isfield(HDR.FILE,'TYPE')
			HDR.FILE.TYPE = 6; % default float32
		end;		
	end;
	
	if HDR.FILE.TYPE==1, 
		HDR.GDFTYP =  0;
		HDR.bits   =  8;
	elseif HDR.FILE.TYPE==2, 
		HDR.GDFTYP =  1;
		HDR.bits   =  8;
	elseif HDR.FILE.TYPE==3, 
		HDR.GDFTYP =  3;
		HDR.bits   = 16;
	elseif HDR.FILE.TYPE==4, 
		HDR.GDFTYP = 255+24;
		HDR.bits   = 24;
	elseif HDR.FILE.TYPE==5, 
		HDR.GDFTYP =  5;
		HDR.bits   = 32;
	elseif HDR.FILE.TYPE==6, 
		HDR.GDFTYP = 16;
		HDR.bits   = 32;
	elseif HDR.FILE.TYPE==7, 
		HDR.GDFTYP = 17;
		HDR.bits   = 64;
	else
		fprintf(2,'Error SOPEN SND-format: datatype %i not supported\n',HDR.FILE.TYPE);
		return;
	end;
	HDR.AS.bpb = HDR.NS*HDR.bits/8;
	
	% Calibration 
	if any(HDR.FILE.TYPE==[2:5]), 
		HDR.Cal = 2^(1-HDR.bits); 
	else
		HDR.Cal = 1; 	
	end;
	
		%%%%% READ 
	if HDR.FILE.OPEN == 1; 
		% check file length
		fseek(HDR.FILE.FID,0,1);
		len = ftell(HDR.FILE.FID); 
		if len ~= (datlen+HDR.HeadLen),
			fprintf(HDR.FILE.stderr,'Warning SOPEN SND-format: header information does not fit file length \n');
			datlen = len - HDR.HeadLen; 
		end;	
		fseek(HDR.FILE.FID,HDR.HeadLen,-1);
		HDR.SPR  = datlen/HDR.AS.bpb;
		HDR.AS.endpos = datlen/HDR.AS.bpb;
		HDR.Dur  = HDR.SPR/HDR.SampleRate;

	        if CHAN==0,		
			HDR.SIE.InChanSelect = 1:HDR.NS;
		elseif all(CHAN>0 & CHAN<=HDR.NS),
			HDR.SIE.InChanSelect = CHAN;
	    	else
			fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
			fclose(HDR.FILE.FID); 
			HDR.FILE.FID = -1;	
			return;
		end;
	
		%%%%% WRITE 
	elseif HDR.FILE.OPEN > 1; 
		datlen = HDR.SPR * HDR.AS.bpb;
		fwrite(HDR.FILE.FID,[hex2dec('2e736e64'),HDR.HeadLen,datlen,HDR.FILE.TYPE,HDR.SampleRate,HDR.NS],'uint32');
		fwrite(HDR.FILE.FID,HDR.INFO,'char');

	end;
	HDR.FILE.POS = 0;
	HDR.NRec = 1;
        
	
elseif strcmp(HDR.TYPE,'AIF') | strcmp(HDR.TYPE,'WAV') ,
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
	
	tmp = setstr(fread(HDR.FILE.FID,[1,4],'char'));        
	if ~strcmpi(tmp,'FORM') & ~strcmpi(tmp,'RIFF'),
		fprintf(2,'Warning SOPEN AIF/WAV-format: file %s might be corrupted 1\n',HDR.FileName);
	end;
	tagsize  = fread(HDR.FILE.FID,1,'uint32');        % which size
	tagsize0 = tagsize + rem(tagsize,2); 
	tmp = setstr(fread(HDR.FILE.FID,[1,4],'char'));        
	if ~strncmpi(tmp,'AIF',3) & ~strncmpi(tmp,'WAVE',4), % not (AIFF or AIFC or WAVE)
		fprintf(2,'Warning SOPEN AIF/WAF-format: file %s might be corrupted 2\n',HDR.FileName);
	end;

	while ~feof(HDR.FILE.FID),	
		[tmp,c] = fread(HDR.FILE.FID,[1,4],'char');
		tag     = setstr(tmp);
		tagsize = fread(HDR.FILE.FID,1,'uint32');        % which size 
		tagsize0= tagsize + rem(tagsize,2); 
		filepos = ftell(HDR.FILE.FID);
                %%%% AIF - section %%%%%
		if strcmpi(tag,'COMM')
			if tagsize<18, 
				fprintf(2,'Error SOPEN AIF: incorrect tag size\n');
				return;
			end;
			HDR.NS   = fread(HDR.FILE.FID,1,'uint16');
			HDR.SPR  = fread(HDR.FILE.FID,1,'uint32');
			HDR.AS.endpos = HDR.SPR;
			HDR.bits = fread(HDR.FILE.FID,1,'uint16');
			HDR.GDFTYP = ceil(HDR.bits/8)*2-1; % unsigned integer of approbriate size;
			HDR.Cal  = 2^(1-HDR.bits);
			HDR.AS.bpb = ceil(HDR.bits/8)*HDR.NS;

			% HDR.SampleRate; % construct Extended 80bit IEEE 754 format 
			tmp = fread(HDR.FILE.FID,1,'int16');
			sgn = sign(tmp);
			if tmp(1)>= 2^15; tmp(1)=tmp(1)-2^15; end;
			e = tmp - 2^14 + 1;
			tmp = fread(HDR.FILE.FID,2,'uint32');
			HDR.SampleRate = sgn * (tmp(1)*(2^(e-31))+tmp(2)*2^(e-63));
			HDR.Dur = HDR.SPR/HDR.SampleRate;

			if tagsize>18,
				[tmp,c] = fread(HDR.FILE.FID,[1,4],'char');
				HDR.AIF.CompressionType = setstr(tmp);
				[tmp,c] = fread(HDR.FILE.FID,taglen-18-c,'char');
				HDR.AIF.CompressionName = setstr(tmp);
				
				if ~strcmpi(HDR.AIF.CompressionType,'NONE');
					fprintf(2,'Warning SOPEN AIFC-format: CompressionType %s is not supported\n', HDR.AIF.CompressionType);
				end;
			end;	
					
		elseif strcmpi(tag,'SSND');
			HDR.AIF.offset   = fread(HDR.FILE.FID,1,'int32');
			HDR.AIF.blocksize= fread(HDR.FILE.FID,1,'int32');
			tmp = (tagsize-8)/HDR.AS.bpb;
			if tmp~=HDR.SPR,
				fprintf(2,'Waring SOPEN AIF: Number of samples do not fit %i vs %i\n',tmp,HDR.SPR);
			end;
			
			HDR.HeadLen = filepos; 
			%HDR.AIF.sounddata= fread(HDR.FILE.FID,tagsize-8,'uint8');

		elseif strcmpi(tag,'FVER');
			if tagsize<4, 
				fprintf(2,'Error SOPEN WAV: incorrect tag size\n');
				return;
			end;
			HDR.AIF.TimeStamp   = fread(HDR.FILE.FID,1,'uint32');

		elseif strcmp(tag,'DATA') & strcmp(HDR.TYPE,'AIF') ;	% AIF uses upper case, there is a potential conflict with WAV using lower case data  

		elseif strcmpi(tag,'INST');

		elseif strcmpi(tag,'MIDI');

		elseif strcmpi(tag,'AESD');

		elseif strcmpi(tag,'APPL');

		elseif strcmpi(tag,'COMT');
		
		elseif strcmpi(tag,'(c) ');
			[tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
			HDR.Copyright = tmp;
                
                %%%% WAV - section %%%%%
        	elseif strcmpi(tag,'fmt ')
			if tagsize<14, 
				fprintf(2,'Error SOPEN WAV: incorrect tag size\n');
				return;
			end;
			HDR.WAV.Format = fread(HDR.FILE.FID,1,'uint16');
			HDR.NS = fread(HDR.FILE.FID,1,'uint16');
			HDR.SampleRate = fread(HDR.FILE.FID,1,'uint32');
			HDR.WAV.AvgBytesPerSec = fread(HDR.FILE.FID,1,'uint32');
			HDR.WAV.BlockAlign = fread(HDR.FILE.FID,1,'uint16');
			if HDR.WAV.Format==1,	% PCM format
    				HDR.bits = fread(HDR.FILE.FID,1,'uint16');
				tmp = ceil(HDR.bits/8);
				%if tmp==1, 	HDR.GDFTYP = 2; 	% uint8
				%elseif tmp==2,	HDR.GDFTYP = 3; 	% int16
				%else
				%end;
				HDR.GDFTYP = tmp + 1; 
				HDR.Cal = 2^(1-HDR.bits);    
			else 
				fprintf(2,'Error SOPEN WAV: format type %i not supported\n',HDR.WAV.Format);	
			end;
			if tagsize>16,
				HDR.WAV.cbSize = fread(HDR.FILE.FID,1,'uint16');
			end;
                        
                elseif strcmp(tag,'data') & strcmp(HDR.TYPE,'WAV') ;	% AIF uses upper case, there is a potential conflict with WAV using lower case data  
                        HDR.HeadLen = filepos; 
			if HDR.WAV.Format == 1, 
				HDR.AS.bpb = HDR.NS * ceil(HDR.bits/8);
				HDR.SPR = tagsize/HDR.AS.bpb;
				HDR.Dur = HDR.SPR/HDR.SampleRate;
				HDR.AS.endpos = HDR.SPR;
				    	
			else 
				fprintf(2,'Error SOPEN WAV: format type %i not supported\n',HDR.WAV.Format);	
			end;

		elseif strcmpi(tag,'fact');
			if tagsize<4, 
				fprintf(2,'Error SOPEN WAV: incorrect tag size\n');
				return;
			end;
	    		[tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
			HDR.WAV.FACT = setstr(tmp);
		
		elseif strcmpi(tag,'disp');
			if tagsize<8, 
				fprintf(2,'Error SOPEN WAV: incorrect tag size\n');
				return;
			end;
	    		[tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
			HDR.WAV.DISP = setstr(tmp);
			if ~all(tmp(1:8)==[0,1,0,0,0,0,1,1])
				HDR.WAV.DISPTEXT = setstr(tmp(5:length(tmp)));
			end;
		
		elseif strcmpi(tag,'list');
			if tagsize<4, 
				fprintf(2,'Error SOPEN WAV: incorrect tag size\n');
				return;
			end;
	    		[tmp,c]  = fread(HDR.FILE.FID,[1,tagsize],'char');
			HDR.WAV.list = setstr(tmp);
			listtype = setstr(tmp(1:4));
			listdata = setstr(tmp(5:length(tmp)));
			HDR.WAV.LIST = setfield(HDR.WAV,listtype, listdata);

		elseif ~isempty(tagsize)
			fprintf(1,'Warning SOPEN AIF/WAV: unknown TAG: %s \n',tag);
		end;
		if ~isempty(tagsize)
			fseek(HDR.FILE.FID,filepos+tagsize0,'bof');
		end;
		
	end;
        
        if ~isfield(HDR,'HeadLen')
		fprintf(2,'Error SOPEN AIF/WAV: missing data section\n');
		fclose(HDR.FILE.FID)	
	end;
	
	fseek(HDR.FILE.FID,HDR.HeadLen,'bof');
	HDR.FILE.POS = 0;
	HDR.FILE.OPEN = 1;
	HDR.NRec = 1;

        if CHAN==0,		
		HDR.SIE.InChanSelect = 1:HDR.NS;
	elseif all(CHAN>0 & CHAN<=HDR.NS),
		HDR.SIE.InChanSelect = CHAN;
	else
		fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
		fclose(HDR.FILE.FID); 
		HDR.FILE.FID = -1;	
		return;
	end;

        
elseif strcmp(HDR.TYPE,'EGI'),
    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        
        HDR.VERSION = fread(HDR.FILE.FID,1,'integer*4');
        
        if ~(HDR.VERSION >= 2 & HDR.VERSION <= 7),
             %   fprintf(HDR.FILE.stderr,'EGI Simple Binary Versions 2-7 supported only.\n');
        end;
        
        HDR.T0 = fread(HDR.FILE.FID,[1,6],'integer*2');
        millisecond = fread(HDR.FILE.FID,1,'integer*4');
        HDR.T0(6) = HDR.T0(6) + millisecond/1000;
        
        HDR.SampleRate = fread(HDR.FILE.FID,1,'integer*2');
        HDR.NS = fread(HDR.FILE.FID,1,'integer*2');
        HDR.gain = fread(HDR.FILE.FID,1,'integer*2');
        HDR.bits = fread(HDR.FILE.FID,1,'integer*2');
        HDR.DigMax  = 2^HDR.bits;
        HDR.PhysMax = fread(HDR.FILE.FID,1,'integer*2');
        
        if CHAN<1, CHAN=1:HDR.NS; end;
        HDR.SIE.ChanSelect = CHAN;
        HDR.SIE.InChanSelect = CHAN;
        
        HDR.eventtypes = 0;
        HDR.categories = 0;
        HDR.catname    = {};
        HDR.eventcode  = '';
        
        if any(HDR.VERSION==[2,4,6]),
                HDR.SPR  = fread(HDR.FILE.FID, 1 ,'integer*4');
                HDR.eventtypes = fread(HDR.FILE.FID,1,'integer*2');
                HDR.NRec = 1;
                HDR.FLAG.TRIGGERED = logical(0); 
                HDR.AS.spb = (HDR.NS+HDR.eventtypes);
                HDR.AS.endpos = HDR.SPR;
		HDR.Dur = 1/HDR.SampleRate;
        elseif any(HDR.VERSION==[3,5,7]),
                HDR.categories = fread(HDR.FILE.FID,1,'integer*2');
                if (HDR.categories),
                        for i=1:HDR.categories,
                                catname_len(i) = fread(HDR.FILE.FID,1,'uchar');
                                HDR.catname{i} = char(fread(HDR.FILE.FID,catname_len(i),'uchar'))';
                        end
                end
                HDR.NRec = fread(HDR.FILE.FID,1,'integer*2');
                HDR.SPR  = fread(HDR.FILE.FID,1,'integer*4');
                HDR.eventtypes = fread(HDR.FILE.FID,1,'integer*2');
                HDR.FLAG.TRIGGERED = logical(1); 
                HDR.AS.spb = HDR.SPR*(HDR.NS+HDR.eventtypes);
                HDR.AS.endpos = HDR.NRec;
		HDR.Dur = HDR.SPR/HDR.SampleRate;
        else
                fprintf(HDR.FILE.stderr,'Invalid EGI version %i\n',HDR.VERSION);
                return;
        end
        
        % get datatype from version number
        if any(HDR.VERSION==[2,3]),
                HDR.datatype = 'integer*2';
                HDR.AS.bpb = HDR.AS.spb*2;
        elseif any(HDR.VERSION==[4,5]),
                HDR.datatype = 'float32';
                HDR.AS.bpb = HDR.AS.spb*4;
        elseif any(HDR.VERSION==[6,7]),
                HDR.datatype = 'float64';
                HDR.AS.bpb = HDR.AS.spb*8;
        else
                error('Unknown data format');
        end
        HDR.AS.bpb = HDR.AS.bpb + 6*HDR.FLAG.TRIGGERED;
                
                
        if isequal(HDR.eventtypes,0),
                HDR.eventcode(1,1:4) = 'none';
        else
                for i = 1:HDR.eventtypes,
                        HDR.eventcode(i,1:4) = char(fread(HDR.FILE.FID,[1,4],'uchar'));
                        HDR.EventData{i} = [];
                end
                
        end
        
        HDR.HeadLen = ftell(HDR.FILE.FID);
        HDR.FILE.POS= 0;
       
        
elseif strcmp(HDR.TYPE,'TEAM'),		% Nicolet TEAM file format
	% implementation of this format is not finished yet.

	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
	%%%%% X-Header %%%%%
	HDR.VERSION = fread(HDR.FILE.FID,1,'int16');
	HDR.NS = fread(HDR.FILE.FID,1,'int16');
	HDR.NRec = fread(HDR.FILE.FID,1,'int16');
	HDR.TEAM.Length = fread(HDR.FILE.FID,1,'int32');
	HDR.TEAM.NSKIP = fread(HDR.FILE.FID,1,'int32');
	HDR.SPR = fread(HDR.FILE.FID,1,'int32');
	HDR.Samptype = fread(HDR.FILE.FID,1,'int16');
	if   	HDR.Samptype==2, HDR.GDFTYP = 3;
	elseif 	HDR.Samptype==4, HDR.GDFTYP = 16; 
	else
		fprintf(2,'Error SOPEN TEAM-format: invalid file\n');
		fclose(HDR.FILE.FID);
		return;
	end;	
	HDR.XLabel = fread(HDR.FILE.FID,[1,8],'char');
	HDR.X0 = fread(HDR.FILE.FID,1,'float');
	HDR.TEAM.Xstep = fread(HDR.FILE.FID,1,'float');
	HDR.SampleRate = 1/HDR.TEAM.Xstep;
	tmp = fread(HDR.FILE.FID,[1,6],'uchar');
	tmp(1) = tmp(1) + 1980;
	HDR.T0 = tmp([4,5,6,1,2,3]);
	HDR.TEAM.Nevents = fread(HDR.FILE.FID,1,'int16');
	HDR.TEAM.Nsegments = fread(HDR.FILE.FID,1,'int16');
	HDR.TEAM.SegmentOffset = fread(HDR.FILE.FID,1,'int32');
	HDR.XPhysDim = fread(HDR.FILE.FID,[1,8],'char');
	HDR.TEAM.RecInfoOffset = fread(HDR.FILE.FID,1,'int32');
        fseek(HDR.FILE.FID,256,'bof');
	%%%%% Y-Header %%%%%
        for k = 1:HDR.NS,
		HDR.Label(k,1:7) = fread(HDR.FILE.FID,[1,7],'char');
		HDR.PhysDim(k,1:7) = fread(HDR.FILE.FID,[1,7],'char');
	    	HDR.Off(1,k) = fread(HDR.FILE.FID,1,'float');
	    	HDR.Cal(1,k) = fread(HDR.FILE.FID,1,'float');
	    	HDR.PhysMax(1,k) = fread(HDR.FILE.FID,1,'float');
	    	HDR.PhysMin(1,k) = fread(HDR.FILE.FID,1,'float');
		fseek(HDR.FILE.FID,2,'cof');
	end;
        HDR.HeadLen = 256+HDR.NS*32;
	
	% Digital (event) information 
	HDR.TEAM.DigitalOffset = 256 + 32*HDR.NS + HDR.NS*HDR.NRec*HDR.SPR*HDR.Samptype;
	fssek(HDR.FILE.FID,HDR.TEAM.DigitalOffset,'bof');
	if HDR.TEAM.DigitalOffset < HDR.TEAM.SegmentOffset,
		HDR.EventLabels = setstr(fread(HDR.FILE.FID,[16,HDR.TEAM.Nevents],'char')');

		% Events could be detected in this way
		% HDR.Events = zeros(HDR.SPR*HDR.NRec,1);
		% for k = 1:ceil(HDR.TEAM.Nevents/16)
		%	HDR.Events = HDR.Events + 2^(16*k-16)*fread(HDR.FILE.FID,HDR.SPR*HDR.NRec,'uint16');
		% end;
	end;
	
	% Segment informatino block entries 
	if HDR.TEAM.Nsegments,
		fseek(HDR.FILE.FID,HDR.TEAM.SegmentOffset,'bof');
		for k = 1:HDR.TEAM.Nsegments, 
			HDR.TEAM.NSKIP(k) = fread(HDR.FILE.FID,1,'int32');
			HDR.SPR(k)  = fread(HDR.FILE.FID,1,'int32');
			HDR.X0(k) = fread(HDR.FILE.FID,1,'float');
			HDR.Xstep(k) = fread(HDR.FILE.FID,1,'float');
			fseek(HDR.FILE.FID,8,'cof');
		end;
	end;
	
	% Recording information block entries
	if HDR.TEAM.RecInfoOffset,
		fseek(HDR.FILE.FID,HDR.TEAM.RecInfoOffset,'bof');
		blockinformation = fread(HDR.FILE.FID,[1,32],'char');
		for k = 1:HDR.NRec, 
			HDR.TRIGGER.Time(k) = fread(HDR.FILE.FID,1,'double');
			HDR.TRIGGER.Date(k,1:3) = fread(HDR.FILE.FID,[1,3],'uint8');
			fseek(HDR.FILE.FID,20,'cof');
		end;
		HDR.TRIGGER.Date(k,1) = HDR.TRIGGER.Date(k,1) + 1900;
 	end;
	fprintf(2,'Warning SOPEN: Implementing Nicolet TEAM file format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
	fclose(HDR.FILE.FID);
	
	
elseif strcmp(HDR.TYPE,'WFT'),	% implementation of this format is not finished yet.

    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
	[s,c] = fread(HDR.FILE.FID,1536,'char');
	[tmp,s] = strtok(s,setstr([0,32]));
	Nic_id0 = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	Niv_id1 = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	Nic_id2 = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	User_id = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.HeadLen = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.FILE.Size = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.VERSION = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.WFT.WaveformTitle = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.T0(1) = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.T0(1,2) = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.T0(1,3) = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	tmp = str2num(tmp);
	HDR.T0(1,4:6) = [floor(tmp/3600000),floor(rem(tmp,3600000)/60000),rem(tmp,60000)];
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.SPR = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.Off = str2num(tmp);
	[tmp,s] = strtok(s,setstr([0,32]));
	HDR.Cal = str2num(tmp);

        fseek(HDR.FILE.FID,HDR.HeadLen,'bof');
	
	fprintf(2,'Warning SOPEN: Implementing Nicolet WFT file format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
	fclose(HDR.FILE.FID);

	
elseif strcmp(HDR.TYPE,'LDR'),
        HDR = openldr(HDR,PERMISSION);      

        
elseif strcmp(HDR.TYPE,'SMA'),  % under constructions
        PERMISSION = PERMISSION(PERMISSION~='b');
        if ~exist('OCTAVE_VERSION'),
        	PERMISSION=[PERMISSION,'t'];	% force text mode [Matlab has default binary-mode]
        end;
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');

        numbegin=0;
        HDR.H1 = [];
        while ~numbegin,
                line = fgetl(HDR.FILE.FID);
                HDR.H1 = [HDR.H1 line];
                if strncmp('"NCHAN%"',line,8) 
                        [tmp,line] = strtok(line,'=');
                        [tmp,line] = strtok(line,'"=');
                        HDR.NS = str2num(tmp);
                end
                if strncmp('"NUM.POINTS"',line,12) 
                        [tmp,line] = strtok(line,'=');
                        [tmp,line] = strtok(line,'"=');
                        HDR.SPR = str2num(tmp);
                end
                if strncmp('"ACT.FREQ"',line,10) 
                        [tmp,line] = strtok(line,'=');
                        [tmp,line] = strtok(line,'"=');
                        HDR.SampleRate= str2num(tmp);
                end
                if strncmp('"DATE$"',line,7)
                        [tmp,line] = strtok(line,'=');
                        [date,line] = strtok(line,'"=');
                        date(date=='-')=' ';
                        date = str2num(date);
                end
                if strncmp('"TIME$"',line,7)
                        [tmp,line] = strtok(line,'=');
                        [time,line] = strtok(line,'"=');
			time(time==':')=' ';
                        time = str2num(time);
                end;
                if strncmp('"UNITS$[]"',line,10)
                        [tmp,line] = strtok(line,'=');
                        for k=1:HDR.NS,
                                [HDR.PhysDim{k},line] = strtok(line,[', =',10,13]);
                        end;
                end
                if strncmp('"CHANNEL.RANGES[]"',line,18)
                        [tmp,line] = strtok(line,'=');
                        for k=1:HDR.NS,
                                [tmp,line] = strtok(line,[' =',10,13]);
                                tmp(tmp=='(' | tmp==')')=' ';
                                tmp=str2num(tmp);
                                HDR.PhysMin(k,1)=tmp(1);
                                HDR.PhysMax(k,1)=tmp(2);
                        end;
                end
                if strncmp('"CHAN$[]"',line,9)
                        [tmp,line] = strtok(line,'=');
                        for k=1:HDR.NS,
                                [HDR.Label{k,1},line] = strtok(line,[', =',10,13]);
                        end;
                end
                if 0,strncmp('"CHANNEL.LABEL$[]"',line,18)
                        [tmp,line] = strtok(line,'=');
                        for k=1:HDR.NS,
                                [HDR.Label{k,1},line] = strtok(line,[', =',10,13]);
                        end;
                end
                if strncmp(line,'"TR"',4) 
                        HDR.H1 = HDR.H1(1:length(HDR.H1)-length(line));
                        line = fgetl(HDR.FILE.FID); % get the time and date stamp line
		        tmp=fread(HDR.FILE.FID,1,'uint8'); % read sync byte hex-AA char
                        if tmp~=hex2dec('AA');
                                fprintf(HDR.FILE.stderr,'Error SOPEN type=SMA: Sync byte is not "AA"\n');
                        end;        
                        numbegin=1;
                end
        end
        HDR.T0 = [date([3,1,2]),time];
        
        %%%%%%%%%%%%%%%%%%% check file length %%%%%%%%%%%%%%%%%%%%
        
        HDR.FILE.POS= 0;
        HDR.HeadLen = ftell(HDR.FILE.FID);  % Length of Header
        fseek(HDR.FILE.FID,0,'eof'); 
        endpos = ftell(HDR.FILE.FID); 
        fseek(HDR.FILE.FID,HDR.HeadLen,'bof');
        %[HDR.AS.endpos,HDR.HeadLen,HDR.NS,HDR.SPR,HDR.NS*HDR.SPR*4,HDR.AS.endpos-HDR.HeadLen - HDR.NS*HDR.SPR*4]
        if endpos-HDR.HeadLen ~= HDR.NS*HDR.SPR*4;
                fprintf(HDR.FILE.stderr,'Warning SOPEN TYPE=SMA: Header information does not fit size of file\n');
                fprintf(HDR.FILE.stderr,'\tProbably more than one data segment - this is not supported in the current version of SOPEN\n');
        end
        HDR.AS.bpb    = HDR.NS*4;
        HDR.AS.endpos = (endpos-HDR.HeadLen)/HDR.AS.bpb;
	HDR.Dur = 1/HDR.SampleRate;
	HDR.NRec = 1;
	
        if ~isfield(HDR,'SMA')
	        HDR.SMA.EVENT_CHANNEL= 1;
        	HDR.SMA.EVENT_THRESH = 2.3;
        end;
        HDR.Filter.T0 = zeros(1,length(HDR.SMA.EVENT_CHANNEL));
        
        if CHAN==0,		
		HDR.SIE.InChanSelect = 1:HDR.NS;
	elseif all(CHAN>0 & CHAN<=HDR.NS),
		HDR.SIE.InChanSelect = CHAN;
	else
		fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
		fclose(HDR.FILE.FID); 
		HDR.FILE.FID = -1;	
		return;
	end;
                
        
elseif strcmp(HDR.TYPE,'RDF'),  
    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');

        fseek(HDR.FILE.FID,6,-1);
        HDR.NS = fread(HDR.FILE.FID,1,'uint16');
	fseek(HDR.FILE.FID,552,-1);
	HDR.SampleRate  = fread(HDR.FILE.FID,1,'uint16');
	fseek(HDR.FILE.FID,580,-1);
	tmp = fread(HDR.FILE.FID,[8,HDR.NS],'char');
	HDR.Label = char(tmp');

	cnt = 0;
	ev_cnt = 0;
	ev = [];
    
	% first pass, scan data
	totalsize = 0;
	while(~feof(HDR.FILE.FID)),
    	        tag = fread(HDR.FILE.FID,1,'uint32');
    		if length(tag) == 0,
    		        break;
    		end
    		if tag == hex2dec('f0aa55'),
        		cnt = cnt + 1;
			HDR.Block.Pos(cnt) = ftell(HDR.FILE.FID);

        		% Read nchans and block length
        		tmp = fread(HDR.FILE.FID,34,'uint16');

    	    		%fseek(HDR.FILE.FID,2,0);
        		nchans = tmp(2); %fread(HDR.FILE.FID,1,'uint16');
    	    		%fread(HDR.FILE.FID,1,'uint16');
        		block_size = tmp(4); %fread(HDR.FILE.FID,1,'uint16');
        		%ndupsamp = fread(HDR.FILE.FID,1,'uint16');
			%nrun = fread(HDR.FILE.FID,1,'uint16');
        		%err_detect = fread(HDR.FILE.FID,1,'uint16');
        		%nlost = fread(HDR.FILE.FID,1,'uint16');
        		nevents = tmp(9); %fread(HDR.FILE.FID,1,'uint16');
        		%fseek(HDR.FILE.FID,50,0);
	    
        		% Read events
        		for i = 1:nevents,
            			tmp = fread(HDR.FILE.FID,2,'uint8');
            			%cond_code = fread(HDR.FILE.FID,1,'uint8');
            			ev_code = fread(HDR.FILE.FID,1,'uint16');
				ev_cnt  = ev_cnt + 1;
            			tmp2.sample_offset = tmp(1) + (cnt-1)*128;
            			tmp2.cond_code     = tmp(2);
                            	tmp2.event_code    = ev_code;
                                if exist('OCTAVE_VERSION')<5, 	   
                                        ev{ev_cnt} = tmp2;
                                end;
    			end;
        		fseek(HDR.FILE.FID,4*(110-nevents)+2*nchans*block_size,0);
    		end
	end
	HDR.NRec = cnt;
    
        HDR.Events = ev;
	HDR.HeadLen = 0;
	HDR.FLAG.TRIGGERED = 1;	        
	HDR.FILE.POS = 0; 
	HDR.SPR = block_size;
	HDR.AS.bpb = HDR.SPR*HDR.NS*2;
	HDR.Dur = HDR.SPR/HDR.SampleRate;
		
	if CHAN==0,		
		HDR.SIE.InChanSelect = 1:HDR.NS;
	elseif all(CHAN>0 & CHAN<=HDR.NS),
		HDR.SIE.InChanSelect = CHAN;
	else
		fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
		fclose(HDR.FILE.FID); 
		HDR.FILE.FID = -1;	
		return;
	end;


elseif strcmp(HDR.TYPE,'LABVIEW'),
    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        HDR.FILE.OPEN=1;
        
        tmp = fread(HDR.FILE.FID,8,'uchar');
        HDR.VERSION = char(fread(HDR.FILE.FID,[1,8],'uchar'));
        HDR.AS.endpos = fread(HDR.FILE.FID,1,'int32'); % 4 first bytes = total header length
        
        HDR.HeadLen  = fread(HDR.FILE.FID,1,'int32'); % 4 first bytes = total header length
        HDR.NS       = fread(HDR.FILE.FID,1,'int32');  % 4 next bytes = channel list string length
        HDR.AS.endpos2 = fread(HDR.FILE.FID,1,'int32'); % 4 first bytes = total header length
        
        HDR.ChanList = fread(HDR.FILE.FID,HDR.NS,'uchar'); % channel string
        
        
        fclose(HDR.FILE.FID);
        HDR.FILE.FID = -1;
        
        return;

        %%%%% READ HEADER from Labview 5.1 supplied VI "create binary header"
        
        HDR.HeadLen  = fread(HDR.FILE.FID,1,'int32'); % 4 first bytes = total header length
        HDR.NS     = fread(HDR.FILE.FID,1,'int32');  % 4 next bytes = channel list string length
	HDR.ChanList = fread(HDR.FILE.FID,HDR.NS,'uchar'); % channel string
        
	% Number of channels = 1 + ord(lastChann) - ord(firstChann):
	HDR.LenN     = fread(HDR.FILE.FID,1,'int32'); % Hardware config length
	HDR.HWconfig = fread(HDR.FILE.FID,HDR.LenN,'uchar'); % its value
	HDR.SampleRate = fread(HDR.FILE.FID,1,'float32');
	HDR.InterChannelDelay = fread(HDR.FILE.FID,1,'float32');
	tmp=fread(HDR.FILE.FID,[1,HDR.HeadLen - ftell(HDR.FILE.FID)],'uchar'); % read rest of header
	[HDR.Date,tmp]= strtok(tmp,9) ; % date is the first 10 elements of this tmp array (strip out tab)
	[HDR.Time,tmp]= strtok(tmp,9); % and time is the next 8 ones
	% HDR.T0 = [yyyy mm dd hh MM ss];   %should be Matlab date/time format like in clock()
	HDR.Description= char(tmp); % description is the rest of elements.

	% Empirically determine the number of bytes per multichannel point:
	HDR.HeadLen = ftell(HDR.FILE.FID) ; 
	dummy10 = fread(HDR.FILE.FID,[HDR.NS,1],'int32');
	HDR.AS.bpb = (ftell(HDR.FILE.FID) - HDR.HeadLen); % hope it's an int !

	tmp = fseek(HDR.FILE.FID,0,'eof'); 
	HDR.AS.endpos = (ftell(HDR.FILE.FID) - HDR.HeadLen)/HDR.AS.bpb;
	fseek(HDR.FILE.FID,HDR.HeadLen,'bof'); 

	if CHAN==0,		
		HDR.SIE.InChanSelect = 1:HDR.NS;
	elseif all(CHAN>0 & CHAN<=HDR.NS),
		HDR.SIE.InChanSelect = CHAN;
	else
		fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
		fclose(HDR.FILE.FID); 
		HDR.FILE.FID = -1;	
		return;
	end;
	HDR.Cal = 1;
        

elseif strcmp(HDR.TYPE,'RG64'),
	if strncmp(HDR.FILE.Ext,'rhf'),
		FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext]);
	elseif strcmp(HDR.FILE.Ext,'rhd'),
		FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext(1:2),'f']);
	elseif strcmp(HDR.FILE.Ext,'RHD'),
		FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext(1:2),'F']);
	end;
    	HDR.FILE.FID = fopen(FILENAME,PERMISSION,'ieee-le');

	HDR.FILE.OPEN=1;
        HDR.IDCODE=fread(HDR.FILE.FID,4,'char');	%
	if strcmp(HDR.IDCODE')~='RG64' 
	    	fprintf(2,'\nError LOADRG64: %s not an RG64-File\n',FILENAME); 
	end; %end;

	tmp = fread(HDR.FILE.FID,2,'int32');
	HDR.VERSION = tmp(1)+tmp(2)/100;
	HDR.NS = fread(HDR.FILE.FID,1,'int32');
	HDR.SampleRate = fread(HDR.FILE.FID,1,'int32');
	HDR.SPR = fread(HDR.FILE.FID,1,'int32');
	AMPF = fread(HDR.FILE.FID,64,'int32');		
	fclose(HDR.FILE.FID);

	HDR.HeadLen = 0;
	HDR.PhysDim = 'uV';
	HDR.AS.endpos = HDR.SPR;
	HDR.AS.bpb    = HDR.NS*2;

	if strncmp(HDR.FILE.Ext,'rhd'),
		FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext]);
	elseif strcmp(HDR.FILE.Ext,'rhf'),
		FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext(1:2),'d']);
	elseif strcmp(HDR.FILE.Ext,'RHF'),
		FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext(1:2),'D']);
	end;
	HDR.FILE.FID=fopen(FILENAME,'rb','ieee-le');
	if HDR.FILE.FID<0,
		fprintf(2,'\nError LOADRG64: %s not found\n',FILENAME); 
		return;
	end;


	if CHAN==0,		
		HDR.SIE.InChanSelect = 1:HDR.NS;
	elseif all(CHAN>0 & CHAN<=HDR.NS),
		HDR.SIE.InChanSelect = CHAN;
	else
		fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
		fclose(HDR.FILE.FID); 
		HDR.FILE.FID = -1;	
		return;
	end;
	HDR.Cal = diag(AMPF(HDR.InChanSelect));
    

elseif strcmp(HDR.TYPE,'DDF'),

	% implementation of this format is not finished yet.
	fprintf(2,'Warning SOPEN: Implementing DASYLAB format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
	%HDR.FILE.FID = -1;
	%return;

	if any(PERMISSION=='r'),
		HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
		HDR.FILE.OPEN = 1;
		HDR.FILE.POS = 0;
		%HDR.ID = fread(HDR.FILE.FID,5,'char');
        	ds=fread(HDR.FILE.FID,[1,128],'char');
		HDR.ID = setstr(ds(1:4));
		DataSource = ds(6:length(ds));
		k = 0;
		while ~(any(ds==26)),
			ds = fread(HDR.FILE.FID,[1,128],'char');
			DataSource = [DataSource,ds];
			k = k+1;	
		end;	
		fseek(HDR.FILE.FID,find(ds==26)+k*128,'bof'); 	% position file identifier
		while ~isempty(DataSource),
			[ds,DataSource] = strtok(setstr(DataSource),[10,13]);
			[field,value] = strtok(ds,'=');
			if findstr(field,'SAMPLE RATE');
				[tmp1,tmp2] = strtok(value,'=');
				HDR.SampleRate = str2num(tmp1);
			elseif findstr(field,'DATA CHANNELS');
				HDR.NS = str2num(value);
			elseif findstr(field,'START TIME');
				Time = value;
			elseif findstr(field,'DATA FILE');
				HDR.FILE.DATA = value;
			end;			 	
		end;
		if 0;%DataSource(length(DataSource))~=26,
			fprintf(1,'Warning: DDF header seems to be incorrenct. Contact <alois.schloegl@tugraz.at> Subject: BIOSIG/DATAFORMAT/DDF  \n');
		end;
		CPUidentifier  = fread(HDR.FILE.FID,[1,2],'char');
		HDR.HeadLen(1) = fread(HDR.FILE.FID,1,'uint16');
		tmp = fread(HDR.FILE.FID,1,'uint16');
		if tmp == 0, HDR.GDFTYP = 4; 		% streamer format (data are raw data in WORD=UINT16)
		elseif tmp == 1, HDR.GDFTYP = 16; 	% Universal Format 1 (FLOAT)
		elseif tmp == 2, HDR.GDFTYP = 17; 	% Universal Format 2 (DOUBLE)
		elseif tmp <= 1000, % reserved
		else		% unused
		end;
		HDR.FILE.Type = tmp;
		HDR.VERSION = fread(HDR.FILE.FID,1,'uint16');
		HDR.HeadLen(2) = fread(HDR.FILE.FID,1,'uint16');	% second global Header
		HDR.HeadLen(3) = fread(HDR.FILE.FID,1,'uint16');	% size of channel Header
		fread(HDR.FILE.FID,1,'uint16');	% size of a block Header
		tmp = fread(HDR.FILE.FID,1,'uint16');
		if tmp ~= isfield(HDR.FILE,'DATA')
			fprintf(1,'Warning: DDF header seems to be incorrenct. Contact <alois.schloegl@tugraz.at> Subject: BIOSIG/DATAFORMAT/DDF  \n');
		end;
		HDR.NS = fread(HDR.FILE.FID,1,'uint16');
		HDR.Delay = fread(HDR.FILE.FID,1,'double');
		HDR.StartTime = fread(HDR.FILE.FID,1,'uint32');  % might be incorrect

		
		% it looks good so far. 
		
		if HDR.FILE.Type==0,
			% second global header
			fread(HDR.FILE.FID,1,'uint16')	% overall number of bytes in this header
			fread(HDR.FILE.FID,1,'uint16')	% number of analog channels
			fread(HDR.FILE.FID,1,'uint16')	% number of counter channels
			fread(HDR.FILE.FID,1,'uint16')	% number of digital ports
			fread(HDR.FILE.FID,1,'uint16')	% number of bits in each digital port
			fread(HDR.FILE.FID,1,'uint16')	% original blocksize when data was stored
			fread(HDR.FILE.FID,1,'uint32')	% sample number of the first sample (when cyclic buffer not activated, always zero
			fread(HDR.FILE.FID,1,'uint32')	% number of samples per channel

			% channel header
			for k = 1:HDR.NS,
				fread(HDR.FILE.FID,1,'uint16')	% number of bytes in this hedader
				fread(HDR.FILE.FID,1,'uint16')	% channel type 0: analog, 1: digital, 2: counter
				HDR.Label = setstr(fread(HDR.FILE.FID,[24,16],'char')');	% 
				tmp = fread(HDR.FILE.FID,1,'uint16')	% dataformat 0 UINT, 1: INT
				HDR.GDFTYP(k) = 3 + (~tmp);
				HDR.Cal(k) = fread(HDR.FILE.FID,1,'double');	% 
				HDR.Off(k) = fread(HDR.FILE.FID,1,'double');	% 
			end;    

		elseif HDR.FILE.Type==1,
			% second global header
			pos = ftell(HDR.FILE.FID);
			HeadLen = fread(HDR.FILE.FID,1,'uint16');	% size of this header 
			HDR.U1G.NS = fread(HDR.FILE.FID,1,'uint16');	% number of channels
			fread(HDR.FILE.FID,1,'uint16');	% multiplexed: 0=no, 1=yes
			fread(HDR.FILE.FID,[1,16],'uint16');	% array of channels collected on each input channel

			% channel header
			for k = 1:HDR.NS,
				fread(HDR.FILE.FID,1,'uint16');	% size of this header
				ch = fread(HDR.FILE.FID,1,'uint16');	% channel number
				HDR.DDF.MAXSPR(ch+1)= fread(HDR.FILE.FID,1,'uint16');	% maximum size of block in samples
				HDR.DDF.delay(ch+1) = fread(HDR.FILE.FID,1,'double');	% time delay between two samples
				HDR.DDF.ChanType(ch+1) = fread(HDR.FILE.FID,1,'uint16');	% channel type 
				HDR.DDF.ChanFlag(ch+1) = fread(HDR.FILE.FID,1,'uint16');	% channel flag 
				unused = fread(HDR.FILE.FID,2,'double');	% must be 0.0 for future extension
				%HDR.PhysDim{ch+1} = fgets(HDR.FILE.FID);	% channel unit
				%HDR.Label{ch+1} = fgets(HDR.FILE.FID);		% channel name 
			end;

			% channel header
			for k = 1:HDR.NS,
				fread(HDR.FILE.FID,[1,4],'char');
				fread(HDR.FILE.FID,1,'uint16');	% overall number of bytes in this header
				HDR.BlockStartTime = fread(HDR.FILE.FID,1,'uint32');  % might be incorrect
				unused = fread(HDR.FILE.FID,2,'double');	% must be 0.0 for future extension
				ch = fread(HDR.FILE.FID,1,'uint32');  % channel number
				
			end;    

		elseif HDR.FILE.Type==2,
			% second global header
			pos = ftell(HDR.FILE.FID);
			HeadLen = fread(HDR.FILE.FID,1,'uint16');	% size of this header 
			fread(HDR.FILE.FID,1,'uint16');	% number of channels
			fseek(HDR.FILE.FID, pos+HeadLen ,'bof');

			% channel header
			for k = 1:HDR.NS,
			pos = ftell(HDR.FILE.FID);
				HeadLen = fread(HDR.FILE.FID,1,'uint16');	% size of this header 
				HDR.DDF.Blocksize(k) = fread(HDR.FILE.FID,1,'uint16');	% 
				HDR.DDF.Delay(k) = fread(HDR.FILE.FID,1,'double');	% 
				HDR.DDF.chantyp(k) = fread(HDR.FILE.FID,1,'uint16');	% 
				HDR.FLAG.TRIGGER(k) = ~~fread(HDR.FILE.FID,1,'uint16');	
				fread(HDR.FILE.FID,1,'uint16');	
				HDR.Cal(k) = fread(HDR.FILE.FID,1,'double');	
			end;
		else
		end;

	end;


elseif strcmp(HDR.TYPE,'MIT')
	if any(PERMISSION=='r'),
		HDR.FileName = fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext]);

		HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-le');
		HDR.FILE.OPEN = 1;
		HDR.FILE.POS = 0;
		
		fid = HDR.FILE.FID;
		z = fgetl(fid);
		tmpfile = strtok(z,' /');
		if ~strcmp(file,tmpfile),
			fprintf(2,'Warning: RecordName %s does not fit filename %s\n',tmpfile,file);
		end;	

		%A = sscanf(z, '%*s %d %d %d',[1,3]);
		[tmp,z] = strtok(z); 
		[tmp,z] = strtok(z);
		HDR.NS  = str2num(tmp);   % number of signals
		[tmp,z] = strtok(z); 
		HDR.SampleRate = str2num(tmp);   % sample rate of data
		[tmp,z] = strtok(z,' ()');
		HDR.SPR   = str2num(tmp);   % sample rate of data
		HDR.NRec  = 1;

		for k=1:HDR.NS,
                        z = fgetl(fid);
                        [HDR.FILE.DAT,z]=strtok(z);
                        for k0 = 1:7,
				[tmp,z] = strtok(z);
				if k0 == 2,  
					% EC13*.HEA files have special gain values like "200(23456)/uV". 
 					[tmp, tmp1] = strtok(tmp,' ()');
				end;
				A(k0) = str2num(tmp); 
			end;
                        HDR.Label{k} = z; 
			dformat(k,1) = A(1);         % format; 
			HDR.gain(k,1) = A(2);              % number of integers per mV
			bitres(k,1) = A(3);            % bitresolution
			HDR.zerovalue(k,1)  = A(4);         % integer value of ECG zero point
			HDR.firstvalue(1,k) = A(5);        % first integer value of signal (to test for errors)
		end;
                z = char(fread(fid,[1,inf],'char'));
                ix1 = [findstr('AGE:',upper(z))+4; findstr('AGE>:',upper(z))+5];
                if ~isempty(ix1),
                        [tmp,z]=strtok(z(ix1(1):length(z)));
                        HDR.Patient.Age = str2num(tmp);
                end;
                ix1 = [findstr('SEX:',upper(z))+4, findstr('SEX>:',upper(z))+5];
                if ~isempty(ix1),
                        [HDR.Patient.Sex,z]=strtok(z(ix1(1):length(z)));
                end;
                ix1 = [findstr('DIAGNOSIS:',upper(z))+10; findstr('DIAGNOSIS>:',upper(z))+11];
                if ~isempty(ix1),
                        [HDR.Patient.Diagnosis,z]=strtok(z(ix1(1):length(z)),char([10,13,abs('#<>')]));
                end;
		ix1 = [findstr('MEDICATIONS:',upper(z))+12, findstr('MEDICATIONS>:',upper(z))+13];
                if ~isempty(ix1),
                        [HDR.Patient.Medication,z]=strtok(z(ix1(1):length(z)),char([10,13,abs('#<>')]));
                end;
                fclose(fid);
                
                if all(dformat==dformat(1)),
			HDR.VERSION = dformat(1);
		else
			fprintf(2,'different DFORMATs not supported.\n');
			HDR.FILE.FID = -1;
			return;
		end;
		
		if HDR.VERSION == 212, 
			HDR.AS.bpb = ceil(HDR.NS*3/2);
		elseif HDR.VERSION == 310, 
			HDR.AS.bpb = ceil(HDR.NS*2/3)*2;
		elseif HDR.VERSION == 311, 
			HDR.AS.bpb = ceil(HDR.NS/3)*4;
		elseif HDR.VERSION == 8, 
			HDR.AS.bpb = HDR.NS;
		elseif HDR.VERSION == 80, 
			HDR.AS.bpb = HDR.NS;
		elseif HDR.VERSION == 160, 
			HDR.AS.bpb = HDR.NS*2;
		elseif HDR.VERSION == 16, 
			HDR.AS.bpb = HDR.NS*2;
		elseif HDR.VERSION == 61, 
			HDR.AS.bpb = HDR.NS*2;
		end;
		HDR.Dur = 1/HDR.SampleRate;


	%------ LOAD ATTRIBUTES DATA ----------------------------------------------
		fid = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.atr']),'rb','ieee-le');
		if fid<0,
			A = []; c = 0;
		else
			[A,c] = fread(fid, [2, inf], 'uint8');
			fclose(fid);
			A = A';
		end;
	
		ATRTIME = zeros(c/2,1);
		ANNOT   = zeros(c/2,1);
		K = 0;
		i = 1;
		while i<=size(A,1),
    			annoth = bitshift(A(i,2),-2);
        		if annoth==59,
				K = K + 1;
		        	ANNOT(K) = bitshift(A(i+3,2),-2);
			        ATRTIME(K) = A(i+2,1)+bitshift(A(i+2,2),8)+ bitshift(A(i+1,1),16)+bitshift(A(i+1,2),24);
				i = i + 3;
	    		elseif annoth==60
		        	% nothing to do!
			elseif annoth==61
				% nothing to do!
			elseif annoth==62
			        % nothing to do!
			elseif annoth==63
				hilfe = bitshift(bitand(A(i,2),3),8)+A(i,1);
				hilfe = hilfe + mod(hilfe,2);
				i = i + hilfe / 2;
			else
				K = K+1;
			        ATRTIME(K) = bitshift(bitand(A(i,2),3),8)+A(i,1);
		        	ANNOT(K)   = bitshift(A(i,2),-2);
			end;
			i = i + 1;
		end;

		ANNOT   = ANNOT(1:K-1);    % last line = EOF (=0)
		ATRTIME = ATRTIME(1:K-1);  % last line = EOF

		clear A;
		HDR.ATRTIME  = (cumsum(ATRTIME))/HDR.SampleRate;
		%ind = find(ATRTIME <= size(signal,1)/HDR.SampleRate);
		%HDR.ATRTIMED = ATRTIME(ind);
		HDR.ANNOT    = round(ANNOT);
		%HDR.ANNOTD = ANNOT(ind);


	%------ LOAD BINARY DATA --------------------------------------------------
		if HDR.VERSION ==61,
			MACHINE_FOMRAT='ieee-be';
		else
			MACHINE_FOMRAT='ieee-le';
		end;
		%HDR.FILE.FID = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.dat']),'r','ieee-le');

		tmpfile = fullfile(HDR.FILE.Path,HDR.FILE.DAT);
		if  exist(tmpfile)~=2, 
			HDR.FILE.DAT = upper(HDR.FILE.DAT);
			tmpfile = fullfile(HDR.FILE.Path,HDR.FILE.DAT);
		end;
		if  exist(tmpfile)~=2, 
			HDR.FILE.DAT = lower(HDR.FILE.DAT);
			tmpfile = fullfile(HDR.FILE.Path,HDR.FILE.DAT);
		end;
		HDR.FILE.FID = fopen(tmpfile,'rb','ieee-le');
		
                HDR.HeadLen  = 0;
                fseek(HDR.FILE.FID,0,'eof');
                tmp = ftell(HDR.FILE.FID);
                fseek(HDR.FILE.FID,0,'bof');
                HDR.AS.endpos = tmp/HDR.AS.bpb;
                
		HDR.SIE.InChanSelect = 1:HDR.NS;
		FLAG_UCAL = HDR.FLAG.UCAL;	
		HDR.FLAG.UCAL = 1;
		[S,HDR] = eegread(HDR,1/HDR.SampleRate); % load 1st sample
                if any(S(1,:) ~= HDR.firstvalue), 
			fprintf(2,'ERROR SOPEN MIT-ECG: inconsistency in the first values\n'); 
		end;
                HDR.FLAG.UCAL = FLAG_UCAL ;	
                fseek(HDR.FILE.FID,0,'bof');	% reset file pointer
                HDR.FILE.POS = 0;
                
		if CHAN==0,		
			HDR.SIE.InChanSelect = 1:HDR.NS;
		elseif all(CHAN>0 & CHAN<=HDR.NS),
			HDR.SIE.InChanSelect = CHAN;
		else
			fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
			fclose(HDR.FILE.FID); 
			HDR.FILE.FID = -1;	
			return;
		end;
	end;


elseif strcmp(HDR.TYPE,'TMS32'),
	if any(PERMISSION=='r'),
		HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le')

		fprintf(HDR.FILE.stderr,'Format not tested yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',PERMISSION);	

		HDR.FILE.OPEN = 1;
		HDR.FILE.POS = 0;
		HDR.ID = fread(HDR.FILE.FID,31,'char');
		HDR.VERSION = fread(HDR.FILE.FID,1,'int16');
		[tmp,c] = fread(HDR.FILE.FID,81,'char');
		HDR.SampleRate = fread(HDR.FILE.FID,1,'int16');
		HDR.TMS32.StorageRate = fread(HDR.FILE.FID,1,'int16');
		HDR.TMS32.StorageType = fread(HDR.FILE.FID,1,'char');
		HDR.NS = fread(HDR.FILE.FID,1,'int16');
			NP = fread(HDR.FILE.FID,1,'int32');
			tmp  = fread(HDR.FILE.FID,1,'int32');
			Time = fread(HDR.FILE.FID,[1,7],'int16');
		HDR.T0   = Time([1:3,5:7]);
		HDR.NRec = fread(HDR.FILE.FID,1,'int32');
		HDR.SPR  = fread(HDR.FILE.FID,1,'uint16');
		HDR.AS.bpb = fread(HDR.FILE.FID,1,'uint16');
		tmp = fread(HDR.FILE.FID,1,'int16');
		tmp = fread(HDR.FILE.FID,64,'char');

		HDR.Label   = zeros(HDR.NS,40);
		HDR.PhysDim = zeros(HDR.NS,10);
		k = 0,
		while k <= HDR.NS,
			c   = fread(HDR.FILE.FID,[1,1],'uint8');
			tmp = fread(HDR.FILE.FID,[1,40],'char');
			if strcmp(tmp(1:4),'(Lo)') ;
				k = k + 1;
				HDR.Label(k,1:c-4) = tmp(5:c);
				HDR.GDFTYP(k) = 16;
			elseif strcmp(tmp(1:4),'(Hi)') ;
				HDR.NS = HDR.NS - 1;				
			else
				k = k+1;
				HDR.Label(k,1:c) = tmp(1:c);
				HDR.GDFTYP(k) = 5;
			end;
			
			tmp = fread(HDR.FILE.FID,[1,4],'char');

			c   = fread(HDR.FILE.FID,[1,1],'uint8');
			tmp = fread(HDR.FILE.FID,[1,10],'char');
			HDR.PhysDim(k,1:c) = tmp(1:c);

			HDR.PhysMin(k,1) = fread(HDR.FILE.FID,1,'float32');			
			HDR.PhysMax(k,1) = fread(HDR.FILE.FID,1,'float32');			
			HDR.DigMin(k,1) = fread(HDR.FILE.FID,1,'float32');			
			HDR.DigMax(k,1) = fread(HDR.FILE.FID,1,'float32');			
			tmp = fread(HDR.FILE.FID,2,'char');
			tmp = fread(HDR.FILE.FID,2,'char');
			tmp = fread(HDR.FILE.FID,60,'char');
		end;
		HDR.Cal = (HDR.PhysMax-HDR.PhysMin)./(HDR.DigMax-HDR.DigMin);
		HDR.Off = HDR.PhysMin - HDR.Cal .* HDR.DigMin;
		HDR.Calib = [HDR.Off';(diag(HDR.Cal))];
		HDR.HeadLen = 217 + HDR.NS*136;

		if CHAN==0,		
			HDR.SIE.InChanSelect = 1:HDR.NS;
		elseif all(CHAN>0 & CHAN<=HDR.NS),
			HDR.SIE.InChanSelect = CHAN;
		else
			fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
			fclose(HDR.FILE.FID); 
			HDR.FILE.FID = -1;	
			return;
		end;

	end;


elseif 0,strcmp(HDR.TYPE,'DAQ'),
        HDR = daqopen(HDR,PERMISSION,CHAN);


elseif strcmp(HDR.TYPE,'MAT4'),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb',HDR.MAT4.opentyp)

                fprintf(HDR.FILE.stderr,'Format not tested yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',PERMISSION);	

		HDR.FILE.OPEN = 1;
		HDR.FILE.POS = 0;
                k=0;NB=0;
                type = fread(HDR.FILE.FID,4,'uchar'); 	% 4-byte header
                while ~isempty(type),
                        type=str2num(char(abs(sprintf('%04i',sum(type(:).*[1;10;100;1000])))'));
                        k=k+1;
                        [mrows,c] = fread(HDR.FILE.FID,1,'uint32'); 	% tag, datatype
                        ncols = fread(HDR.FILE.FID,1,'uint32'); 	% tag, datatype
                        imagf = fread(HDR.FILE.FID,1,'uint32'); 	% tag, datatype
                        namelen  = fread(HDR.FILE.FID,1,'uint32'); 	% tag, datatype
                        [name,c] = fread(HDR.FILE.FID,namelen,'char'); 
                        
                        if imagf, HDR.ErrNo=-1; fprintf(HDR.FILE.stderr,'Warning %s: Imaginary data not tested\n',mfilename); end;
                        if type(1)==2,
                                HDR.ErrNo=-1;
                                fprintf(HDR.FILE.stderr,'Error %s: sparse data not supported\n',mfilename);
                        elseif type(1)>2, 
                                type(1)=bitand(type(1),1);
                        end;
                        
                        dt=type(3);
                        if     dt==0, SIZOF=8; TYP = 'float64';
                        elseif dt==6, SIZOF=1; TYP = 'uint8';
                        elseif dt==4, SIZOF=2; TYP = 'uint16';
                        elseif dt==3, SIZOF=2; TYP = 'int16';
                        elseif dt==2, SIZOF=4; TYP = 'int32';
                        elseif dt==1, SIZOF=4; TYP = 'float32';
                        else
                                fprintf(HDR.FILE.stderr,'Error %s: unknown data type\n',mfilename);
                        end;
                        
                        HDR.Var(k).Name  = char(name(1:length(name)-1)');
                        HDR.Var(k).Size  = [mrows,ncols];
                        HDR.Var(k).SizeOfType = SIZOF;
                        HDR.Var(k).Type  = [type;~~imagf]';
                        HDR.Var(k).TYP   = TYP;
                        HDR.Var(k).Pos   = ftell(HDR.FILE.FID);
                        
                        c=0; 
                        %% find the ADICHT data channels
                        if findstr(HDR.Var(k).Name,'data_block'),
                                HDR.ADI.DB(str2num(HDR.Var(k).Name(11:length(HDR.Var(k).Name))))=k;
                        elseif findstr(HDR.Var(k).Name,'ticktimes_block'),
                                HDR.ADI.TB(str2num(HDR.Var(k).Name(16:length(HDR.Var(k).Name))))=k;
                        end;
                        
                        tmp1=ftell(HDR.FILE.FID);
                        
                        % skip next block
                        tmp=(prod(HDR.Var(k).Size)-c)*HDR.Var(k).SizeOfType*(1+(~~imagf));
                        fseek(HDR.FILE.FID,tmp,0); 
                        
                        tmp2=ftell(HDR.FILE.FID);
                        if (tmp2-tmp1) < tmp,  % if skipping the block was not successful
                                HDR.ErrNo = -1;
                                HDR.ERROR = sprintf('file %s is corrupted',HDR.FileName);
                                fprintf(HDR.FILE.stderr,'Error SOPEN: MAT4 (ADICHT) file %s is corrupted\n',HDR.FileName);
                                return;
                        end;	                
                        
                        type = fread(HDR.FILE.FID,4,'uchar');  	% 4-byte header
                end;
        end;
        
        if isfield(HDR,'ADI')
                HDR.TYPE = 'ADI', % ADICHT-data, converted into a Matlab 4 file
        end;
        
        if strcmp(HDR.TYPE,'ADI'), 
                %% set internal sampling rate to 1000Hz (default). Set HDR.iFs=[] if no resampling should be performed 
                HDR.iFs = []; %1000;
                HDR.NS  = HDR.Var(HDR.ADI.DB(1)).Size(1);
                HDR.ADI.comtick = [];        
                HDR.ADI.comTick = [];        
                HDR.ADI.comtext = [];
                HDR.ADI.comchan = [];
                HDR.ADI.comblok = [];
                HDR.ADI.index   = [];
                HDR.ADI.range   = [];
                HDR.ADI.scale   = [];
                HDR.ADI.titles  = [];
                
                HDR.ADI.units   = [];
                
                for k=1:length(HDR.ADI.TB),
                        [HDR,t1] = matread(HDR,['ticktimes_block' int2str(k)],[1 2]);	% read first and second element of timeblock
                        [HDR,t2] = matread(HDR,['ticktimes_block' int2str(k)],HDR.Var(HDR.ADI.DB(k)).Size(2)); % read last element of timeblock
                        HDR.ADI.ti(k,1:2) = [t1(1),t2];
                        HDR.SampleRate(k) = round(1/diff(t1));
                        
                        [HDR,tmp] = matread(HDR,['comtick_block' int2str(k)]);	% read first and second element of timeblock
                        HDR.ADI.comtick = [HDR.ADI.comtick;tmp];
                        %HDR.ADI.comTick = [HDR.ADI.comTick;tmp/HDR.SampleRate(k)+HDR.ADI.ti(k,1)];
                        [HDR,tmp] = matread(HDR,['comchan_block' int2str(k)]);	% read first and second element of timeblock
                        HDR.ADI.comchan = [HDR.ADI.comchan;tmp];
                        [HDR,tmp] = matread(HDR,['comtext_block' int2str(k)]);	% read first and second element of timeblock
                        tmp2 = size(HDR.ADI.comtext,2)-size(tmp,2);
                        if tmp2>=0,
                                HDR.ADI.comtext = [HDR.ADI.comtext;[tmp,zeros(size(tmp,1),tmp2)]];
                        else
                                HDR.ADI.comtext = [[HDR.ADI.comtext,zeros(size(HDR.ADI.comtext,1),-tmp2)];tmp];
                        end;
                        HDR.ADI.comblok=[HDR.ADI.comblok;repmat(k,size(tmp,1),1)];
                        
                        [HDR,tmp] = matread(HDR,['index_block' int2str(k)]);	% read first and second element of timeblock
                        if isempty(tmp),
                                HDR.ADI.index{k} = 1:HDR.NS;
                        else
                                HDR.NS=length(tmp); %
                                HDR.ADI.index{k} = tmp;
                        end;
                        [HDR,tmp] = matread(HDR,['range_block' int2str(k)]);	% read first and second element of timeblock
                        HDR.ADI.range{k} = tmp;
                        [HDR,tmp] = matread(HDR,['scale_block' int2str(k)]);	% read first and second element of timeblock
                        HDR.ADI.scale{k} = tmp;
                        [HDR,tmp] = matread(HDR,['titles_block' int2str(k)]);	% read first and second element of timeblock
                        HDR.ADI.titles{k} = tmp;
                        
                        [HDR,tmp] = matread(HDR,['units_block' int2str(k)]);	% read first and second element of timeblock
                        HDR.ADI.units{k} = char(tmp);
                        if k==1;
                                HDR.PhysDim = char(sparse(find(HDR.ADI.index{1}),1:sum(HDR.ADI.index{1}>0),1)*HDR.ADI.units{1}); % for compatibility with the EDF toolbox
                        elseif any(size(HDR.ADI.units{k-1})~=size(tmp))
                                fprintf(2,'Warning MATOPEN: Units are different from block to block\n');
                        elseif any(any(HDR.ADI.units{k-1}~=tmp))
                                fprintf(2,'Warning MATOPEN: Units are different from block to block\n');
                        end;	
                        HDR.PhysDim = char(sparse(find(HDR.ADI.index{k}),1:sum(HDR.ADI.index{k}>0),1)*HDR.ADI.units{k}); % for compatibility with the EDF toolbox
                        %HDR.PhysDim=HDR.ADI.PhysDim;
                end;                
                HDR.T0 = datevec(datenum(1970,1,1)+HDR.ADI.ti(1,1)/24/3600);
                for k=1:size(HDR.ADI.comtext,1),
                        HDR.ADI.comtime0(k)=HDR.ADI.comtick(k)./HDR.SampleRate(HDR.ADI.comblok(k))'+HDR.ADI.ti(HDR.ADI.comblok(k),1)-HDR.ADI.ti(1,1);
                end;	
                
                % Test if timeindex is increasing
		tmp = size(HDR.ADI.ti,1);
                if ~all(HDR.ADI.ti(2:tmp,2)>HDR.ADI.ti(1:tmp-1,1)), 
                        HDR.ErrNo=-1;
                        fprintf(HDR.FILE.stderr,'Warning MATOPEN: Time index are not monotonic increasing !!!\n');
                        return;
                end;	
                % end of ADI-Mode
        else        
                fclose(HDR.FILE.FID);
                HDR.FILE.FID = -1;
        end;

        
elseif strcmp(HDR.TYPE,'ISHNE'),
	if any(PERMISSION=='r'),
		HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le')

		fprintf(HDR.FILE.stderr,'Format not tested yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',PERMISSION);	

		HDR.FILE.OPEN = 1;
		fseek(HDR.FILE.FID,10,'bof');
		HDR.variable_length_block = fread(HDR.FILE.FID,1,'int32');		
		HDR.SPR = fread(HDR.FILE.FID,1,'int32');		
		HDR.NRec= 1;
		HDR.offset_variable_length_block = fread(HDR.FILE.FID,1,'int32');
		HDR.HeadLen = fread(HDR.FILE.FID,1,'int32');		
		HDR.VERSION = fread(HDR.FILE.FID,1,'int16');		
		HDR.Patient.Name = fread(HDR.FILE.FID,80,'char');		
		%HDR.Surname = fread(HDR.FILE.FID,40,'char');		
		HDR.PID = fread(HDR.FILE.FID,20,'char');		
		HDR.Patient.Sex = fread(HDR.FILE.FID,1,'int16');		
		HDR.Patient.Race = fread(HDR.FILE.FID,1,'int16');		
		HDR.Patient.Birthday = fread(HDR.FILE.FID,3,'int16');		
		%HDR.Surname = fread(HDR.FILE.FID,40,'char')		
		Date = fread(HDR.FILE.FID,[1,3],'int16');		
		Date2 = fread(HDR.FILE.FID,[1,3],'int16');		
		Time = fread(HDR.FILE.FID,[1,3],'int16');		
		HDR.T0 = [Date([3,2,1]),Time];
		HDR.NS = fread(HDR.FILE.FID,1,'int16');		
		HDR.Lead.Specification = fread(HDR.FILE.FID,12,'int16');		
		HDR.Lead.Quality = fread(HDR.FILE.FID,12,'int16');		
		AmplitudeResolution = fread(HDR.FILE.FID,12,'int16');
		if any(HDR.Lead.AmplitudeResolution(HDR.NS+1:12)~=-9)
			fprintf(HDR.FILE.stderr,'Warning: AmplitudeResolution and Number of Channels %i do not fit.\n',HDR.NS);
			fclose(HDR.FILE.FID); 
			HDR.FILE.FID = -1;	
		end;

		HDR.PacemakerCode = fread(HDR.FILE.FID,1,'int16');		
		HDR.TypeOfRecorder = fread(HDR.FILE.FID,40,'char');		
		HDR.SampleRate = fread(HDR.FILE.FID,1,'int16');		
		HDR.Proprietary_of_ECG = fread(HDR.FILE.FID,80,'char');		
		HDR.Copyright = fread(HDR.FILE.FID,80,'char');		
		HDR.reserved1 = fread(HDR.FILE.FID,80,'char');		
		if ftell(HDR.FILE.FID)~=HDR.offset_variable_length_block,
			fprintf(HDR.FILE.stderr,'ERROR: length of fixed header does not fit %i %i \n',ftell(HDR.FILE.FID),HDR.offset_variable_length_block);
			fclose(HDR.FILE.FID); 
			HDR.FILE.FID = -1;	
			return;
		end;
		HDR.VariableHeader=fread(HDR.FILE.FID,HDR.variable_length_block,'char');	
		if ftell(HDR.FILE.FID)~=HDR.HeadLen,
			fprintf(HDR.FILE.stderr,'ERROR: length of variable header does not fit %i %i \n',ftell(HDR.FILE.FID),HDR.HeadLen);
			fclose(HDR.FILE.FID); 
			HDR.FILE.FID = -1;	
			return;
		end;

		if CHAN==0,		
			HDR.SIE.InChanSelect = 1:HDR.NS;
		elseif all(CHAN>0 & CHAN<=HDR.NS),
			HDR.SIE.InChanSelect = CHAN;
		else
			fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
			fclose(HDR.FILE.FID); 
			HDR.FILE.FID = -1;	
			return;
		end;
		
		HDR.Cal = eye(AmplitudeResolution(HDR.InChanSelect))/1000;
		HDR.PhysDim = 'uV';
		HDR.AS.bpb = 2*HDR.NS;
		HDR.AS.endpos = 8+2+512+HDR.variable_length_block+HDR.NS*2*HDR.SPR;
		HDR.FLAG.TRIGGERED = 0;	% Trigger Flag
		
	else
		fprintf(HDR.FILE.stderr,'PERMISSION %s not supported\n',PERMISSION);	
	end;			


elseif strncmp(HDR.TYPE,'SEG2',4),
	if any(PERMISSION,'rb'),
		if HDR.Endianity,
			HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-be')
		else
			HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le')
		end;

		HDR.FILE.OPEN = 1;
		HDR.FILE.POS  = 0;
		HDR.VERSION = fread(HDR.FILE.FID,1,'int16');
		HDR.HeadLen = fread(HDR.FILE.FID,1,'uint16');
		HDR.NS      = fread(HDR.FILE.FID,1,'uint16');
		HDR.SEG2.nsterm = fread(HDR.FILE.FID,1,'uint8'); 	% number of string terminator 
		HDR.SEG2.sterm  = fread(HDR.FILE.FID,2,'uchar'); 	% string terminator 
		HDR.SEG2.nlterm = fread(HDR.FILE.FID,1,'uint8'); 	% number of line terminator 
		HDR.SEG2.lterm  = fread(HDR.FILE.FID,2,'uchar'); 	% line terminator 
		HDR.SEG2.TraceDesc = fread(HDR.FILE.FID,HDR.NS,'uint32');

		% initialize date
		HDR.SEG2.blocksize = repmat(nan,HDR.NS,1);
		HDR.AS.bpb = repmat(nan,HDR.NS,1);
		HDR.AS.spb = repmat(nan,HDR.NS,1);
		HDR.SEG2.DateFormatCode = repmat(nan,HDR.NS,1);

		if ftell(HDR.FILE.FID) ~= HDR.HeadLen, 
			fprintf(HDR.FILE.stderr,'Warning SOPEN TYPE=SEG2: headerlength does not fit.\n');
		end; 

		optstrings = fread(HDR.FILE.FID,HDR.SEG2.TraceDesc(1)-HDR.Headlen,'uchar');
		
		id_tmp = fread(HDR.FILE.FID,1,'uint16');
		if id_tmp ~=hex2dec('4422')
			fprintf(HDR.FILE.stderr,'Error SOPEN TYPE=SEG2: incorrect trace descriptor block ID.\n');
		end;

		for k = 1:HDR.NS, 
			fseek(HDR.FILE.FID,HDR.SEG2.TraceDesc(k),'bof');
			HDR.SEG2.blocksize(k)  = fread(HDR.FILE.FID,1,'uint16');
			HDR.AS.bpb(k)  = fread(HDR.FILE.FID,1,'uint32');
			HDR.AS.spb(k)  = fread(HDR.FILE.FID,1,'uint32');
			HDR.SEG2.DateFormatCode(k) = fread(HDR.FILE.FID,1,'uchar');
	    		
			fseek(HDR.FILE.FID,32-13,'cof');
			%[tmp,c] = fread(HDR.FILE.FID,32-13,'char');	% reserved
			
			optstrings = fread(HDR.FILE.FID,HDR.SEG2.blocksize(k)-32,'uchar');
    		end; 
		
		fprintf(HDR.FILE.stderr,'Format %s not implemented yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',HDR.TYPE);	
		fclose(HDR.FILE.FID);
		HDR.FILE.FID = -1;
	end;		


elseif strncmp(HDR.TYPE,'SIGIF',5),
	if any(PERMISSION=='r'),
		HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
		HDR.FILE.OPEN = 1;
		HDR.FILE.POS  = 0;

		HDR.fingerprint=fgetl(HDR.FILE.FID);   % 1

		if length(HDR.fingerprint)>6
		        HDR.VERSION = int2str(HDR.fingerprint(7));
		else
	                HDR.VERSION = 1.1;
		end;        
		HDR.Comment=fgetl(HDR.FILE.FID);		% 2        
		HDR.SignalName=fgetl(HDR.FILE.FID);	% 3
		HDR.Date=fgetl(HDR.FILE.FID);		% 4 
		HDR.modifDate=fgetl(HDR.FILE.FID);	% 5

		[tmp1,tmp] = strtok(HDR.Date,'-/'); 
		HDR.T0     = zeros(1,6);
		HDR.T0(1)  = str2num(tmp1);
		if length(tmp1)<3, HDR.T0(1) = 1900+HDR.T0(1); end;
		[tmp1,tmp] = strtok(tmp,'-/'); 
		HDR.T0(2)  = str2num(tmp1);
		[tmp1,tmp] = strtok(tmp,'-/'); 
		HDR.T0(3)  = str2num(tmp1);

		HDR.SIG.Type   = fgetl(HDR.FILE.FID);		% 6 simultaneous or serial sampling
		    Source = fgetl(HDR.FILE.FID);		% 7 - obsolete
		HDR.NS     = str2num(fgetl(HDR.FILE.FID));  	% 8 number of channels
		HDR.NRec   = str2num(fgetl(HDR.FILE.FID)); % 9 number of segments
		    NFrames= str2num(fgetl(HDR.FILE.FID));  % 10 number of frames per segment - obsolete

		%HDR.SPR    = str2num(fgetl(HDR.FILE.FID));  			% 11 	number of samples per frame
		HDR.AS.spb  = str2num(fgetl(HDR.FILE.FID));  			% 11 	number of samples per frame
		H1.Bytes_per_Sample = str2num(fgetl(HDR.FILE.FID));	% 12 number of bytes per samples
		HDR.AS.bpb = HDR.AS.spb * H1.Bytes_per_Sample;
		HDR.Sampling_order   = str2num(fgetl(HDR.FILE.FID));  	% 13
		HDR.FLAG.INTEL_format = str2num(fgetl(HDR.FILE.FID));  	% 14
		HDR.FormatCode = str2num(fgetl(HDR.FILE.FID));  	% 15

		HDR.CompressTechnique = fgetl(HDR.FILE.FID);  		% 16
		HDR.SignalType = fgetl(HDR.FILE.FID);  			% 17

		for k=1:HDR.NS,
		        chandata = fgetl(HDR.FILE.FID);			% 18
		        [HDR.Label{k},chandata] = strtok(chandata,' ,;:');  
		        [tmp,chandata] = strtok(chandata,' ,;:');  
		        HDR.Cal(k) = str2num(tmp);  
        
		        [tmp,chandata] = strtok(chandata,' ,;:');
		        HDR.SampleRate(k) = str2num(tmp);
                
		        %[tmp,chandata] = strtok(chandata);  
		        HDR.Variable{k} = chandata;  
        
		        while  ~isempty(chandata)
    			        [tmp,chandata] = strtok(chandata,' ,;:'); 
		                if strcmp(tmp,'G')
            			        [HDR.PhysDim{k},chandata] = strtok(chandata,' ,;:');  
		                end;        
		        end;
		end;
		HDR.Segment_separator = fgetl(HDR.FILE.FID);  		% 19
		%HDR.Segment_separator = hex2dec(fgetl(HDR.FILE.FID));  

		HDR.FLAG.TimeStamp = str2num(fgetl(HDR.FILE.FID));  	% 20

		if HDR.VERSION>=3,
    			HDR.FLAG.SegmentLength = str2num(fgetl(HDR.FILE.FID));	% 21  
		        HDR.AppStartMark = fgetl(HDR.FILE.FID);  		% 22
		        HDR.AppInfo = fgetl(HDR.FILE.FID);  			% 23
		else
		        HDR.FLAG.SegmentLength = 0;    
		end;        
		HDR.footer = fgets(HDR.FILE.FID,6);			% 24

		if ~strcmp(HDR.footer,'oFSvAI')
			fprintf(2,['Warning LOADSIG in ' FILENAME ': Footer not found\n']);  
		end;

		if HDR.VERSION<2,
			HDR.FLAG.SegmentLength = 0;
		end;

		switch HDR.FormatCode,
		case 0; HDR.GDFTYP = 'uint16';
		case 3; HDR.GDFTYP = 'int16';  
			HDR.Segment_separator = hex2dec(HDR.Segment_separator([3:4,1:2]));
		case 5; HDR.GDFTYP = 'float';
		otherwise;
		        fprintf(2,'Warning LOADSIG: FormatCode %i not implemented\n',HDR.FormatCode);
		end;

		tmp = ftell(HDR.FILE.FID);
		if ~HDR.FLAG.INTEL_format,
			fclose(HDR.FILE.FID);
	    		HDR.FILE.FID = fopen(HDR.FileName,'rt','ieee-be');
			fseek(HDR.FILE.FID,tmp,'bof');
		end;
		HDR.HeadLen = tmp + HDR.FLAG.TimeStamp*9;
	
		if ~HDR.NRec, HDR.NRec = inf; end;
		k = 0;
		while (k < HDR.NRec) & ~feof(HDR.FILE.FID),
			k = k+1;
			HDR.Block.Pos(k) = ftell(HDR.FILE.FID);
			if HDR.FLAG.TimeStamp,
				HDR.Frame(k).TimeStamp = fread(HDR.FILE.FID,[1,9],'char');
			end;
			
			if HDR.FLAG.SegmentLength,
		        	HDR.Block.Length(k) = fread(HDR.FILE.FID,1,'uint16');  %#26
				fseek(HDR.FILE.FID,HDR.Block.Length(k)*H1.Bytes_per_Sample,'cof');
			else
				tmp = HDR.Segment_separator-1;
				count = 0;
				data  = [];
				dat   = [];
				while ~(any(dat==HDR.Segment_separator));
					[dat,c] = fread(HDR.FILE.FID,[HDR.NS,1024],HDR.GDFTYP);
					count   = count + c;
				end;
				tmppos = min(find(dat(:)==HDR.Segment_separator));
				HDR.Block.Length(k) = count - c + tmppos;
			end;
		end;
		HDR.SPR = HDR.Block.Length/HDR.NS;
		HDR.Dur = max(HDR.SPR./HDR.SampleRate);
		HDR.NRec = k; 

		if HDR.FLAG.TimeStamp,
			tmp=char(HDR.Frame(1).TimeStamp);
			HDR.T0(4) = str2num(tmp(1:2));
			HDR.T0(5) = str2num(tmp(3:4));
			HDR.T0(6) = str2num([tmp(5:6),'.',tmp(7:9)]);
		end;

		if CHAN==0,		
			HDR.SIE.InChanSelect = 1:HDR.NS;
		elseif all(CHAN>0 & CHAN<=HDR.NS),
			HDR.SIE.InChanSelect = CHAN;
		else
			fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
			fclose(HDR.FILE.FID); 
			HDR.FILE.FID = -1;	
			return;
		end;
	end;
else
	%fprintf(2,'SOPEN does not support your data format yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
	HDR.FILE.FID = -1;	% this indicates that file could not be opened. 
        return;
end;

