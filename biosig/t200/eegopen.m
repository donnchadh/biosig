function [HDR,H1,h2] = eegopen(arg1,PERMISSION,CHAN,MODE,arg5,arg6)
% Opens EEG files for reading and writing. 
% The following data formats are supported: EDF, BKR, CNT, BDF, GDF
%
% HDR = eegopen(Filename,PERMISSION, [, CHAN [, MODE]]);
% [S,HDR] = eegread(HDR, NoR, StartPos)
%
% PERMISSION is one of the following strings 
%	'r'	read header
%       'r+'	writes header of an existing file
%	'w'	write header
%
% CHAN defines the selected Channels
% MODE	'UCAL'	uncalibrated 
%
% HDR contains the Headerinformation and internal data
% S 	returns the EEG data format
%
% see also: EEGOPEN, EEGREAD, EEGSEEK, EEGTELL, EEGCLOSE, EEGWRITE, EEGEOF


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

%	$Revision: 1.21 $
%	$Id: eegopen.m,v 1.21 2003-06-09 19:00:20 schloegl Exp $
%	(C) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


if nargin<2, PERMISSION = 'r'; end; 
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

if exist(HDR.FileName)==2,
	fid = fopen(HDR.FileName,'r');
	if fid>0,
		[s,c] = fread(fid,[1,32],'uchar');
		s = char(s);
                if c,
                        if strcmp(char(s),['MATLAB Data Acquisition File.' 0 25 0]);% Matlab Data Acquisition File 
		                HDR.TYPE='DAQ';
                        elseif strncmp(s,'0       ',8); 
                                HDR.TYPE='EDF';
                        elseif all(s(1:8)==[255,abs('BIOSEMI')]); 
                                HDR.TYPE='BDF';
                        elseif strncmp(s,'GDF',3); 
                                HDR.TYPE='GDF';
                        elseif strncmp(s,'Version ',8); 
                                HDR.TYPE='CNT';
                        elseif strncmp(s,'ISHNE1.0',8);	% ISHNE Holter standard output file.
                                HDR.TYPE='ISHNE';
                        elseif strncmp(s,'POLY_SAM',8);	% Poly5/TMS32 sample file format.
                                HDR.TYPE='TMS32';
                        elseif strncmp(s,'"Snap-Master Data File"',23);	% Snap-Master Data File .
                                HDR.TYPE='SMA';
                        elseif s(1)==207; 
                                HDR.TYPE='BKR';
                        elseif strncmp(s,HDR.FILE.Name,length(HDR.FILE.Name)); 
                                HDR.TYPE='MIT';
                        elseif strncmp(s,'RG64',4); 
                                HDR.TYPE='RG64';
                        elseif strncmp(s,'DTDF',4); 
                                HDR.TYPE='DDF';
                        elseif strncmp(s,['RSRC',10,13,0,3],8); %strncmp(s,'RSRC',4); 
                                HDR.TYPE='LABVIEW';
                        elseif strncmp(s,'IAvSFo',6); 
                                HDR.TYPE='SIG';
                        elseif any(s(4)==(2:7)) & all(s(1:3)==0); % [int32] 2...7
                                HDR.TYPE='EGI';
                        elseif strncmp(s,'rhdE',4);	% Holter Excel 2 file, not supported yet. 
                                HDR.TYPE='rhdE';          
			elseif any(s(3:6)*(2.^[0;8;16;24]) == (30:40))
				HDR.TYPE='ACQ';
			elseif all(s(1:2)==[hex2dec('55'),hex2dec('AA')]);
				HDR.TYPE='RDF';

			elseif all(s(1:2)==[hex2dec('55'),hex2dec('3A')]); % little endian 
				HDR.TYPE='SEG2 l';
			elseif all(s(1:2)==[hex2dec('3A'),hex2dec('55')]); % big endian 
				HDR.TYPE='SEG2 b';
					% SEG2 format specification: ftp://diftp.epfl.ch/pub/detec/doc/seg2.pdf

                        else
                                %TYPE='unknown';
                        end;
                end;
                fclose(fid);
	end;
end;

if ~isfield(HDR,'TYPE'),
       HDR.TYPE = upper(FileExt(2:length(FileExt)));
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



if strcmp(HDR.TYPE,'EDF'),
        HDR = sdfopen(HDR,PERMISSION,CHAN);
	HDR.FLAG.TRIGGERED = 0;	% Trigger Flag
        
elseif strcmp(HDR.TYPE,'BDF'),
        HDR = sdfopen(HDR,PERMISSION,CHAN);
	HDR.FLAG.TRIGGERED = 0;	% Trigger Flag
        
elseif strcmp(HDR.TYPE,'GDF'),
        HDR = sdfopen(HDR,PERMISSION,CHAN);
        
elseif strcmp(HDR.TYPE,'BKR'),
    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        if HDR.FILE.FID<=0,
                fprintf(HDR.FILE.stderr,'EEGOPEN: BKR-File %s couldnot be opened\n',HDR.FileName);
                return;
        end;        

	if strcmp(PERMISSION,'r'),
		HDR = bkropen(HDR,'r',CHAN);
        
        elseif strcmp(PERMISSION,'r+'),
		HDR = eegchkhdr(HDR);
                HDR.FILE.OPEN = 2;	        
                HDR = bkropen(HDR,'r+',CHAN);
        
        elseif strcmp(PERMISSION,'w'),
		HDR = eegchkhdr(HDR);
                HDR.FILE.OPEN = 2;	        
                HDR = bkropen(HDR,'w',CHAN);
        
        elseif strcmp(PERMISSION,'w+'),
		if feof(HDR.FILE.FID),
			HDR = eegchkhdr(HDR);
                	HDR = bkropen(HDR,'r+',CHAN);
		else
                	HDR = bkropen(HDR,'r',CHAN);
        	end;
       	        HDR.FILE.OPEN = 2;
		
        elseif strcmp(PERMISSION,'a'),
       	        HDR.FILE.OPEN = 3;
		HDR.FILE.POS  = HDR.AS.endpos;
        
        elseif strcmp(PERMISSION,'a+'),
		if ftell(HDR.FILE.FID),
        		fseek(HDR.FILE.FID,0,'bof');
			HDR = bkropen(HDR,'r',CHAN);
			HDR = eegseek(HDR, 0,'eof');
		else
			HDR = eegchkhdr(HDR);
                	HDR = bkropen(HDR,'w',CHAN);
		end;
       	        HDR.FILE.OPEN = 3;	        
	else
		fprintf(HDR.FILE.stderr,'PERMISSION %s not supported\n',PERMISSION);	
        end;
        

elseif strmatch(HDR.TYPE,{'CNT','AVG','EEG'}),
	if strcmp(PERMISSION,'r'),
	        [HDR,H1,h2] = cntopen(HDR,'r',CHAN);
	else
		fprintf(HDR.FILE.stderr,'PERMISSION %s not supported\n',PERMISSION);	
        end;
        

elseif strcmp(HDR.TYPE,'ACQ'),
    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        if HDR.FILE.FID <= 0,
                fprintf(HDR.FILE.stderr,'EEGOPEN TYPE=ACQ: File %s couldnot be opened\n',HDR.FileName);
                return;
        end;        

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
        	DHR.ChanSel(k) = fread(HDR.FILE.FID,1,'int16');
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
	
        
elseif strcmp(HDR.TYPE,'EGI'),
    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        if HDR.FILE.FID <= 0,
                fprintf(HDR.FILE.stderr,'EEGOPEN TYPE=EGI: File %s couldnot be opened\n',HDR.FileName);
                return;
        end;        
        
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
       
        
elseif strcmp(HDR.TYPE,'LDR'),
        HDR = openldr(HDR,PERMISSION);      

        
elseif strcmp(HDR.TYPE,'SMA'),  % under constructions
    	HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        if HDR.FILE.FID <= 0,
                fprintf(HDR.FILE.stderr,'EEGOPEN: SMA-File %s couldnot be opened\n',HDR.FileName);
                return;
        end;        
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
                end
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
                                fprintf(HDR.FILE.stderr,'Error EEGOPEN type=SMA: Sync byte is not "AA"\n');
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
                fprintf(HDR.FILE.stderr,'Warning EEGOPEN TYPE=SMA: Header information does not fit size of file\n');
                fprintf(HDR.FILE.stderr,'\tProbably more than one data segment - this is not supported in the current version of EEGOPEN\n');
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
        if HDR.FILE.FID <= 0,
                fprintf(HDR.FILE.stderr,'EEGOPEN: RDF-File %s couldnot be opened\n',HDR.FileName);
                return;
        end;        
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
            			ev(ev_cnt).sample_offset = tmp(1) + (cnt-1)*128;
            			ev(ev_cnt).cond_code     = tmp(2);
            			ev(ev_cnt).event_code    = ev_code;
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
        if HDR.FILE.FID<=0,
                fprintf(HDR.FILE.stderr,'EEGOPEN: File %s couldnot be opened\n',HDR.FileName);
                return;
        end;        
	HDR.FILE.OPEN=1;

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
        if HDR.FILE.FID<=0,
                fprintf(HDR.FILE.stderr,'EEGOPEN: File %s couldnot be opened\n',HDR.FileName);
                return;
        end;        
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
	HDR.FILE.FID=fopen(FILENAME,'r','ieee-le');
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
	fprintf(2,'EEGOPEN does not support DASYLAB format yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
	HDR.FILE.FID = -1;
	return;

	if strcmp(PERMISSION,'r'),
		HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-le')
		if HDR.FILE.FID < 0,
			fprintf(HDR.FILE.stderr,'Error: file %s not found.\n',HDR.FileName);
			return;
		end;
		HDR.FILE.OPEN = 1;
		HDR.FILE.POS = 0;
		HDR.ID = fread(HDR.FILE.FID,5,'char');
        	ds=fgets(HDR.FILE.FID);
		DataSource = ds;
		k = 1;
		while ~(any(ds==26)),
			ds = fgets(HDR.FILE.FID);
			DataSource = [DataSource,ds];
			[field,value] = strtok(ds,'=');
			if findstr(field,'SAMPLE RATE');
				[tmp1,tmp2] = strtok(value);
				HDR.SampleRate = str2num(tmp1);
			elseif findstr(field,'DATA CHANNELS');
				HDR.NS = str2num(value);
			elseif findstr(field,'START TIME');
				Time = value;
			elseif findstr(field,'DATA FILE');
				HDR.FILE.DATA = value;
			end;			 	
			k = k+1;	
		end;	
		if DataSource(length(DataSource))~=26,
			fprintf(1,'Warning: DDF header seems to be incorrenct. Contact <alois.schloegl@tugraz.at> Subject: BIOSIG/DATAFORMAT/DDF  \n');
		end;
		CPUidentifier = fread(HDR.FILE.FID,2,'char');
		HDR.HeadLen = fread(HDR.FILE.FID,1,'uint16');
		tmp = fread(HDR.FILE.FID,1,'uint16');
		if tmp == 0, HDR.GDFTYP = 3; 
		elseif tmp == 1, HDR.GDFTYP = 16; 
		elseif tmp == 2, HDR.GDFTYP = 17; 
		elseif tmp <= 1000, % reserved
		else		% unused
		end;
		HDR.VERSION = fread(HDR.FILE.FID,1,'uint16');
		HDR.HeadLen = HDR.HeadLen + fread(HDR.FILE.FID,1,'uint16');
		HDR.HeadLen = HDR.HeadLen + fread(HDR.FILE.FID,1,'uint16');
		tmp = fread(HDR.FILE.FID,1,'uint16');
		if tmp ~= isfield(HDR.FILE.DATA)
			fprintf(1,'Warning: DDF header seems to be incorrenct. Contact <alois.schloegl@tugraz.at> Subject: BIOSIG/DATAFORMAT/DDF  \n');
		end;
		HDR.NS = fread(HDR.FILE.FID,1,'uint16');
		HDR.Delay = fread(HDR.FILE.FID,1,'double');
		Datum = fread(HDR.FILE.FID,7,'uint16');  % might be incorrect
	end;


elseif strcmp(HDR.TYPE,'MIT')
	if strcmp(PERMISSION,'r'),

		HDR.FileName = fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext]);

		HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-le');
		if HDR.FILE.FID < 0,
			fprintf(HDR.FILE.stderr,'Error: file %s not found.\n',HDR.FileName);
			return;
		end;
		HDR.FILE.OPEN = 1;
		HDR.FILE.POS = 0;
		
		fid = HDR.FILE.FID;
		z = fgetl(fid);
		if ~strcmp(file,strtok(z,' /')),
			fprintf(2,'Warning: RecordName %s does not fit filename %s\n',strtok(z,' /'),file);
		end;	

		%A = sscanf(z, '%*s %d %d %d',[1,3]);
		[tmp,z] = strtok(z); 
		[tmp,z] = strtok(z);
		HDR.NS = str2num(tmp);   % number of signals
		[tmp,z] = strtok(z); 
		HDR.SampleRate = str2num(tmp);   % sample rate of data
		[tmp,z] = strtok(z,' ()'); 
		HDR.SPR   = str2num(tmp);   % sample rate of data
		HDR.NRec  = 1;

		for k=1:HDR.NS,
                        z = fgetl(fid);
                        [HDR.FILE.DAT,z]=strtok(z);
                        [A,count,errmsg,nextidx] = sscanf(z, '%d %d %d %d %d %d %d ',[1,7]);
                        HDR.Label{k}=z(nextidx:length(z));
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
		fid = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.atr']),'r','ieee-le');
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
		HDR.FILE.FID = fopen(fullfile(HDR.FILE.Path,HDR.FILE.DAT),'r','ieee-le');
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
			fprintf(2,'ERROR EEGOPEN MIT-ECG: inconsistency in the first bit values'); 
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
	if strcmp(PERMISSION,'r'),
		HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-le')
		if HDR.FILE.FID < 0,
			fprintf(HDR.FILE.stderr,'Error: file %s not found.\n',HDR.FileName);
			return;
		else
			fprintf(HDR.FILE.stderr,'Format not tested yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',PERMISSION);	
		end;
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
        
	        
elseif strcmp(HDR.TYPE,'ISHNE'),
	if strcmp(PERMISSION,'r'),
		HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-le')
		if HDR.FILE.FID < 0,
			fprintf(HDR.FILE.stderr,'Error: file %s not found.\n',HDR.FileName);
			return;
		else
			fprintf(HDR.FILE.stderr,'Format not tested yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',PERMISSION);	
		end;
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
	if strcmp(PERMISSION,'r'),

		if strcmp(HDR.TYPE,'SEG2 l'),
			HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-le')
		elseif strcmp(HDR.TYPE,'SEG2 l'),
			HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-be')
		end;
		HDR.TYPE = 'SEG2'; % remove endian indicator  
		if HDR.FILE.FID < 0,
			fprintf(HDR.FILE.stderr,'Error: file %s not found.\n',HDR.FileName);
			return;
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
			fprintf(HDR.FILE.stderr,'Warning EEGOPEN TYPE=SEG2: headerlength does not fit.\n');
		end; 

		optstrings = fread(HDR.FILE.FID,HDR.SEG2.TraceDesc(1)-HDR.Headlen,'uchar');
		
		id_tmp = fread(HDR.FILE.FID,1,'uint16');
		if id_tmp ~=hex2dec('4422')
			fprintf(HDR.FILE.stderr,'Error EEGOPEN TYPE=SEG2: incorrect trace descriptor block ID.\n');
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
		
else
	%fprintf(2,'EEGOPEN does not support your data format yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
	HDR.FILE.FID = -1;	% this indicates that file could not be opened. 
        return;

end;

