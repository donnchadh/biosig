function [HDR,H1,h2] = sopen(arg1,PERMISSION,CHAN,MODE,arg5,arg6)
% SOPEN opens signal files for reading and writing and returns 
%       the header information. Many different data formats are supported.
%
% HDR = sopen(Filename, PERMISSION, [, CHAN [, MODE]]);
% [S,HDR] = sread(HDR, NoR, StartPos);
% HDR = sclose(HDR);
%
% PERMISSION is one of the following strings 
%	'r'	read header
%	'w'	write header
%
% CHAN defines a list of selected Channels
%   Alternative CHAN can be also a Re-Referencing Matrix ReRefMx
%       (i.e. a spatial filter). 
%   E.g. the following command returns the difference and 
%       the mean of the first two channels. 
%   HDR = sopen(Filename, 'r', [[1;-1],[.5,5]]);
%   [S,HDR] = sread(HDR, Duration, Start);
%   HDR = sclose(HDR);
%
% MODE  'UCAL'  uncalibrated data
%       'OVERFLOWDETECTION:OFF' turns off automated overflow detection
%       Several options can be concatenated within MODE. 
%
% HDR contains the Headerinformation and internal data
% S 	returns the signal data 
%
% Several files can be loaded at once with SLOAD
%
% see also: SLOAD, SREAD, SSEEK, STELL, SCLOSE, SWRITE, SEOF


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

%	$Revision: 1.95 $
%	$Id: sopen.m,v 1.95 2005-02-22 09:55:09 schloegl Exp $
%	(C) 1997-2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


if isnan(str2double('1, 3'));
        fprintf(2,'Warning BIOSIG: incorrect version of STR2DOUBLE.\n');
        fprintf(2,'- Its recommended to update STR2DOUBLE. Contact Alois!\n');
end;

global FLAG_NUMBER_OF_OPEN_FIF_FILES;

if ischar(arg1),
        HDR.FileName = arg1;
%elseif length(arg1)~=1,
%	HDR = [];
elseif isfield(arg1,'name')
        HDR.FileName = arg1.name;
	HDR.FILE = arg1; 
else %if isfield(arg1,'FileName')
        HDR = arg1;
%else
%	HDR = [];
end;


if nargin<2, 
        PERMISSION='r'; 
elseif isempty(PERMISSION),
        PERMISSION='r'; 
elseif isnumeric(PERMISSION),
        fprintf(HDR.FILE.stderr,'Warning SOPEN: second argument should be PERMISSION, assume its the channel selection\n');
        CHAN = PERMISSION; 
        PERMISSION = 'r'; 
elseif ~any(PERMISSION(1)=='RWrw'),
        fprintf(HDR.FILE.stderr,'Warning SOPEN: PERMISSION must be ''r'' or ''w''. Assume PERMISSION is ''r''\n');
        PERMISSION = 'r'; 
end;
if ~any(PERMISSION=='b');
        PERMISSION = [PERMISSION,'b']; % force binary open. Needed for Octave
end;

if nargin<3, CHAN = 0; end; 
if all(size(CHAN)>1) | any(floor(CHAN)~=CHAN) | (any(CHAN<0) & prod(size(CHAN))>1),
        ReRefMx = CHAN; 
        CHAN = find(any(CHAN,2));
end

if nargin<4, MODE = ''; end;
if isempty(MODE), MODE=' '; end;	% Make sure MODE is not empty -> FINDSTR

% test for type of file 
if any(PERMISSION=='r'),
        HDR = getfiletype(HDR);
	if HDR.ERROR.status, 
		fprintf(HDR.FILE.stderr,'%s\n',HDR.ERROR.message);
		return;
	end;
else
	[pfad,file,FileExt] = fileparts(HDR.FileName);
	HDR.FILE.Name = file;
	HDR.FILE.Path = pfad;
	HDR.FILE.Ext  = FileExt(2:length(FileExt));
	if ~isfield(HDR.FILE,'stderr'),
	        HDR.FILE.stderr = 2;
	end;
	if ~isfield(HDR.FILE,'stdout'),
	        HDR.FILE.stdout = 1;
	end;	
	HDR.FILE.OPEN = 0;
        HDR.FILE.FID  = -1;
	HDR.ERROR.status  = 0; 
	HDR.ERROR.message = ''; 
end;

%% Initialization
if ~isfield(HDR,'NS');
        HDR.NS = NaN; 
end;
if ~isfield(HDR,'SampleRate');
        HDR.SampleRate = NaN; 
end;
if ~isfield(HDR,'Label');
        HDR.Label = []; 
end;
if ~isfield(HDR,'PhysDim');
        HDR.PhysDim = ''; 
end;
if ~isfield(HDR,'T0');
        HDR.T0 = repmat(nan,1,6);
end;
if ~isfield(HDR,'Filter');
        HDR.Filter.LowPass  = NaN; 
        HDR.Filter.HighPass = NaN; 
end;
if ~isfield(HDR,'FLAG');
        HDR.FLAG.UCAL = ~isempty(strfind(MODE,'UCAL'));   % FLAG for UN-CALIBRATING
        HDR.FLAG.FILT = 0; 	% FLAG if any filter is applied; 
        HDR.FLAG.TRIGGERED = 0; % the data is untriggered by default
        HDR.FLAG.OVERFLOWDETECTION = isempty(strfind(upper(MODE),'OVERFLOWDETECTION:OFF'));
end;
if ~isfield(HDR,'EVENT');
        HDR.EVENT.TYP = []; 
        HDR.EVENT.POS = []; 
end;

if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'GDF') | strcmp(HDR.TYPE,'BDF'),
        if any(PERMISSION=='w');
                HDR = eegchkhdr(HDR);
        end;
        if nargin<4,
                HDR = sdfopen(HDR,PERMISSION,CHAN);
        else
                HDR = sdfopen(HDR,PERMISSION,CHAN,MODE);
        end;

	%%% Event file stored in GDF-format
	if ~any([HDR.NS,HDR.NRec,~length(HDR.EVENT.POS)]);
		HDR.TYPE = 'EVENT';
	end;	
        

elseif strcmp(HDR.TYPE,'EVENT') & any(lower(PERMISSION)=='w'),
	%%% Save event file in GDF-format
	HDR.TYPE = 'GDF';
	HDR.NS   = 0; 
	HDR.NRec = 0; 
	if any(isnan(HDR.T0))
		HDR.T0 = clock;
	end;
        HDR = sdfopen(HDR,'w');
	HDR = sclose(HDR);
	HDR.TYPE = 'EVENT';


elseif strcmp(HDR.TYPE,'BKR'),
        HDR = bkropen(HDR,PERMISSION,CHAN);
        %%% Get trigger information from BKR data 

        
elseif strmatch(HDR.TYPE,['CNT';'AVG';'EEG']),
        if any(PERMISSION=='r');
                [HDR,H1,h2] = cntopen(HDR,PERMISSION,CHAN);
                if ~isfield(HDR,'GDFTYP'), HDR.GDFTYP='int16'; end; 

        elseif any(PERMISSION=='w');
                % check header information
                if ~isfield(HDR,'NS'),
                        HDR.NS = 0;
                end;
                if ~isfinite(HDR.NS) | (HDR.NS<0)
                        fprintf(HDR.FILE.stderr,'Error SOPEN CNT-Write: HDR.NS not defined\n');
                        return;
                end;	
                if ~isfield(HDR,'SPR'),
                        HDR.SPR = 0;
                end;
                if ~isfinite(HDR.SPR)
                        HDR.SPR = 0;
                end;	
                type = 2;
                if strmatch(HDR.TYPE,'EEG'), type = 1;
                elseif strmatch(HDR.TYPE,'AVG'), type = 0;
                end;
                
                if ~isfield(HDR,'PID')
                        HDR.PID = char(repmat(32,1,20));
                elseif prod(size(HDR.PID))>20,
                        HDR.PID = HDR.PID(1:20);
                else 
                        HDR.PID = [HDR.PID(:)',repmat(32,1,20-length(HDR.PID(:)))];
                        %HDR.PID = [HDR.PID,repmat(32,1,20-length(HDR.PID))];
                end;
                
                if ~isfield(HDR,'Label')
                        HDR.Label = int2str((1:HDR.NS)');
                elseif iscell(HDR.Label),
                        HDR.Label = cat(1,HDR.Label);
                end;
                if size(HDR.Label,2)>10,
                        HDR.Label = HDR.Label(:,1:10);
                elseif size(HDR.Label,2)<10, 
                        HDR.Label = [HDR.Label,repmat(32,HDR.NS,10-size(HDR.Label,2))];
                end;
                
                if ~isfield(HDR,'Calib')
                        HDR.Cal = ones(HDR.NS,1);
                        e.sensitivity = ones(HDR.NS,1)*204.8;
                        HDR.Off = zeros(HDR.NS,1);
                else
                        HDR.Cal = diag(HDR.Calib(2:end,:));
                        e.sensitivity = ones(HDR.NS,1)*204.8;
                        HDR.Off = round(HDR.Calib(1,:)'./HDR.Cal);
                end;
                
                % open file 
                HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
                if HDR.FILE.FID < 0,
                        return;
                end;
                HDR.FILE.OPEN = 2; 
                if any([HDR.SPR] <= 0);
                        HDR.FILE.OPEN = 3; 
                end;
                
                % write fixed header
                fwrite(HDR.FILE.FID,'Version 3.0','char');
                fwrite(HDR.FILE.FID,zeros(2,1),'uint32');
                fwrite(HDR.FILE.FID,type,'uchar');
                fwrite(HDR.FILE.FID,HDR.PID,'uchar');
                
                fwrite(HDR.FILE.FID,repmat(0,1,900-ftell(HDR.FILE.FID)),'uchar')
                
                % write variable header
                for k = 1:HDR.NS,
                        count = fwrite(HDR.FILE.FID,HDR.Label(k,:),'uchar');
                        count = fwrite(HDR.FILE.FID,zeros(5,1),'uchar');
                        count = fwrite(HDR.FILE.FID, 0, 'ushort');
                        count = fwrite(HDR.FILE.FID,zeros(2,1),'uchar');
                        
                        count = fwrite(HDR.FILE.FID,zeros(7,1),'float');
                        count = fwrite(HDR.FILE.FID,HDR.Off(k),'short');
                        count = fwrite(HDR.FILE.FID,zeros(2,1),'uchar');
                        count = fwrite(HDR.FILE.FID,[zeros(2,1),e.sensitivity(k)],'float');
                        count = fwrite(HDR.FILE.FID,zeros(3,1),'char');
                        count = fwrite(HDR.FILE.FID,zeros(4,1),'uchar');
                        count = fwrite(HDR.FILE.FID,zeros(1,1),'char');
                        count = fwrite(HDR.FILE.FID,HDR.Cal(k),'short');
                end;	
                
                HDR.HeadLen = ftell(HDR.FILE.FID);
                if HDR.HeadLen ~= (900+75*HDR.NS),
                        fprintf(HDR.FILE.stderr,'Error SOPEN CNT-Write: Headersize does not fit\n');
                end;
        end;
        
        
elseif strcmp(HDR.TYPE,'FEF'),		% FEF/Vital format included
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
        status = fseek(HDR.FILE.FID,32,'bof'); 	% skip preamble
        
        if exist('fefopen','file') & ~status,
                HDR = fefopen(HDR);
        end;
        
        fprintf(HDR.FILE.stderr,'Warning SOPEN: Implementing Vital/FEF format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
        HDR.FILE.FID = -1;
        return;        

        
elseif strcmp(HDR.TYPE,'SCP'),	%
        HDR = scpopen(HDR,PERMISSION);        
	if HDR.ERROR.status,
		fclose(HDR.FILE.FID);
		HDR.FILE.OPEN = 0; 
		return;
	end;	
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
        
        
elseif strcmp(HDR.TYPE,'EBS'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        
        fprintf(HDR.FILE.stderr,'Warning SOPEN: Implementing EBS format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
        
        %%%%% (1) Fixed Header (32 bytes) %%%%%
        HDR.VERSION = fread(HDR.FILE.FID,[1,8],'char');	%
        if setstr(HDR.VERSION(1:3)')~='EBS' 
                fprintf(HDR.FILE.stderr,'Error LOADEBS: %s not an EBS-File',HDR.FileName); 
                if any(HDR.VERSION(4:8)~=hex2dec(['94';'0a';'13';'1a';'0d'])'); 
                        fprintf(HDR.FILE.stderr,'Warning SOPEN EBS: %s may be corrupted',HDR.FileName); 
                end; 
        end;
        HDR.EncodingId = fread(HDR.FILE.FID,1,'int32');	%
        HDR.NS  = fread(HDR.FILE.FID,1,'uint32');	% Number of Channels
        HDR.SPR = fread(HDR.FILE.FID,2,'uint32');	% Number of Samples
        if HDR.SPR(1)==0,
                HDR.SPR=HDR.SPR(2)
        else 
                fprintf(HDR.FILE.stderr,'Error SOPEN: EBS-FILE %s too large',HDR.FileName); 
        end;
        LenData=fread(HDR.FILE.FID,1,'int64');	% Data Length
        
        %%%%% (2) LOAD Variable Header %%%%%
        tag=fread(HDR.FILE.FID,1,'int32');	% Tag field
        while (tag~=0),
                l  =fread(HDR.FILE.FID,1,'int32');	% length of value field
                val=setstr(fread(HDR.FILE.FID,4*l,'char')');	% depends on Tag field
                if     tag==hex2dec('00000002'),	%IGNORE
                elseif tag==hex2dec('00000004') HDR.PATIENT_NAME=val;
                elseif tag==hex2dec('00000006') HDR.PATIENT_ID=val;
                elseif tag==hex2dec('00000008') HDR.PATIENT_BIRTHDAY=val;
                elseif tag==hex2dec('0000000a') HDR.PATIENT_SEX=val;
                elseif tag==hex2dec('0000000c') HDR.SHORT_DESCRIPTION=val;
                elseif tag==hex2dec('0000000e') HDR.DESCRIPTION=val;
                elseif tag==hex2dec('00000010') HDR.SAMPLE_RATE=str2double(val);
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
                tag=fread(HDR.FILE.FID,1,'int32');	% Tag field
        end; 
        fclose(HDR.FILE.FID);
        
        
elseif strcmp(HDR.TYPE,'rhdE'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        
        %fseek(HDR.FILE.FID,4,'bof');		% skip 4 bytes ID
        %HDR.HeadLen = fread(HDR.FILE.FID,1,'int32');	% Length of Header ? 
        %HDR.H2 = fread(HDR.FILE.FID,5,'int32');	
        %HDR.NS = fread(HDR.FILE.FID,1,'int32');		% ? number of channels
        %HDR.H3 = fread(HDR.FILE.FID,5,'int32');	
        tmp = fread(HDR.FILE.FID,10,'int32');	
        HDR.HeadLen = tmp(2);		% Length of Header ? 
        HDR.H2 = tmp;
        HDR.NS = tmp(8);		% ? number of channels
        
        HDR.AS.endpos = HDR.FILE.size;
        HDR.NRec = (HDR.AS.endpos-HDR.HeadLen)/1024;
        
        fprintf(1,'Warning SOPEN HolterExcel2: is under construction.\n');
        
        if (nargout>1),	% just for testing
                H1 = fread(HDR.FILE.FID,[1,inf],'uchar')';
        end;
        fclose(HDR.FILE.FID);
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
        
        
elseif strcmp(HDR.TYPE,'alpha') & any(PERMISSION=='r'),
        HDR.FILE.FID = -1;      % make sure SLOAD does not call SREAD;
        
        % The header files are text files (not binary).
        try
                PERMISSION = 'rt';	% MatLAB default is binary, force Mode='rt';
                fid = fopen(fullfile(HDR.FILE.Path,'rawhead'),PERMISSION);	
        catch
                PERMISSION = 'r';	% Octave 2.1.50 default is text, but does not support Mode='rt', 
                fid = fopen(fullfile(HDR.FILE.Path,'rawhead'),PERMISSION);	
        end;
        
        fid = fopen(fullfile(HDR.FILE.Path,'rawhead'),PERMISSION);	
        if fid < 0,
                fprintf(HDR.FILE.stderr,'Error SOPEN (alpha): couldnot open RAWHEAD\n');
        else
                H = []; k = 0;
                HDR.TYPE = 'unknown';
                while ~feof(fid),
                        [s] = fgetl(fid);
                        if ~isnumeric(s),
                                [tag,s] = strtok(s,'=');
                                [tmp,s] = strtok(s,'=');
                                [val,status] = str2double(tmp);
                                if status,
                                        val = tmp;
                                end;
                                tag = deblank(tag);
                                if strcmp(tag,'Version'),
                                        HDR.VERSION = val;
                                        HDR.TYPE = 'alpha';
                                elseif strcmp(tag,'BitsPerValue'),
                                        HDR.Bits = val;
                                elseif strcmp(tag,'ChanCount'),
                                        HDR.NS = val;
                                elseif strcmp(tag,'SampleCount'),
                                        HDR.SPR = val;
                                elseif strcmp(tag,'SampleFreq'),
                                        HDR.SampleRate = val;
                                elseif strcmp(tag,'NotchFreq'),
                                        HDR.Filter.Notch = val;
                                else
                                        %fprintf(HDR.FILE.stderr,'Warning SOPEN Type=alpha: unknown Tag %s.\n',tag);
                                end;
                        end;
                end;
                fclose(fid);
        end;
        
        HDR.PhysDim = '  ';
        fid = fopen(fullfile(HDR.FILE.Path,'cal_res'),PERMISSION);
        if fid < 0,
                fprintf(HDR.FILE.stderr,'Warning SOPEN alpha-trace: could not open CAL_RES. Data is uncalibrated.\n');
                HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
        else
                [s] = fgetl(fid);       % read version
                [s] = fgetl(fid);       % read calibration time 
                
                HDR.Label = char(zeros(HDR.NS,1));
                HDR.Cal   = repmat(NaN,HDR.NS,1);
                HDR.Off   = zeros(HDR.NS,1);
                OK   = zeros(HDR.NS,1);
                for k = 1:HDR.NS,
                        [s] = fgetl(fid);
                        [tmp,s] = strtok(s,',');
                        [lab,ok] = strtok(tmp,' =,');
                        [ok] = strtok(ok,' =,');
                        
                        [cal,s] = strtok(s,' ,');
                        cal = str2double(cal);
                        [off,s] = strtok(s,' ,');
                        off = str2double(off);
                        
                        HDR.Off(k)=off;
                        HDR.Cal(k)=cal;
                        OK(k)=strcmpi(ok,'yes');
                        HDR.Label(k,1:length(lab))=lab;
                end;
                fclose(fid);
                
                HDR.FLAG.UCAL = ~all(OK);
                if ~all(OK),
                        fprintf(HDR.FILE.stderr,'Warning SOPEN (alpha): calibration not valid for some channels\n');
                end;
                HDR.Cal(find(~OK)) = NaN;
                HDR.Calib = sparse([-HDR.Off';eye(HDR.NS*[1,1])])*sparse(1:HDR.NS,1:HDR.NS,HDR.Cal);
                HDR.PhysDim = 'µV';
        end;        
        
        fid = fopen(fullfile(HDR.FILE.Path,'r_info'),PERMISSION);
        if fid < 0,
                fprintf(HDR.FILE.stderr,'Warning SOPEN alpha-trace: couldnot open R_INFO\n');
        else
                H = [];
                HDR.TYPE = 'unknown';
                while ~feof(fid),
                        [s] = fgetl(fid);
                        if ~isnumeric(s),
                                [tag,s] = strtok(s,'=');
                                [tmp,s] = strtok(s,'=');
                                [val,status] = str2double(tmp);
                                if status,
                                        val = tmp;
                                end;
                                tag = deblank(tag);
                                if strcmp(tag,'Version'),
                                        HDR.VERSION = val;
                                        HDR.TYPE = 'alpha';
                                elseif strcmp(tag,'RecId'),
                                        HDR.ID.Recording = val;
                                elseif strcmp(tag,'Laboratory'),
                                        HDR.ID.Lab = val;
                                elseif strcmp(tag,'RecDate'),
                                        pos = [1,find(val=='.'),length(val)];
                                        tmp = [str2double(val(pos(3)+1:pos(4))),str2double(val(pos(2)+1:pos(3)-1)),str2double(val(pos(1):pos(2)-1))];
                                        HDR.T0(1:3) = tmp;
                                elseif strcmp(tag,'RecTime'),
                                        pos = [1,find(val=='.'),length(val)];
                                        tmp = [str2double(val(pos(1):pos(2)-1)),str2double(val(pos(2)+1:pos(3)-1)),str2double(val(pos(3)+1:pos(4)))];
                                        HDR.T0(4:6) = tmp; 
                                else
                                        %fprintf(HDR.FILE.stderr,'Warning SOPEN Type=alpha: unknown Tag %s.\n',tag);
                                end;
                        end;
                end;
        end;        
        
        fid = fopen(fullfile(HDR.FILE.Path,'digin'),PERMISSION);
        if fid < 0,
                fprintf(HDR.FILE.stderr,'Warning SOPEN alpha-trace: couldnot open DIGIN - no event information included\n');
        else
                [s] = fgetl(fid);       % read version
                
                k = 0; POS = []; DUR = []; TYP = []; IO = [];
                while ~feof(fid),
                        [s] = fgetl(fid);
                        if ~isnumeric(s),
                                [timestamp,s] = strtok(s,'='); 
                                [type,io] = strtok(s,'=,');
                                timestamp = str2double(timestamp);
                                if ~isnan(timestamp),
                                        k = k + 1;
                                        POS(k) = timestamp;     
                                        TYP(k) = hex2dec(type);   
                                        DUR(k) = 0;
                                        if (k>1) & (TYP(k)==0),
                                                DUR(k-1) = POS(k)-POS(k-1);
                                        end;
                                else
                                        fprintf(HDR.FILE.stderr,'Warning SOPEN: alpha: invalid Event type\n');
                                end;	                        
                                if length(io)>1,
                                        IO(k) = io(2);
                                end;
                        end;
                end;
                fclose(fid);
                HDR.EVENT.N   = k;
                HDR.EVENT.POS = POS(:);
                HDR.EVENT.DUR = DUR(:);
                HDR.EVENT.TYP = TYP(:);
                HDR.EVENT.IO  = IO(:);
                HDR.EVENT.CHN = zeros(HDR.EVENT.N,1);
        end;
        if all(abs(HDR.VERSION-[407.1,409.5]) > 1e-6);
                fprintf(HDR.FILE.stderr,'Warning SLOAD: Format ALPHA Version %6.2f not tested yet.\n',HDR.VERSION);
        end;
        
        HDR.FILE.FID = fopen(fullfile(HDR.FILE.Path,'rawdata'),'rb');
        if HDR.FILE.FID > 0,
                HDR.VERSION2  = fread(HDR.FILE.FID,1,'int16');
                HDR.NS   = fread(HDR.FILE.FID,1,'int16');
                HDR.bits = fread(HDR.FILE.FID,1,'int16');
                HDR.AS.bpb = HDR.NS*HDR.bits/8;
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS  = 0;
                HDR.HeadLen = ftell(HDR.FILE.FID);
                fseek(HDR.FILE.FID,0,'eof');
                HDR.AS.endpos = (ftell(HDR.FILE.FID)-HDR.HeadLen)/HDR.AS.bpb;
                HDR.SPR = HDR.AS.endpos;
                fseek(HDR.FILE.FID,HDR.HeadLen,'bof');
        end;
                
elseif strcmp(HDR.TYPE,'DEMG'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');      % ### native should be fixed
        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
                % read header
                fseek(HDR.FILE.FID,4,'bof');    % skip first 4 bytes, should contain 'DEMG'
                HDR.VERSION = fread(HDR.FILE.FID,1,'uint16');	
                HDR.NS  = fread(HDR.FILE.FID,1,'uint16'); 
                HDR.SampleRate = fread(HDR.FILE.FID,1,'uint32');
                HDR.SPR = fread(HDR.FILE.FID,1,'uint32');
                HDR.NRec = 1; 
                
                HDR.bits = fread(HDR.FILE.FID,1,'uint8');
                HDR.PhysMin = fread(HDR.FILE.FID,1,'int8');
                HDR.PhysMax = fread(HDR.FILE.FID,1,'int8');
                if HDR.VERSION==1,
                        HDR.GDFTYP = 'float32';
                        HDR.Cal = 1; 
                        HDR.Off = 0; 
                        HDR.AS.bpb = 4*HDR.NS;
                elseif HDR.VERSION==2,
                        HDR.GDFTYP = 'uint16';
                        HDR.Cal = (HDR.PhysMax-HDR.PhysMin)/(2^HDR.bits-1);
                        HDR.Off = HDR.PhysMin;
                        HDR.AS.bpb = 2*HDR.NS;
                else    
                        fprintf(HDR.FILE.stderr,'Error SOPEN DEMG: invalid version number.\n');
                        fclose(HDR.FILE.FID);
                        HDR.FILE.FID=-1;
                        return;
                end;
                HDR.Calib = sparse([ones(1,HDR.NS),2:HDR.NS+1],[1:HDR.NS,1:HDR.NS],ones(HDR.NS,1)*[HDR.Off,HDR.Cal],HDR.NS+1,HDR.NS);
                HDR.HeadLen = ftell(HDR.FILE.FID);
                HDR.FILE.POS = 0;
                HDR.FILE.OPEN = 1; 
                HDR.AS.endpos = HDR.SPR;
                %HDR.Filter.LowPass = 450;       % default values
                %HDR.Filter.HighPass = 20;       % default values
                
        else
                fprintf(HDR.FILE.stderr,'Warning SOPEN DEMG: writing not implemented, yet.\n');
        end;
        
        
elseif strcmp(HDR.TYPE,'ACQ'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        
        %--------    Fixed Header        
        ItemHeaderLen = fread(HDR.FILE.FID,1,'uint16');
        HDR.VERSION = fread(HDR.FILE.FID,1,'uint32');
        HDR.ACQ.ExtItemHeaderLen = fread(HDR.FILE.FID,1,'uint32');

        HDR.NS = fread(HDR.FILE.FID,1,'int16');
        HDR.ACQ.HorizAxisType = fread(HDR.FILE.FID,1,'int16');
        HDR.ACQ.CurChannel = fread(HDR.FILE.FID,1,'int16');
        HDR.ACQ.SampleTime = fread(HDR.FILE.FID,1,'float64')/1000;
        HDR.SampleRate = 1/HDR.ACQ.SampleTime;
        HDR.TimeOffset = fread(HDR.FILE.FID,1,'float64')/1000;
        HDR.TimeScale  = fread(HDR.FILE.FID,1,'float64');
        HDR.ACQ.TimeCursor1  = fread(HDR.FILE.FID,1,'float64');
        HDR.ACQ.TimeCursor2  = fread(HDR.FILE.FID,1,'float64');
        HDR.ACQ.rcWindow  = fread(HDR.FILE.FID,1,'float64');
        HDR.ACQ.MeasurementType = fread(HDR.FILE.FID,6,'int16');
        HDR.ACQ.HiLite    = fread(HDR.FILE.FID,2,'uint8');
        HDR.FirstTimeOffset = fread(HDR.FILE.FID,1,'float64');
        
        fseek(HDR.FILE.FID,HDR.ACQ.ExtItemHeaderLen,'bof');

        % --------   Variable Header        
        
        % --------   Per Channel data section 
        HDR.Labels = char(zeros(HDR.NS,40));
        HDR.Off = zeros(HDR.NS,1);
        HDR.Cal = ones(HDR.NS,1);
        HDR.ChanHeaderLen = zeros(HDR.NS,1);
        HDR.PhysDim = char(zeros(HDR.NS,20));
        offset = ftell(HDR.FILE.FID); 
        for k = 1:HDR.NS;
                fseek(HDR.FILE.FID,offset+sum(HDR.ChanHeaderLen),'bof');
                HDR.ChanHeaderLen(k) = fread(HDR.FILE.FID,1,'uint32');
                HDR.ChanSel(k) = fread(HDR.FILE.FID,1,'int16');
                HDR.Label(k,1:40) = fread(HDR.FILE.FID,[1,40],'char');
                rgbColor = fread(HDR.FILE.FID,4,'int8');
                DispChan = fread(HDR.FILE.FID,2,'int8');
                HDR.Off(k) = fread(HDR.FILE.FID,1,'float64');
                HDR.Cal(k) = fread(HDR.FILE.FID,1,'float64');
                HDR.PhysDim(k,1:20) = fread(HDR.FILE.FID,[1,20],'char');
                HDR.SPR(k) = fread(HDR.FILE.FID,1,'int32');
                HDR.AmpGain(k) = fread(HDR.FILE.FID,1,'float64');
                HDR.AmpOff(k) = fread(HDR.FILE.FID,1,'float64');
                HDR.ACQ.ChanOrder = fread(HDR.FILE.FID,1,'int16');
                HDR.ACQ.DispSize = fread(HDR.FILE.FID,1,'int16');
                
                if HDR.VERSION >= 34,
                        fseek(HDR.FILE.FID,10,'cof');
                end;
                if HDR.VERSION >= 38,
                        HDR.Description(k,1:128) = fread(HDR.FILE.FID,[1,128],'char');
                        HDR.VarSampleDiv(k) = fread(HDR.FILE.FID,1,'uint16');
                else
                        HDR.VarSampleDiv(k) = 1;
                end;
        end;
        HDR.Label = char(HDR.Label);
        HDR.PhysDim = char(HDR.PhysDim);
        HDR.Calib = [HDR.Off(:).';diag(HDR.Cal)];
        HDR.MAXSPR = HDR.VarSampleDiv(1);
        HDR.NRec = 1; 
        for k = 2:length(HDR.VarSampleDiv);
                HDR.MAXSPR = lcm(HDR.MAXSPR,HDR.VarSampleDiv(k));
        end;
        HDR.ACQ.SampleRate = 1./(HDR.VarSampleDiv*HDR.ACQ.SampleTime);
        HDR.SampleRate = 1/HDR.ACQ.SampleTime;
        HDR.Dur = HDR.MAXSPR*HDR.ACQ.SampleTime;
        
        %--------   foreign data section
        ForeignDataLength = fread(HDR.FILE.FID,1,'int16');
        HDR.ACQ.ForeignDataID = fread(HDR.FILE.FID,1,'uint16');
        HDR.ACQ.ForeignData = fread(HDR.FILE.FID,[1,ForeignDataLength-4],'char');
        %fseek(HDR.FILE.FID,ForeignDataLength-2,'cof');
        
        %--------   per channel data type section
        offset3 = 0;
        HDR.AS.bpb = 0;	
        HDR.AS.spb = 0;	
        for k = 1:HDR.NS,
                sz = fread(HDR.FILE.FID,1,'uint16');
                HDR.AS.bpb = HDR.AS.bpb + HDR.MAXSPR/HDR.VarSampleDiv(k)*sz; 
                HDR.AS.spb = HDR.AS.spb + HDR.MAXSPR/HDR.VarSampleDiv(k); 
                offset3 = offset3 + HDR.SPR(k) * sz;
                
                typ = fread(HDR.FILE.FID,1,'uint16');
                HDR.GDFTYP(k) = 31-typ*14;   % 1 = int16; 2 = double
        end;
        
        HDR.HeadLen = HDR.ACQ.ExtItemHeaderLen + sum(HDR.ChanHeaderLen) + ForeignDataLength + 4*HDR.NS; 
        HDR.FILE.POS = 0; 
        HDR.FILE.OPEN = 1; 
        HDR.AS.endpos = HDR.HeadLen + offset3; 
        fseek(HDR.FILE.FID,HDR.AS.endpos,'bof');	

        %--------  Markers Header section
        len = fread(HDR.FILE.FID,1,'uint32');
        HDR.EVENT.N   = fread(HDR.FILE.FID,1,'uint32');
        HDR.EVENT.POS = repmat(nan,HDR.EVENT.N  ,1);
        HDR.EVENT.TYP = repmat(nan,HDR.EVENT.N  ,1);

        for k = 1:HDR.EVENT.N, 
                %HDR.Event(k).Sample = fread(HDR.FILE.FID,1,'int32');
                HDR.EVENT.POS(k) = fread(HDR.FILE.FID,1,'int32');
                tmp = fread(HDR.FILE.FID,4,'uint16');
                HDR.Event(k).selected = tmp(1); 
                HDR.Event(k).TextLocked = tmp(2); 
                HDR.Event(k).PositionLocked = tmp(3); 
                textlen = tmp(4);
                HDR.Event(k).Text = fread(HDR.FILE.FID,textlen,'char');
        end;
        fseek(HDR.FILE.FID,HDR.HeadLen,'bof');	
        
        
elseif strncmp(HDR.TYPE,'AKO',3),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        HDR.Header = fread(HDR.FILE.FID,[1,46],'char');
        warning('support of AKO format not completed');
        HDR.Patient.ID = char(HDR.Header(17:24));
        HDR.SampleRate = 128; % ???
        HDR.NS = 1;
        HDR.NRec = 1; 
        HDR.Calib = [-127;1];
        [HDR.data,HDR.SPR] = fread(HDR.FILE.FID,inf,'uint8');
        fclose(HDR.FILE.FID);
        HDR.FILE.POS = 0;
        HDR.TYPE = 'native';
        
        
elseif strcmp(HDR.TYPE,'ALICE4'),
        fprintf(HDR.FILE.stderr,'Support of ALICE4 format not completeted. \n\tCalibration, filter setttings and SamplingRate are missing\n');
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        [s,c]  = fread(HDR.FILE.FID,[1,408],'char');
        HDR.NS = s(55:56)*[1;256];
        HDR.SampleRate   = 100; 
        HDR.Patient.ID   = char(s(143:184));
        HDR.Patient.Sex  = char(s(185));
        HDR.Patient.Date = char(s(187:194));
        [H2,c] = fread(HDR.FILE.FID,[118,HDR.NS],'char');
        HDR.Label = char(H2(1:12,:)');
        HDR.FILE.POS = 0;
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
        HDR.FLAG.UCAL = 1; 
        [tmp, count] = fread(HDR.FILE.FID,[HDR.NS,inf],'uint16');
        HDR.SPR  = count/HDR.NS;
        HDR.data = tmp';
        fclose(HDR.FILE.FID);
        HDR.TYPE  = 'native';
        
        
elseif strcmp(HDR.TYPE,'ATES'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        HDR.Header = fread(HDR.FILE.FID,[1,128],'char');
        tmp = fread(HDR.FILE.FID,1,'int16');
        HDR.FLAG.Monopolar = logical(tmp);
        HDR.SampleRate = fread(HDR.FILE.FID,1,'int16');
        HDR.Cal = fread(HDR.FILE.FID,1,'float32');
        type = fread(HDR.FILE.FID,1,'float32');
        if type==2,
                HDR.GDFTYP = 'int16';
        else
                error('ATES: unknown type');
        end;
        HDR.ATES.Mask = fread(HDR.FILE.FID,2,'uint32');
        HDR.DigMax = fread(HDR.FILE.FID,1,'uint16');
        HDR.Filter.Notch = fread(HDR.FILE.FID,1,'uint16');
        HDR.SPR = fread(HDR.FILE.FID,1,'uint32');
        HDR.ATES.MontageName = fread(HDR.FILE.FID,8,'uchar');
        HDR.ATES.MontageComment = fread(HDR.FILE.FID,31,'uchar');
        HDR.NS = fread(HDR.FILE.FID,1,'int16');
        fclose(HDR.FILE.FID);

        
elseif strcmp(HDR.TYPE,'RigSys'),       % thanks to  J. Chen
        HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-le');
        [thdr,count] = fread(HDR.FILE.FID,[1,1024],'char');
        thdr = char(thdr);
        HDR.RigSys.H1 = thdr;        
        empty_char = NaN; 
        STOPFLAG = 1; 
        while (STOPFLAG & ~isempty(thdr));
                [tline, thdr] = strtok(thdr,[13,10,0]);
                [tag, value]  = strtok(tline,'=');
                value = strtok(value,'=');
                if strcmp(tag,'FORMAT ISSUE'), 
                        HDR.VERSION = value; 
                elseif strcmp(tag,'EMPTY HEADER CHARACTER'), 
                        [t,v]=strtok(value);
                        if strcmp(t,'ASCII')
                                empty_char = str2double(v);
                                STOPFLAG = 0; 
                        else
                                fprintf(HDR.FILE.stderr,'Warning SOPEN (RigSys): Couldnot identify empty character');;
                        end;
                end;
        end                        
        if ~isfield(HDR,'VERSION')
                fprintf(HDR.FILE.stderr,'Error SOPEN (RigSys): could not open file %s. Specification not known.\n',HDR.FileName);
                HDR.TYPE = 'unknown';
                fclose(HDR.FILE.FID);
                return;
        end;
        [thdr,H1] = strtok(thdr,empty_char);
        while ~isempty(thdr);
                [tline, thdr] = strtok(thdr,[13,10,0]);
                [tag, value]  = strtok(tline,'=');
                value = strtok(value,'=');
                if 0, 
                elseif strcmp(tag,'HEADER SIZE'), 
                        HDR.RigSys.H1size = str2double(value);
                        if    count == HDR.RigSys.H1size,
                        elseif count < HDR.RigSys.H1size,
                                tmp = fread(HDR.FILE.FID,[1,HDR.RigSys.H1size-count],'char');
                                thdr = [thdr,char(tmp)];
                        elseif count > HDR.RigSys.H1size,
                                status = fseek(HDR.FILE.FID,HDR.RigSys.H1size);
                        end;        
                elseif strcmp(tag,'CHANNEL HEADER SIZE'), 
                        HDR.RigSys.H2size = str2double(value);
                elseif strcmp(tag,'FRAME HEADER SIZE'), 
                        HDR.RigSys.H3size = str2double(value);
                elseif strcmp(tag,'NCHANNELS'), 
                        HDR.NS = str2double(value);
                elseif strcmp(tag,'SAMPLE INTERVAL'), 
                        HDR.SampleRate = 1/str2double(value);
                elseif strcmp(tag,'HISTORY LENGTH'), 
                        HDR.AS.endpos = str2double(value);
                elseif strcmp(tag,'REFERENCE TIME'), 
                        HDR.RigSys.TO=value;
                        HDR.T0(1:6) = round(datevec(value)*1e4)*1e-4;
                end
        end;                                
        HDR.HeadLen = HDR.RigSys.H1size+HDR.RigSys.H1size*HDR.NS;
        
        [H1,c] = fread(HDR.FILE.FID,[HDR.RigSys.H2size,HDR.NS],'char');
        for chan=1:HDR.NS,
                [thdr] = strtok(char(H1(:,chan)'),empty_char);
                while ~isempty(thdr);
                        [tline, thdr] = strtok(thdr,[13,10,0]);
                        [tag, value]  = strtok(tline,'=');
                        value = strtok(value,'=');
                        if strcmp(tag,'FULL SCALE'), 
                                HDR.Gain(chan,1) = str2double(value); 
                        elseif strcmp(tag,'UNITS'), 
                                HDR.PhysDim{chan} = value; 
                        elseif strcmp(tag,'OFFSET'), 
                                HDR.Off(chan) = str2double(value); 
                        elseif 0, strcmp(tag,'CHANNEL DESCRIPTION'), 
                                HDR.Label{chan} = value; 
                        elseif strcmp(tag,'CHANNEL NAME'), 
                                HDR.Label{chan} = value; 
                        elseif strcmp(tag,'SAMPLES PER BLOCK'), 
                                HDR.AS.SPR(chan) = str2double(value); 
                        elseif strcmp(tag,'BYTES PER SAMPLE'), 
                                HDR.Bits(chan) = str2double(value)*8; 
                        end;          
                end;
        end;
        fhsz = HDR.RigSys.H3size*8/HDR.Bits(1);
        s = fread(HDR.FILE.FID,[1024*HDR.NS+fhsz,inf],'int16');
        fclose(HDR.FILE.FID);
        HDR.RigSys.FrameHeaders=s(1:12,:);

        for k=1:HDR.NS,
                if k==1, HDR.MAXSPR = HDR.AS.SPR(1);
                else HDR.MAXSPR = lcm(HDR.MAXSPR, HDR.AS.SPR(1));
                end;
        end;
        HDR.AS.bi = [0;cumsum(HDR.AS.SPR(:))];
        HDR.SPR  = HDR.MAXSPR; 
        HDR.NRec = size(s,2); 
        HDR.FLAG.TRIGGERED = 0; 
        HDR.data = zeros(HDR.MAXSPR*HDR.NRec,HDR.NS);
        for k = 1:HDR.NS,
                tmp = s(fhsz+[HDR.AS.bi(k)+1:HDR.AS.bi(k+1)],:);
                HDR.data(:,k) = rs(tmp(:),1,HDR.MAXSPR/HDR.AS.SPR(k));
        end;
        HDR.data  = HDR.data(1:HDR.AS.endpos,:);
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Gain(:)./16384);

        HDR.Label    = strvcat(HDR.Label);
        HDR.PhysDim  = strvcat(HDR.PhysDim);
        HDR.FILE.POS = 0; 
        HDR.TYPE     = 'native';

        
elseif strcmp(HDR.TYPE,'SND'),
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
                HDR.GDFTYP =  'uchar';
                HDR.bits   =  8;
        elseif HDR.FILE.TYPE==2, 
                HDR.GDFTYP =  'int8';
                HDR.bits   =  8;
        elseif HDR.FILE.TYPE==3, 
                HDR.GDFTYP =  'int16';
                HDR.bits   = 16;
        elseif HDR.FILE.TYPE==4, 
                HDR.GDFTYP = 'bit24';
                HDR.bits   = 24;
        elseif HDR.FILE.TYPE==5, 
                HDR.GDFTYP = 'int32';
                HDR.bits   = 32;
        elseif HDR.FILE.TYPE==6, 
                HDR.GDFTYP = 'float32';
                HDR.bits   = 32;
        elseif HDR.FILE.TYPE==7, 
                HDR.GDFTYP = 'float64';
                HDR.bits   = 64;
                
        elseif HDR.FILE.TYPE==11, 
                HDR.GDFTYP = 'uint8';
                HDR.bits   =  8;
        elseif HDR.FILE.TYPE==12, 
                HDR.GDFTYP = 'uint16';
                HDR.bits   = 16;
        elseif HDR.FILE.TYPE==13, 
                HDR.GDFTYP = 'ubit24';
                HDR.bits   = 24;
        elseif HDR.FILE.TYPE==14, 
                HDR.GDFTYP = 'uint32';
                HDR.bits   = 32;
                
        else
                fprintf(HDR.FILE.stderr,'Error SOPEN SND-format: datatype %i not supported\n',HDR.FILE.TYPE);
                return;
        end;
        HDR.AS.bpb = HDR.NS*HDR.bits/8;
        
        % Calibration 
        if any(HDR.FILE.TYPE==[2:5]), 
                HDR.Cal = 2^(1-HDR.bits); 
        else
                HDR.Cal = 1; 	
        end;
        HDR.Off = 0;
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal);
        
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
                
                
                %%%%% WRITE 
        elseif HDR.FILE.OPEN > 1; 
                datlen = HDR.SPR * HDR.AS.bpb;
                fwrite(HDR.FILE.FID,[hex2dec('2e736e64'),HDR.HeadLen,datlen,HDR.FILE.TYPE,HDR.SampleRate,HDR.NS],'uint32');
                fwrite(HDR.FILE.FID,HDR.INFO,'char');
                
        end;
        HDR.FILE.POS = 0;
        HDR.NRec = 1;
        
        
elseif strncmp(HDR.TYPE,'EEG-1100',8),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
                [H1,count] = fread(HDR.FILE.FID,[1,6160],'char');
                HDR.Patient.Name = char(H1(79+(1:32)));
                if count<6160, 
                        fclose(HDR.FILE.FID);
                        return;
                end;
                HDR.T0(1:6) = str2double({H1(65:68),H1(69:70),H1(71:72),H1(6148:6149),H1(6150:6151),H1(6152:6153)});
                if strcmp(HDR.FILE.Ext,'LOG')
                        [s,c] = fread(HDR.FILE.FID,[1,inf],'char');
                        s = char([H1(1025:end),s]);
                        K = 0; 
                        [t1,s] = strtok(s,0);
                        while ~isempty(s),
                                K = K + 1; 
                                [HDR.EVENT.x{K},s] = strtok(s,0);
                        end
                end;
                fclose(HDR.FILE.FID);
        end;
        
        
elseif strcmp(HDR.TYPE,'MFER'),
	HDR = mwfopen(HDR,PERMISSION);
	if (HDR.FRAME.N ~= 1),
		fprintf(HDR.FILE.stderr,'Error SOPEN (MFER): files with more than one frame not implemented, yet.\n');
		fclose(HDR.FILE.FID);
		HDR.FILE.FID  =-1;
		HDR.FILE.OPEN = 0;
	end

        
elseif strcmp(HDR.TYPE,'MPEG'),
        % http://www.dv.co.yu/mpgscript/mpeghdr.htm
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
                % read header
                try,
                        tmp = fread(HDR.FILE.FID,1,'ubit11');
                catch
                        fprintf(HDR.FILE.stderr,'Error 1003 SOPEN: datatype UBIT11 not implented. Header cannot be read.\n');
                        return;
                end;
                HDR.MPEG.syncword = tmp;
                HDR.MPEG.ID = fread(HDR.FILE.FID,1,'ubit2');
                HDR.MPEG.layer = fread(HDR.FILE.FID,1,'ubit2');
                HDR.MPEG.protection_bit = fread(HDR.FILE.FID,1,'ubit1');
                HDR.MPEG.bitrate_index = fread(HDR.FILE.FID,1,'ubit4');
                HDR.MPEG.sampling_frequency_index = fread(HDR.FILE.FID,1,'ubit2');
                HDR.MPEG.padding_bit = fread(HDR.FILE.FID,1,'ubit1');
                HDR.MPEG.privat_bit = fread(HDR.FILE.FID,1,'ubit1');
                HDR.MPEG.mode = fread(HDR.FILE.FID,1,'ubit2');
                HDR.MPEG.mode_extension = fread(HDR.FILE.FID,1,'ubit2');
                HDR.MPEG.copyright = fread(HDR.FILE.FID,1,'ubit1');
                HDR.MPEG.original_home = fread(HDR.FILE.FID,1,'ubit1');
                HDR.MPEG.emphasis = fread(HDR.FILE.FID,1,'ubit2');
                
                switch HDR.MPEG.ID,	%Layer 
                        case 0,
                                HDR.VERSION = 2.5;
                        case 1,
                                HDR.VERSION = -1;% reserved
                        case 2,
                                HDR.VERSION = 2;
                        case 3,
                                HDR.VERSION = 1;
                end;
                
                tmp = [32,32,32,32,8; 64,48,40,48,16; 96,56,48,56,24; 128,64,56,64,32; 160,80,64,80,40; 192,96,80,96,48; 224,112,96,112,56; 256,128,112,128,64; 288,160,128,144,80; 320,192 160,160,96; 352,224,192,176,112; 384,256,224, 192,128; 416,320,256,224,144;  448,384,320,256,160];
                tmp = [tmp,tmp(:,5)];
                if HDR.MPEG.bitrate_index==0,
                        HDR.bitrate = NaN;
                elseif HDR.MPEG.bitrate_index==15,
                        fclose(HDR.FILE.FID);
                        fprintf(HDR.FILE.stderr,'SOPEN: corrupted MPEG file %s ',HDR.FileName);
                        return;
                else
                        HDR.bitrate = tmp(HDR.MPEG.bitrate_index,floor(HDR.VERSION)*3+HDR.MPEG.layer-3);
                end;
                
                switch HDR.MPEG.sampling_frequency_index,
                        case 0,
                                HDR.SampleRate = 44.100;
                        case 1,
                                HDR.SampleRate = 48.000;
                        case 2,
                                HDR.SampleRate = 32.000;
                        otherwise,
                                HDR.SampleRate = NaN;
                end;
                HDR.SampleRate_units = 'kHz';
                HDR.SampleRate = HDR.SampleRate*(2^(1-ceil(HDR.VERSION)));
                
                switch 4-HDR.MPEG.layer,	%Layer 
                        case 1,
                                HDR.SPR = 384;
                                slot = 32*HDR.MPEG.padding_bit; % bits, 4 bytes
                                HDR.FrameLengthInBytes = (12*HDR.bitrate/HDR.SampleRate+slot)*4; 
                        case {2,3},
                                HDR.SampleRate = 1152;
                                slot = 8*HDR.MPEG.padding_bit; % bits, 1 byte
                                HDR.FrameLengthInBytes = 144*HDR.bitrate/HDR.SampleRate+slot; 
                end;
                
                if ~HDR.MPEG.protection_bit,
                        HDR.MPEG.error_check = fread(HDR.FILE.FID,1,'uint16');
                end;
                
                HDR.MPEG.allocation = fread(HDR.FILE.FID,[1,32],'ubit4');
                HDR.MPEG.NoFB = sum(HDR.MPEG.allocation>0);
                HDR.MPEG.idx = find(HDR.MPEG.allocation>0);
                HDR.MPEG.scalefactor = fread(HDR.FILE.FID,[1,HDR.MPEG.NoFB],'ubit6');
                for k = HDR.MPEG.idx,
                        HDR.MPEG.temp(1:12,k) = fread(HDR.FILE.FID,[12,1],['ubit',int2str(HDR.MPEG.allocation(k))]);
                end;
                fprintf(HDR.FILE.stderr,'Warning SOPEN: MPEG not ready for use (%s)\n',HDR.FileName);
                HDR.FILE.OPEN = 1; 
        end;
        HDR.FILE.OPEN = 0; 
        fclose(HDR.FILE.FID);
        HDR.FILE.FID = -1; 
        return; 
        
        
elseif strcmp(HDR.TYPE,'QTFF'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
                HDR.FILE.OPEN = 1; 
                offset = 0; 
                while ~feof(HDR.FILE.FID),	
                        tagsize = fread(HDR.FILE.FID,1,'uint32');        % which size 
                        if ~isempty(tagsize),
                                offset = offset + tagsize; 
                                tag = setstr(fread(HDR.FILE.FID,[1,4],'char'));
                                if tagsize==0,
                                        tagsize=inf; %tagsize-8;        
                                elseif tagsize==1,
                                        tagsize=fread(HDR.FILE.FID,1,'uint64');        
                                end;
                                
                                if tagsize <= 8,
                                elseif strcmp(tag,'free'),
                                        val = fread(HDR.FILE.FID,[1,tagsize-8],'char');
                                        HDR.MOV.free = val;
                                elseif strcmp(tag,'skip'),
                                        val = fread(HDR.FILE.FID,[1,tagsize-8],'char');
                                        HDR.MOV.skip = val;
                                elseif strcmp(tag,'wide'),
                                        %val = fread(HDR.FILE.FID,[1,tagsize-8],'char');
                                        %HDR.MOV.wide = val;
                                elseif strcmp(tag,'pnot'),
                                        val = fread(HDR.FILE.FID,[1,tagsize-8],'char');
                                        HDR.MOV.pnot = val;
                                elseif strcmp(tag,'moov'),
                                        offset2 = 8;
                                        while offset2 < tagsize, 
                                                tagsize2 = fread(HDR.FILE.FID,1,'uint32');        % which size 
                                                if tagsize2==0,
                                                        tagsize2 = inf;
                                                elseif tagsize2==1,
                                                        tagsize2=fread(HDR.FILE.FID,1,'uint64');        
                                                end;
                                                offset2 = offset2 + tagsize2;                
                                                tag2 = setstr(fread(HDR.FILE.FID,[1,4],'char'));
                                                if tagsize2 <= 8,
                                                elseif strcmp(tag2,'mvhd'),
                                                        HDR.MOOV.Version = fread(HDR.FILE.FID,1,'char');
                                                        HDR.MOOV.Flags = fread(HDR.FILE.FID,3,'char');
                                                        HDR.MOOV.Times = fread(HDR.FILE.FID,5,'uint32');
                                                        HDR.T0 = datevec(HDR.MOOV.Times(1)/(3600*24))+[1904,0,0,0,0,0];
                                                        HDR.MOOV.prefVol = fread(HDR.FILE.FID,1,'uint16');
                                                        HDR.MOOV.reserved = fread(HDR.FILE.FID,10,'char');
                                                        HDR.MOOV.Matrix = fread(HDR.FILE.FID,[3,3],'int32')';
                                                        HDR.MOOV.Matrix(:,1:2) = HDR.MOOV.Matrix(:,1:2)/2^16; 
                                                        HDR.MOOV.Preview = fread(HDR.FILE.FID,5,'uint32');
                                                elseif strcmp(tag2,'trak'),
                                                        HDR.MOOV.trak = fread(HDR.FILE.FID,[1,tagsize2-8],'char');
                                                elseif strcmp(tag2,'cmov'),
                                                        HDR.MOOV.cmov = fread(HDR.FILE.FID,[1,tagsize2-8],'uchar');
                                                elseif strcmp(tag2,'free'),
                                                        HDR.MOOV.free = fread(HDR.FILE.FID,[1,tagsize2-8],'char');
                                                elseif strcmp(tag2,'clip'),
                                                        HDR.MOOV.clip = fread(HDR.FILE.FID,[1,tagsize2-8],'char');
                                                elseif strcmp(tag2,'udta'),
                                                        HDR.MOOV.udta = fread(HDR.FILE.FID,[1,tagsize2-8],'char');
                                                elseif strcmp(tag2,'ctab'),
                                                        HDR.MOOV.ctab = fread(HDR.FILE.FID,[1,tagsize2-8],'char');
                                                else
                                                end;
                                        end;
                                        %HDR.MOV.moov = fread(HDR.FILE.FID,[1,tagsize-8],'char');
                                        
                                elseif strcmp(tag,'mdat'),
                                        HDR.HeadLen = ftell(HDR.FILE.FID);        
                                        offset2 = 8;
                                        while offset2 < tagsize, 
                                                tagsize2 = fread(HDR.FILE.FID,1,'uint32');        % which size 
                                                tag2 = char(fread(HDR.FILE.FID,[1,4],'char'));
                                                if tagsize2==0,
                                                        tagsize2 = inf;
                                                elseif tagsize2==1,
                                                        tagsize2 = fread(HDR.FILE.FID,1,'uint64');        
                                                end;
                                                offset2  = offset2 + tagsize2;
                                                if tagsize2 <= 8,
                                                elseif strcmp(tag2,'mdat'),
                                                        HDR.MDAT.mdat = fread(HDR.FILE.FID,[1,tagsize2-8],'char');
                                                elseif strcmp(tag2,'wide'),
                                                        HDR.MDAT.wide = fread(HDR.FILE.FID,[1,tagsize2],'char');
                                                elseif strcmp(tag2,'clip'),
                                                        HDR.MDAT.clip = fread(HDR.FILE.FID,[1,tagsize2-8],'char');
                                                elseif strcmp(tag2,'udta'),
                                                        HDR.MDAT.udta = fread(HDR.FILE.FID,[1,tagsize2-8],'char');
                                                elseif strcmp(tag2,'ctab'),
                                                        HDR.MDAT.ctab = fread(HDR.FILE.FID,[1,tagsize2-8],'char');
                                                else
                                                end;
                                        end;
                                        %HDR.MOV.mdat = fread(HDR.FILE.FID,[1,tagsize-8],'char');
                                else
                                        val = fread(HDR.FILE.FID,[1,tagsize-8],'char');
                                        fprintf(HDR.FILE.stderr,'Warning SOPEN Type=MOV: unknown Tag %s.\n',tag);
                                end;
                                fseek(HDR.FILE.FID,offset,'bof');
                        end;       
                end;
        end;        
        %fclose(HDR.FILE.FID);
        
        
elseif strcmp(HDR.TYPE,'ASF') ,
        if exist('asfopen','file'),
                HDR = asfopen(HDR,PERMISSION);
        else
                fprintf(1,'SOPEN ASF-File: Microsoft claims that its illegal to implement the ASF format.\n');
                fprintf(1,'     Anyway Microsoft provides the specification at http://www.microsoft.com/windows/windowsmedia/format/asfspec.aspx \n');
                fprintf(1,'     So, you can implement it and use it for your own purpose.\n');
        end; 
        
        
elseif strcmp(HDR.TYPE,'MIDI') | strcmp(HDR.TYPE,'RMID') ,
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);

        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
                
                [tmp,c] = fread(HDR.FILE.FID,[1,4+12*strcmp(HDR.TYPE,'RMID') ],'char');
		tmp = char(tmp(c+(-3:0)));
                if ~strcmpi(tmp,'MThd'),
                        fprintf(HDR.FILE.stderr,'Warning SOPEN (MIDI): file %s might be corrupted 1\n',HDR.FileName);
                end;

                while ~feof(HDR.FILE.FID),	
                        tag     = setstr(tmp);
                        tagsize = fread(HDR.FILE.FID,1,'uint32');        % which size 
                        filepos = ftell(HDR.FILE.FID);
                        
                        if 0, 

			%%%% MIDI file format 	
                        elseif strcmpi(tag,'MThd');
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize/2],'uint16');
                                HDR.MIDI.Format = tmp(1);
                                HDR.NS = tmp(2);
				if tmp(3)<2^15,
		                        HDR.SampleRate = tmp(3);
				else	
					tmp4 = floor(tmp(3)/256);
					if tmp>127, 
						tmp4 = 256-tmp4; 
						HDR.SampleRate = (tmp4*rem(tmp(3),256));
					end
				end;
				CurrentTrack = 0; 

                        elseif strcmpi(tag,'MTrk');
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uint8');
				CurrentTrack = CurrentTrack + 1; 
				HDR.MIDI.Track{CurrentTrack} = tmp; 
				k = 1; 
				while 0,k<c,
					deltatime = 1; 
					while tmp(k)>127,
						deltatime = mod(tmp(k),128) + deltatime*128;
						k = k+1;
					end;
					deltatime = tmp(k) + deltatime*128;
					k = k+1;
					status_byte = tmp(k);
					k = k+1;
					
					if any(floor(status_byte/16)==[8:11]), % Channel Mode Message
						databyte = tmp(k:k+1);
						k = k+2;
				
					elseif any(floor(status_byte/16)==[12:14]), % Channel Voice Message
						databyte = tmp(k);
						k = k+1;

					elseif any(status_byte==hex2dec(['F0';'F7'])) % Sysex events
						len = 1; 
						while tmp(k)>127,
							len = mod(tmp(1),128) + len*128
							k = k+1;
						end;
						len = tmp(k) + len*128;
						data = tmp(k+(1:len));
				
					% System Common Messages 
					elseif status_byte==240, % F0 
					elseif status_byte==241, % F1 
						while tmp(k)<128,
							k = k+1;
						end;
					elseif status_byte==242, % F2 
						k = k + 1; 
					elseif status_byte==243, % F3 
						k = k + 1; 
					elseif status_byte==244, % F4 
					elseif status_byte==245, % F5 
					elseif status_byte==246, % F6 
					elseif status_byte==247, % F7 
					elseif status_byte==(248:254), % F7:FF 
								
					elseif (status_byte==255) % Meta Events
						type = tmp(k);
						k = k+1;
						len = 1; 
						while tmp(k)>127,
							len = mod(tmp(1),128) + len*128
							k = k+1;
						end;
						len = tmp(k) + len*128;
						data = tmp(k+1:min(k+len,length(tmp)));
						if 0, 
						elseif type==0,	HDR.MIDI.SequenceNumber = data;
						elseif type==1,	HDR.MIDI.TextEvent = char(data);
						elseif type==2,	HDR.Copyright = char(data);
						elseif type==3,	HDR.MIDI.SequenceTrackName = char(data);
						elseif type==4,	HDR.MIDI.InstrumentNumber = char(data);
						elseif type==5,	HDR.MIDI.Lyric = char(data);
						elseif type==6,	HDR.EVENT.POS = data;
						elseif type==7,	HDR.MIDI.CuePoint = char(data);
						elseif type==32,MDR.MIDI.ChannelPrefix = data;
						elseif type==47,MDR.MIDI.EndOfTrack = k;
						
						end;
					else
					end; 
				end;
                                
                        elseif ~isempty(tagsize)
                                fprintf(HDR.FILE.stderr,'Warning SOPEN (MIDI): unknown TAG in %s: %s(%i) \n',HDR.FileName,tag,tagsize);
                                [tmp,c] = fread(HDR.FILE.FID,[1,min(100,tagsize)],'uchar');
                                fprintf(HDR.FILE.stderr,'%s\n',char(tmp));
			end,

                        if ~isempty(tagsize)
                                status = fseek(HDR.FILE.FID,filepos+tagsize,'bof');
                                if status, 
                                        fprintf(HDR.FILE.stderr,'Warning SOPEN (MIDI): fseek failed. Probably tagsize larger than end-of-file and/or file corrupted\n');
                                        fseek(HDR.FILE.FID,0,'eof');
                                end; 
                        end;
                        [tmp,c] = fread(HDR.FILE.FID,[1,4],'char');
		end;
                HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);

                HDR.FILE.POS = 0;
                HDR.FILE.OPEN = 1;
                HDR.NRec = 1;
        end; 
        
        
elseif strmatch(HDR.TYPE,['AIF';'IIF';'WAV';'AVI']),
	if strcmp(HDR.TYPE,'AIF')
	        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        else
		HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
	end;
	
        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 

                tmp = setstr(fread(HDR.FILE.FID,[1,4],'char'));
                if ~strcmpi(tmp,'FORM') & ~strcmpi(tmp,'RIFF')
                        fprintf(HDR.FILE.stderr,'Warning SOPEN AIF/WAV-format: file %s might be corrupted 1\n',HDR.FileName);
                end;
                tagsize  = fread(HDR.FILE.FID,1,'uint32');        % which size
                tagsize0 = tagsize + rem(tagsize,2); 
                tmp = setstr(fread(HDR.FILE.FID,[1,4],'char'));
                if ~strncmpi(tmp,'AIF',3) & ~strncmpi(tmp,'WAVE',4) & ~strncmpi(tmp,'AVI ',4),
			% not (AIFF or AIFC or WAVE)
                        fprintf(HDR.FILE.stderr,'Warning SOPEN AIF/WAF-format: file %s might be corrupted 2\n',HDR.FileName);
                end;
                
                [tmp,c] = fread(HDR.FILE.FID,[1,4],'char');
                while ~feof(HDR.FILE.FID),	
                        tag     = setstr(tmp);
                        tagsize = fread(HDR.FILE.FID,1,'uint32');        % which size 
                        tagsize0= tagsize + rem(tagsize,2); 
                        filepos = ftell(HDR.FILE.FID);
                        
                        %%%% AIF - section %%%%%
                        if strcmpi(tag,'COMM')
                                if tagsize<18, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN AIF: incorrect tag size\n');
                                        return;
                                end;
                                HDR.NS   = fread(HDR.FILE.FID,1,'uint16');
                                HDR.SPR  = fread(HDR.FILE.FID,1,'uint32');
                                HDR.AS.endpos = HDR.SPR;
                                HDR.bits = fread(HDR.FILE.FID,1,'uint16');
                                %HDR.GDFTYP = ceil(HDR.bits/8)*2-1; % unsigned integer of approbriate size;
                                if HDR.bits == 8;
					HDR.GDFTYP = 'uint8';
                                elseif HDR.bits == 16;
					HDR.GDFTYP = 'uint16';
                                elseif HDR.bits == 32;
					HDR.GDFTYP = 'uint32';
                                else
					HDR.GDFTYP = ['ubit', int2str(HDR.bits)];
				end;	
                                HDR.Cal  = 2^(1-HDR.bits);
                                HDR.Off  = 0; 
                                HDR.AS.bpb = ceil(HDR.bits/8)*HDR.NS;
                                
                                % HDR.SampleRate; % construct Extended 80bit IEEE 754 format 
                                tmp = fread(HDR.FILE.FID,1,'int16');
                                sgn = sign(tmp);
                                if tmp(1)>= 2^15; tmp(1)=tmp(1)-2^15; end;
                                e = tmp - 2^14 + 1;
                                tmp = fread(HDR.FILE.FID,2,'uint32');
                                HDR.SampleRate = sgn * (tmp(1)*(2^(e-31))+tmp(2)*2^(e-63));
                                HDR.Dur = HDR.SPR/HDR.SampleRate;
                                HDR.FILE.TYPE = 0;
                                
                                if tagsize>18,
                                        [tmp,c] = fread(HDR.FILE.FID,[1,4],'char');
                                        HDR.AIF.CompressionType = setstr(tmp);
                                        [tmp,c] = fread(HDR.FILE.FID,[1,tagsize-18-c],'char');
                                        HDR.AIF.CompressionName = tmp;
                                        
                                        if strcmpi(HDR.AIF.CompressionType,'NONE');
                                        elseif strcmpi(HDR.AIF.CompressionType,'fl32');
                                                HDR.GDFTYP = 'uint16';
                                                HDR.Cal = 1;
                                        elseif strcmpi(HDR.AIF.CompressionType,'fl64');
                                                HDR.GDFTYP = 'float64';
                                                HDR.Cal = 1;
                                        elseif strcmpi(HDR.AIF.CompressionType,'alaw');
                                                HDR.GDFTYP = 'uint8';
                                                HDR.AS.bpb = HDR.NS;
                                                %HDR.FILE.TYPE = 1;
                                                fprintf(HDR.FILE.stderr,'Warning SOPEN AIFC-format: data not scaled because of CompressionType ALAW\n');
                                                HDR.FLAG.UCAL = 1;
                                        elseif strcmpi(HDR.AIF.CompressionType,'ulaw');
                                                HDR.GDFTYP = 'uint8';
                                                HDR.AS.bpb = HDR.NS;
                                                HDR.FILE.TYPE = 1;  
                                                
                                                %%%% other compression types - currently not supported, probably obsolete
                                                %elseif strcmpi(HDR.AIF.CompressionType,'DWVW');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'GSM');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'ACE2');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'ACE8');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'ima4');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'MAC3');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'MAC6');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'Qclp');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'QDMC');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'rt24');
                                                %elseif strcmpi(HDR.AIF.CompressionType,'rt29');
                                        else
                                                fprintf(HDR.FILE.stderr,'Warning SOPEN AIFC-format: CompressionType %s is not supported\n', HDR.AIF.CompressionType);
                                        end;
                                end;	
                                
                        elseif strcmpi(tag,'SSND');
                                HDR.AIF.offset   = fread(HDR.FILE.FID,1,'int32');
                                HDR.AIF.blocksize= fread(HDR.FILE.FID,1,'int32');
                                HDR.AIF.SSND.tagsize = tagsize-8; 
				
                                HDR.HeadLen = filepos+8; 
                                %HDR.AIF.sounddata= fread(HDR.FILE.FID,tagsize-8,'uint8');
                                
                        elseif strcmpi(tag,'FVER');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN WAV: incorrect tag size\n');
                                        return;
                                end;
                                HDR.AIF.TimeStamp   = fread(HDR.FILE.FID,1,'uint32');
                                
                        elseif strcmp(tag,'DATA') & strcmp(HDR.TYPE,'AIF') ;	% AIF uses upper case, there is a potential conflict with WAV using lower case data  
                                HDR.AIF.DATA  = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                
                        elseif strcmpi(tag,'INST');   % not sure if this is ok !
                                %HDR.AIF.INST  = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                %HDR.AIF.INST.notes  = fread(HDR.FILE.FID,[1,6],'char');
                                HDR.AIF.INST.baseNote  = fread(HDR.FILE.FID,1,'char');
                                HDR.AIF.INST.detune    = fread(HDR.FILE.FID,1,'char');
                                HDR.AIF.INST.lowNote   = fread(HDR.FILE.FID,1,'char');
                                HDR.AIF.INST.highNote  = fread(HDR.FILE.FID,1,'char');
                                HDR.AIF.INST.lowvelocity  = fread(HDR.FILE.FID,1,'char');
                                HDR.AIF.INST.highvelocity = fread(HDR.FILE.FID,1,'char');
                                HDR.AIF.INST.gain      = fread(HDR.FILE.FID,1,'int16');
                                
                                HDR.AIF.INST.sustainLoop_PlayMode = fread(HDR.FILE.FID,1,'char');
                                HDR.AIF.INST.sustainLoop = fread(HDR.FILE.FID,2,'uint16');
                                HDR.AIF.INST.releaseLoop_PlayMode = fread(HDR.FILE.FID,1,'char');
                                HDR.AIF.INST.releaseLoop = fread(HDR.FILE.FID,2,'uint16');
                                
                        elseif strcmpi(tag,'MIDI');
                                HDR.AIF.MIDI = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                
                        elseif strcmpi(tag,'AESD');
                                HDR.AIF.AESD = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                
                        elseif strcmpi(tag,'APPL');
                                HDR.AIF.APPL = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                
                        elseif strcmpi(tag,'COMT');
                                HDR.AIF.COMT = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                
                        elseif strcmpi(tag,'ANNO');
                                HDR.AIF.ANNO = setstr(fread(HDR.FILE.FID,[1,tagsize],'uchar'));
                                
                        elseif strcmpi(tag,'(c) ');
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.Copyright = setstr(tmp);
                                
                                %%%% WAV - section %%%%%
                        elseif strcmpi(tag,'fmt ')
                                if tagsize<14, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN WAV: incorrect tag size\n');
                                        return;
                                end;
                                HDR.WAV.Format = fread(HDR.FILE.FID,1,'uint16');
                                HDR.NS = fread(HDR.FILE.FID,1,'uint16');
                                HDR.SampleRate = fread(HDR.FILE.FID,1,'uint32');
                                HDR.WAV.AvgBytesPerSec = fread(HDR.FILE.FID,1,'uint32');
                                HDR.WAV.BlockAlign = fread(HDR.FILE.FID,1,'uint16');
                                if HDR.WAV.Format==1,	% PCM format
                                        HDR.bits = fread(HDR.FILE.FID,1,'uint16');
                                        HDR.Off = 0;
                                        HDR.Cal = 2^(1-8*ceil(HDR.bits/8));
                                        if HDR.bits<=8,
                                                HDR.GDFTYP = 'uchar'; 
                                                HDR.Off =  1;
                                                %HDR.Cal = HDR.Cal*2;
                                        elseif HDR.bits<=16,
                                                HDR.GDFTYP = 'int16'; 
                                        elseif HDR.bits<=24,
                                                HDR.GDFTYP = 'bit24'; 
                                        elseif HDR.bits<=32,
                                                HDR.GDFTYP = 'int32'; 
                                        end;
                                else 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN WAV: format type %i not supported\n',HDR.WAV.Format);	
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
                                        fprintf(HDR.FILE.stderr,'Error SOPEN WAV: format type %i not supported\n',HDR.WAV.Format);	
                                end;
                                
                        elseif strcmpi(tag,'fact');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN WAV: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.FACT = setstr(tmp);
                                
                        elseif strcmpi(tag,'disp');
                                if tagsize<8, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN WAV: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.DISP = setstr(tmp);
                                if ~all(tmp(1:8)==[0,1,0,0,0,0,1,1])
                                        HDR.RIFF.DISPTEXT = setstr(tmp(5:length(tmp)));
                                end;
                                
                        elseif strcmpi(tag,'list');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN WAV: incorrect tag size\n');
                                        return;
                                end;
                                
                                if ~isfield(HDR,'RIFF');
                                        HDR.RIFF.N1 = 1;
                                elseif ~isfield(HDR.RIFF,'N');
                                        HDR.RIFF.N1 = 1;
                                else
                                        HDR.RIFF.N1 = HDR.RIFF.N1+1;
                                end;
                                
                                %HDR.RIFF.list = setstr(tmp);
                                [tag,c1]  = fread(HDR.FILE.FID,[1,4],'char');
				tag = char(tag);
                                [val,c2]  = fread(HDR.FILE.FID,[1,tagsize-4],'char');
				HDR.RIFF = setfield(HDR.RIFF,tag,char(val));
                                if 1,
				elseif strcmp(tag,'INFO'),
                                        HDR.RIFF.INFO=val;
                                elseif strcmp(tag,'movi'),
                                        HDR.RIFF.movi = val;
                                elseif strcmp(tag,'hdrl'),
                                        HDR.RIFF.hdr1 = val;
					
                                elseif 0,strcmp(tag,'mdat'),
                                        %HDR.RIFF.mdat = val;
                                else
                                        fprintf(HDR.FILE.stderr,'Warning SOPEN Type=RIFF: unknown Tag %s.\n',tag);
                                end;
                                % AVI  audio video interleave format 	
                        elseif strcmpi(tag,'movi');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.movi = setstr(tmp);
                                
                        elseif strcmp(tag,'idx1');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.idx1 = setstr(tmp);
                                
                        elseif strcmpi(tag,'junk');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.junk = setstr(tmp);
                                
                        elseif strcmpi(tag,'MARK');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.MARK = setstr(tmp);
                                
                        elseif strcmpi(tag,'AUTH');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.AUTH = setstr(tmp);
                                
                        elseif strcmpi(tag,'NAME');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.NAME = setstr(tmp);
                                
                        elseif strcmpi(tag,'afsp');
                                if tagsize<4, 
                                        fprintf(HDR.FILE.stderr,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.afsp = setstr(tmp);
                                
                        elseif ~isempty(tagsize)
                                fprintf(HDR.FILE.stderr,'Warning SOPEN AIF/WAV: unknown TAG in %s: %s(%i) \n',HDR.FileName,tag,tagsize);
                                [tmp,c] = fread(HDR.FILE.FID,[1,min(100,tagsize)],'uchar');
                                fprintf(HDR.FILE.stderr,'%s\n',char(tmp));
                        end;

                        if ~isempty(tagsize)
                                status = fseek(HDR.FILE.FID,filepos+tagsize0,'bof');
                                if status, 
                                        fprintf(HDR.FILE.stderr,'Warning SOPEN (WAF/AIF/AVI): fseek failed. Probably tagsize larger than end-of-file and/or file corrupted\n');
                                        fseek(HDR.FILE.FID,0,'eof');
                                end; 
                        end;
                        [tmp,c] = fread(HDR.FILE.FID,[1,4],'char');
                end;
                
		if strncmpi(tmp,'AIF',3),
                        if HDR.AIF.SSND.tagsize~=HDR.SPR*HDR.AS.bpb,
                                fprintf(HDR.FILE.stderr,'Warning SOPEN AIF: Number of samples do not fit %i vs %i\n',tmp,HDR.SPR);
                        end;
                end;
		                
                if ~isfield(HDR,'HeadLen') 
                        fprintf(HDR.FILE.stderr,'Warning SOPEN AIF/WAV: missing data section\n');
                else
                        status = fseek(HDR.FILE.FID, HDR.HeadLen, 'bof');
                end;
		
		if isnan(HDR.NS), return; end; 

                % define Calib: implements S = (S+.5)*HDR.Cal - HDR.Off;
                HDR.Calib = [repmat(.5,1,HDR.NS);eye(HDR.NS)] * diag(repmat(HDR.Cal,1,HDR.NS));
                HDR.Calib(1,:) = HDR.Calib(1,:) - HDR.Off;

                HDR.FILE.POS = 0;
                HDR.FILE.OPEN = 1;
                HDR.NRec = 1;
                
                
        elseif ~isempty(findstr(PERMISSION,'w')),	%%%%% WRITE 
                HDR.FILE.OPEN = 3; 
                if strcmp(HDR.TYPE,'AIF') 
                        fwrite(HDR.FILE.FID,'FORM','char');	
                        fwrite(HDR.FILE.FID,0,'uint32');	
                        fwrite(HDR.FILE.FID,'AIFFCOMM','char');	
                        fwrite(HDR.FILE.FID,18,'uint32');	
                        fwrite(HDR.FILE.FID,HDR.NS,'uint16');	
                        fwrite(HDR.FILE.FID,HDR.SPR,'uint32');	
                        fwrite(HDR.FILE.FID,HDR.bits,'uint16');	
                        
                        %HDR.GDFTYP = ceil(HDR.bits/8)*2-1; % unsigned integer of appropriate size;
                        HDR.GDFTYP = ['ubit', int2str(HDR.bits)];
                        HDR.Cal    = 2^(1-HDR.bits);
                        HDR.AS.bpb = ceil(HDR.bits/8)*HDR.NS;
                        
                        [f,e] = log2(HDR.SampleRate);
                        tmp = e + 2^14 - 1;
                        if tmp<0, tmp = tmp + 2^15; end;
                        fwrite(HDR.FILE.FID,tmp,'uint16');	
                        fwrite(HDR.FILE.FID,[bitshift(abs(f),31),bitshift(abs(f),63)],'uint32');	
                        
                        HDR.AS.bpb = HDR.NS * ceil(HDR.bits/8);
                        tagsize = HDR.SPR*HDR.AS.bpb + 8;
                        HDR.Dur = HDR.SPR/HDR.SampleRate;
                        HDR.AS.endpos = HDR.SPR;
                        
                        if 0; isfield(HDR.AIF,'INST');	% does not work yet
                                fwrite(HDR.FILE.FID,'SSND','char');	
                                fwrite(HDR.FILE.FID,20,'uint32');	
                                
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.baseNote,'char');
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.detune,'char');
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.lowNote,'char');
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.highNote,'char');
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.lowvelocity,'char');
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.highvelocity,'char');
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.gain,'int16');
                                
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.sustainLoop_PlayMode,'char');
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.sustainLoop,'uint16');
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.releaseLoop_PlayMode,'char');
                                fwrite(HDR.FILE.FID,HDR.AIF.INST.releaseLoop,'uint16');
                        end;
                        
                        fwrite(HDR.FILE.FID,'SSND','char');	
                        HDR.WAV.posis = [4, ftell(HDR.FILE.FID)];
                        fwrite(HDR.FILE.FID,[tagsize,0,0],'uint32');	
                        
                        HDR.HeadLen = ftell(HDR.FILE.FID);
                        
                elseif  strcmp(HDR.TYPE,'WAV'),
                        fwrite(HDR.FILE.FID,'RIFF','char');	
                        fwrite(HDR.FILE.FID,0,'uint32');	
                        fwrite(HDR.FILE.FID,'WAVEfmt ','char');	
                        fwrite(HDR.FILE.FID,16,'uint32');	
                        fwrite(HDR.FILE.FID,[1,HDR.NS],'uint16');	
                        fwrite(HDR.FILE.FID,[HDR.SampleRate,HDR.bits/8*HDR.NS*HDR.SampleRate],'uint32');	
                        fwrite(HDR.FILE.FID,[HDR.bits/8*HDR.NS,HDR.bits],'uint16');	
                        
                        if isfield(HDR,'Copyright'),
                                fwrite(HDR.FILE.FID,'(c) ','char');	
                                if rem(length(HDR.Copyright),2),
                                        HDR.Copyright(length(HDR.Copyright)+1)=' ';
                                end;	
                                fwrite(HDR.FILE.FID,length(HDR.Copyright),'uint32');	
                                fwrite(HDR.FILE.FID,HDR.Copyright,'char');	
                        end;
                        
                        HDR.Off = 0;
                        HDR.Cal = 2^(1-8*ceil(HDR.bits/8));
                        if HDR.bits<=8,
                                HDR.GDFTYP = 'uchar'; 
                                HDR.Off =  1;
                                %HDR.Cal = HDR.Cal*2;
                        elseif HDR.bits<=16,
                                HDR.GDFTYP = 'int16'; 
                        elseif HDR.bits<=24,
                                HDR.GDFTYP = 'bit24'; 
                        elseif HDR.bits<=32,
                                HDR.GDFTYP = 'int32'; 
                        end;
                        
                        HDR.AS.bpb = HDR.NS * ceil(HDR.bits/8);
                        tagsize = HDR.SPR*HDR.AS.bpb;
                        HDR.Dur = HDR.SPR/HDR.SampleRate;
                        HDR.AS.endpos = HDR.SPR;
                        
                        fwrite(HDR.FILE.FID,'data','char');	
                        HDR.WAV.posis=[4,ftell(HDR.FILE.FID)];
                        fwrite(HDR.FILE.FID,tagsize,'uint32');	
                        
                        if rem(tagsize,2)
                                fprintf(HDR.FILE.stderr,'Error SOPEN WAV: data section has odd number of samples.\n. This violates the WAV specification\n');
                                fclose(HDR.FILE.FID);
                                HDR.FILE.OPEN = 0;
                                return;  
                        end;
                        
                        HDR.HeadLen = ftell(HDR.FILE.FID);
                end;
        end;

        
elseif strcmp(HDR.TYPE,'FLAC'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        
	if HDR.FILE.FID > 0,
    	        HDR.magic  = fread(HDR.FILE.FID,[1,4],'char');
		
		% read METADATA_BLOCK
		% 	read METADATA_BLOCK_HEADER
    	        tmp = fread(HDR.FILE.FID,[1,4],'uchar')
		while (tmp(1)<128),
			BLOCK_TYPE = mod(tmp(1),128);
			LEN = tmp(2:4)*2.^[0;8;16];
			POS = ftell(HDR.FILE.FID);
			if (BLOCK_TYPE == 0),		% STREAMINFO
				minblksz = fread(HDR.FILE.FID,1,'uint16')
				maxblksz = fread(HDR.FILE.FID,1,'uint16')
				minfrmsz = 2.^[0,8,16]*fread(HDR.FILE.FID,3,'uint8')
				maxfrmsz = 2.^[0,8,16]*fread(HDR.FILE.FID,3,'uint8')
				%Fs = fread(HDR.FILE.FID,3,'ubit20')
			elseif (BLOCK_TYPE == 1),	% PADDING
			elseif (BLOCK_TYPE == 2),	% APPLICATION
				HDR.FLAC.Reg.Appl.ID = fread(HDR.FILE.FID,1,'uint32')
			elseif (BLOCK_TYPE == 3),	% SEEKTABLE
				HDR.EVENT.N = LEN/18;
				for k = 1:LEN/18,
	    				HDR.EVENT.POS(k) = 2.^[0,32]*fread(HDR.FILE.FID,2,'uint32');
	    				HDR.EVENT.DUR(k) = 2.^[0,32]*fread(HDR.FILE.FID,2,'uint32');
	    				HDR.EVENT.nos(k) = fread(HDR.FILE.FID,1,'uint16');
				
				end;
			elseif (BLOCK_TYPE == 4),	% VORBIS_COMMENT
			elseif (BLOCK_TYPE == 5),	% CUESHEET
			else					% reserved
			end;
			
			fseek(HDR.FILE.FID, POS+LEN,'bof');
        	        tmp = fread(HDR.FILE.FID,[1,4],'uchar')
		end;
		
		% 	read METADATA_BLOCK_DATA

		% read METADATA_BLOCK_DATA
		% 	read METADATA_BLOCK_STREAMINFO
		% 	read METADATA_BLOCK_PADDING
		% 	read METADATA_BLOCK_APPLICATION
		% 	read METADATA_BLOCK_SEEKTABLE
		% 	read METADATA_BLOCK_COMMENT
		% 	read METADATA_BLOCK_CUESHEET

		% read FRAME
		%	read FRAME_HEADER
		%	read FRAME_SUBFRAME
		%		read FRAME_SUBFRAME_HEADER
		%	read FRAME_HEADER

                fclose(HDR.FILE.FID)        

                fprintf(HDR.FILE.stderr,'Warning SOPEN: FLAC not ready for use\n');
		return;
        end;

        
elseif strcmp(HDR.TYPE,'OGG'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        
	if HDR.FILE.FID > 0,
		% chunk header
		tmp = fread(HDR.FILE.FID,1,'uchar');
		QualityIndex = mod(tmp(1),64);
		if (tmp(1)<128), % golden frame 
			tmp = fread(HDR.FILE.FID,2,'uchar');
			HDR.VERSION = tmp(1);
			HDR.VP3.Version = floor(tmp(2)/8);
			HDR.OGG.KeyFrameCodingMethod = floor(mod(tmp(2),8)/4);
		end;
		
		% block coding information
		% coding mode info
		% motion vectors
		% DC coefficients
		% DC coefficients
		% 1st AC coefficients
		% 2nd AC coefficients
		% ...
		% 63rd AC coefficient

                fclose(HDR.FILE.FID);        
                fprintf(HDR.FILE.stderr,'Warning SOPEN: OGG not ready for use\n');
		return;
        end;

        
elseif strcmp(HDR.TYPE,'RMF'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        
	if HDR.FILE.FID > 0,
                fclose(HDR.FILE.FID)        

                fprintf(HDR.FILE.stderr,'Warning SOPEN: RMF not ready for use\n');
		return;
        end;

        
elseif strcmp(HDR.TYPE,'EGI'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        
        HDR.VERSION  = fread(HDR.FILE.FID,1,'uint32');
        
        if ~(HDR.VERSION >= 2 & HDR.VERSION <= 7),
                %   fprintf(HDR.FILE.stderr,'EGI Simple Binary Versions 2-7 supported only.\n');
        end;
        
        HDR.T0 = fread(HDR.FILE.FID,[1,6],'uint16');
        millisecond = fread(HDR.FILE.FID,1,'uint32');
        HDR.T0(6) = HDR.T0(6) + millisecond/1000;
        
        HDR.SampleRate = fread(HDR.FILE.FID,1,'uint16');
        HDR.NS   = fread(HDR.FILE.FID,1,'uint16');
        HDR.gain = fread(HDR.FILE.FID,1,'uint16');
        HDR.bits = fread(HDR.FILE.FID,1,'uint16');
        HDR.DigMax  = 2^HDR.bits;
        HDR.PhysMax = fread(HDR.FILE.FID,1,'uint16');
        if ( HDR.bits ~= 0 & HDR.PhysMax ~= 0 )
                HDR.Cal = (HDR.PhysMax/HDR.DigMax);
        else
                HDR.Cal = 1;
        end;
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal,HDR.NS+1,HDR.NS);
        HDR.PhysDim = 'uV';
        HDR.Label = char(zeros(HDR.NS,5));
        for k=1:HDR.NS,
                HDR.Label(k,:)=sprintf('# %3i',k);
        end;
        
        if CHAN<1, CHAN=1:HDR.NS; end;
        
        HDR.categories = 0;
        HDR.EGI.catname= {};
        
        if any(HDR.VERSION==[2,4,6]),
                HDR.SPR  = fread(HDR.FILE.FID, 1 ,'int32');
                HDR.EVENT.N = fread(HDR.FILE.FID,1,'int16');
                HDR.NRec = 1;
                HDR.FLAG.TRIGGERED = logical(0); 
                HDR.AS.spb = HDR.NS;
                HDR.AS.endpos = HDR.SPR;
                HDR.Dur = 1/HDR.SampleRate;
        elseif any(HDR.VERSION==[3,5,7]),
                HDR.EGI.categories = fread(HDR.FILE.FID,1,'uint16');
                if (HDR.EGI.categories),
                        for i=1:HDR.EGI.categories,
                                catname_len(i) = fread(HDR.FILE.FID,1,'uchar');
                                HDR.EGI.catname{i} = char(fread(HDR.FILE.FID,catname_len(i),'uchar'))';
                        end
                end
                HDR.NRec = fread(HDR.FILE.FID,1,'int16');
                HDR.SPR  = fread(HDR.FILE.FID,1,'int32');
                HDR.EVENT.N = fread(HDR.FILE.FID,1,'int16');
                HDR.FLAG.TRIGGERED = logical(1); 
                HDR.AS.spb = HDR.SPR*(HDR.NS+HDR.EVENT.N);
                HDR.AS.endpos = HDR.NRec;
                HDR.Dur = HDR.SPR/HDR.SampleRate;
        else
                fprintf(HDR.FILE.stderr,'Invalid EGI version %i\n',HDR.VERSION);
                return;
        end
        
        % get datatype from version number
        if any(HDR.VERSION==[2,3]),
                HDR.GDFTYP = 'int16';
                HDR.AS.bpb = HDR.AS.spb*2;
        elseif any(HDR.VERSION==[4,5]),
                HDR.GDFTYP = 'float32';
                HDR.AS.bpb = HDR.AS.spb*4;
        elseif any(HDR.VERSION==[6,7]),
                HDR.GDFTYP = 'float64';
                HDR.AS.bpb = HDR.AS.spb*8;
        else
                error('Unknown data format');
        end
        HDR.AS.bpb = HDR.AS.bpb + 6*HDR.FLAG.TRIGGERED;
        
        tmp = fread(HDR.FILE.FID,[4,HDR.EVENT.N],'uchar');
        HDR.EGI.eventcode = reshape(tmp,[4,HDR.EVENT.N])';
        HDR.EVENT.TYP = HDR.EGI.eventcode*(2.^[24;16;8;0]);
        
        HDR.HeadLen = ftell(HDR.FILE.FID);
        HDR.FILE.POS= 0;
	HDR.FILE.OPEN = 1; 
        
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
        if   	HDR.Samptype==2, HDR.GDFTYP = 'int16';
        elseif 	HDR.Samptype==4, HDR.GDFTYP = 'float32'; 
        else
                fprintf(HDR.FILE.stderr,'Error SOPEN TEAM-format: invalid file\n');
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
        
        HDR.EVENT.N   = fread(HDR.FILE.FID,1,'int16');
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
                HDR.EventLabels = setstr(fread(HDR.FILE.FID,[16,HDR.EVENT.N],'char')');
                
                % Events could be detected in this way
                % HDR.Events = zeros(HDR.SPR*HDR.NRec,1);
                % for k = 1:ceil(HDR.EVENT.N/16)
                %	HDR.Events = HDR.Events + 2^(16*k-16)*fread(HDR.FILE.FID,HDR.SPR*HDR.NRec,'uint16');
                % end;
        end;
        
        % Segment information block entries 
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
        fprintf(HDR.FILE.stderr,'Warning SOPEN: Implementing Nicolet TEAM file format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
        fclose(HDR.FILE.FID);
        
        
elseif strcmp(HDR.TYPE,'WFT'),	% implementation of this format is not finished yet.
        
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        [s,c] = fread(HDR.FILE.FID,1536,'char');
        [tmp,s] = strtok(s,setstr([0,32]));
        Nic_id0 = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        Niv_id1 = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        Nic_id2 = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        User_id = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.HeadLen = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.FILE.Size = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.VERSION = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.WFT.WaveformTitle = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.T0(1) = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.T0(1,2) = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.T0(1,3) = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        tmp = str2double(tmp);
        HDR.T0(1,4:6) = [floor(tmp/3600000),floor(rem(tmp,3600000)/60000),rem(tmp,60000)];
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.SPR = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.Off = str2double(tmp);
        [tmp,s] = strtok(s,setstr([0,32]));
        HDR.Cal = str2double(tmp);
        
        fseek(HDR.FILE.FID,HDR.HeadLen,'bof');
        
        fprintf(HDR.FILE.stderr,'Warning SOPEN: Implementing Nicolet WFT file format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
        fclose(HDR.FILE.FID);
        
        
elseif strcmp(HDR.TYPE,'WG1'),

        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
                HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
                
                HDR.Version = dec2hex(fread(HDR.FILE.FID,1,'uint32')); 
                HDR.WG1.MachineId = fread(HDR.FILE.FID,1,'uint32');
                HDR.WG1.Day = fread(HDR.FILE.FID,1,'uint32'); 
                HDR.WG1.millisec = fread(HDR.FILE.FID,1,'uint32');
		HDR.T0    = datevec(HDR.WG1.Day-15755-hex2dec('250000'));
		HDR.T0(1) = HDR.T0(1) + 1970;
		HDR.T0(4) = floor(HDR.WG1.millisec/3600000);
		HDR.T0(5) = mod(floor(HDR.WG1.millisec/60000),60);
		HDR.T0(6) = mod(HDR.WG1.millisec/1000,60);
                dT = fread(HDR.FILE.FID,1,'uint32');
                HDR.SampleRate = 1e6/dT;
                HDR.WG1.pdata = fread(HDR.FILE.FID,1,'uint16');
                HDR.NS = fread(HDR.FILE.FID,1,'uint16'); 
                HDR.WG1.poffset = fread(HDR.FILE.FID,1,'uint16');
                HDR.WG1.pad1 = fread(HDR.FILE.FID,38,'char');
		HDR.Cal = repmat(NaN,HDR.NS,1);
		HDR.ChanSelect = repmat(NaN,HDR.NS,1);
                for k=1:HDR.NS,
                        Label(k,1:8) = fread(HDR.FILE.FID,[1,8],'char');
                        HDR.Cal(k,1) = fread(HDR.FILE.FID,1,'uint32')/1000;
                        tmp = fread(HDR.FILE.FID,[1,2],'uint16');
                        HDR.ChanSelect(k) = tmp(1)+1;
                end;
		HDR.Label = char(Label);
		HDR.Calib = sparse(2:HDR.NS+1,HDR.ChanSelect,HDR.Cal);

                status = fseek(HDR.FILE.FID,7*256,'bof');
                HDR.WG1.neco1 = fread(HDR.FILE.FID,1,'uint32');
                HDR.Patient.Id = fread(HDR.FILE.FID,[1,12],'char');
                HDR.Patient.LastName = fread(HDR.FILE.FID,[1,20],'char');
                HDR.Patient.text1 = fread(HDR.FILE.FID,[1,20],'char');
                HDR.Patient.FirstName = fread(HDR.FILE.FID,[1,20],'char');
                HDR.Patient.Sex = fread(HDR.FILE.FID,[1,2],'char');
                HDR.Patient.vata = fread(HDR.FILE.FID,[1,8],'char');
                HDR.Patient.text2 = fread(HDR.FILE.FID,[1,14],'char');
                HDR.WG1.Datum = fread(HDR.FILE.FID,1,'uint32');
                HDR.WG1.mstime = fread(HDR.FILE.FID,1,'uint32');
                HDR.WG1.nic = fread(HDR.FILE.FID,[1,4],'uint32');
                HDR.WG1.neco3 = fread(HDR.FILE.FID,1,'uint32');
               
                status = fseek(HDR.FILE.FID,128,'cof');
                HDR.HeadLen = ftell(HDR.FILE.FID);
                HDR.FILE.OPEN = 1; 
		HDR.FILE.POS  = 0; 

		HDR.WG1.szBlock  = 256;  
		HDR.WG1.szOffset = 128;
		HDR.WG1.szExtra  = HDR.WG1.pdata-(HDR.NS+HDR.WG1.poffset);
		szOneRec = HDR.WG1.szOffset*4+(HDR.NS+HDR.WG1.szExtra)*HDR.WG1.szBlock;
		HDR.AS.bpb = szOneRec;
		HDR.WG1.szRecs = floor((HDR.FILE.size-HDR.HeadLen)/HDR.AS.bpb);
		HDR.WG1.szData = HDR.WG1.szBlock*HDR.WG1.szRecs;
    		HDR.WG1.unknownNr = 11;
        	conv = round(19*sinh((0:127)/19));
		conv = [conv, HDR.WG1.unknownNr, -conv(end:-1:2)];
    		HDR.WG1.conv = conv;

		HDR.NRec = HDR.WG1.szRecs;
		HDR.SPR  = HDR.WG1.szBlock;
		HDR.Dur  = HDR.SPR/HDR.SampleRate;
		HDR.AS.endpos = HDR.NRec*HDR.SPR;
		
		%----- load event information -----
		eventFile = fullfile(HDR.FILE.Path,[HDR.FILE.Name, '.wg2']);
		if ~exist(eventFile,'file')
			eventFile = fullfile(HDR.FILE.Path,[HDR.FILE.Name, '.WG2']);
		end;	
		if exist(eventFile,'file')
    			fid= fopen(eventFile,'r');
    			nr = 1;
			[s,c] = fread(fid,1,'uint32');
    			while ~feof(fid)
        			HDR.EVENT.POS(nr,1) = s;
        			pad = fread(fid,3,'uint32');
        			len = fread(fid,1,'uint8');
        			tmp = char(fread(fid,[1,47], 'char'));
				HDR.EVENT.Desc{nr,1} = tmp(1:len);  
    				% find string between quotation marks
				%  HDR.EVENT.Desc{nr}=regexpi(Event,'(?<=\'').*(?=\'')','match','once');
				[s,c] = fread(fid,1,'uint32');
        			nr  = nr+1;
		        end;
    			HDR.EVENT.TYP = zeros(size(HDR.EVENT.POS));
			fclose(fid);
		end;
        end;

        
elseif strcmp(HDR.TYPE,'LDR'),
        HDR = openldr(HDR,PERMISSION);      
        
        
elseif strcmp(HDR.TYPE,'SMA'),  % under constructions
        PERMISSION = PERMISSION(PERMISSION~='b');
        try     % MatLAB default is binary, force Mode='rt';
                HDR.FILE.FID = fopen(HDR.FileName,[PERMISSION,'t'],'ieee-le');
        catch 	% Octave 2.1.50 default is text, but does not support Mode='rt', 
                HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        end
        numbegin=0;
        HDR.H1 = '';
        delim = char([abs('"='),10,13]);
        while ~numbegin,
                line = fgetl(HDR.FILE.FID);
                HDR.H1 = [HDR.H1, line];
                if strncmp('"NCHAN%"',line,8) 
                        [tmp,line] = strtok(line,'=');
                        [tmp,line] = strtok(line,delim);
                        HDR.NS = str2double(char(tmp));
                end
                if strncmp('"NUM.POINTS"',line,12) 
                        [tmp,line] = strtok(line,'=');
                        [tmp,line] = strtok(line,delim);
                        HDR.SPR = str2double(tmp);
                end
                if strncmp('"ACT.FREQ"',line,10) 
                        [tmp,line] = strtok(line,'=');
                        [tmp,line] = strtok(line,delim);
                        HDR.SampleRate= str2double(tmp);
                end
                if strncmp('"DATE$"',line,7)
                        [tmp,line] = strtok(line,'=');
                        [date,line] = strtok(line,delim);
                        [tmp,date]=strtok(date,'-');
                        HDR.T0(3) = str2double(tmp);
                        [tmp,date]=strtok(date,'-');
                        HDR.T0(2) = str2double(tmp);
                        [tmp,date]=strtok(date,'-');
                        HDR.T0(1) = str2double(tmp);
                end
                if strncmp('"TIME$"',line,7)
                        [tmp,line] = strtok(line,'=');
                        [time,line] = strtok(line,delim);
                        [tmp,date]=strtok(time,':');
                        HDR.T0(4) = str2double(tmp);
                        [tmp,date]=strtok(date,':');
                        HDR.T0(5) = str2double(tmp);
                        [tmp,date]=strtok(date,':');
                        HDR.T0(6) = str2double(tmp);
                end;
                if strncmp('"UNITS$[]"',line,10)
                        [tmp,line] = strtok(char(line),'=');
                        for k=1:HDR.NS,
                                [tmp,line] = strtok(line,[' ,',delim]);
                                HDR.PhysDim(k,1:length(tmp)) = tmp;
                        end;
                end
                if strncmp('"CHANNEL.RANGES[]"',line,18)
                        [tmp,line] = strtok(line,'= ');
                        [tmp,line] = strtok(line,'= ');
                        for k=1:HDR.NS,
                                [tmp,line] = strtok(line,[' ',delim]);
                                [tmp1, tmp]=strtok(tmp,'(),');
                                HDR.PhysMin(k,1)=str2double(tmp1);
                                [tmp2, tmp]=strtok(tmp,'(),');
                                HDR.PhysMax(k,1)=str2double(tmp2);
                        end;
                end
                if strncmp('"CHAN$[]"',line,9)
                        [tmp,line] = strtok(line,'=');
                        for k=1:HDR.NS,	
                                [tmp,line] = strtok(line,[' ,',delim]);
                                HDR.Label(k,1:length(tmp)) = char(tmp);
                        end;
                end
                if 0,strncmp('"CHANNEL.LABEL$[]"',line,18)
                        [tmp,line] = strtok(line,'=');
                        for k=1:HDR.NS,
                                [HDR.Label{k,1},line] = strtok(line,delim);
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
        
        %%%%%%%%%%%%%%%%%%% check file length %%%%%%%%%%%%%%%%%%%%
        
        HDR.FILE.POS= 0;
        HDR.HeadLen = ftell(HDR.FILE.FID);  % Length of Header
        fseek(HDR.FILE.FID,0,'eof'); 
        endpos = ftell(HDR.FILE.FID); 

        fclose(HDR.FILE.FID);
        PERMISSION = PERMISSION(PERMISSION~='t');       % open in binary mode 
        HDR.FILE.FID = fopen(HDR.FileName,[PERMISSION,'b'],'ieee-le');
        
        fseek(HDR.FILE.FID,HDR.HeadLen,'bof');
        %[HDR.AS.endpos,HDR.HeadLen,HDR.NS,HDR.SPR,HDR.NS*HDR.SPR*4,HDR.AS.endpos-HDR.HeadLen - HDR.NS*HDR.SPR*4]
        HDR.AS.endpos = HDR.NS*HDR.SPR*4 - HDR.HeadLen;
        if endpos-HDR.HeadLen ~= HDR.NS*HDR.SPR*4;
                fprintf(HDR.FILE.stderr,'Warning SOPEN TYPE=SMA: Header information does not fit size of file\n');
                fprintf(HDR.FILE.stderr,'\tProbably more than one data segment - this is not supported in the current version of SOPEN\n');
        end
        HDR.AS.bpb    = HDR.NS*4;
        HDR.AS.endpos = (HDR.AS.endpos-HDR.HeadLen)/HDR.AS.bpb;
        HDR.Dur = 1/HDR.SampleRate;
        HDR.NRec = 1;
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);

        if ~isfield(HDR,'SMA')
                HDR.SMA.EVENT_CHANNEL= 1;
                HDR.SMA.EVENT_THRESH = 2.3;
        end;
        HDR.Filter.T0 = zeros(1,length(HDR.SMA.EVENT_CHANNEL));
        
        
elseif strcmp(HDR.TYPE,'RDF'),  % UCSD ERPSS acqusition software DIGITIZE
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        
        status = fseek(HDR.FILE.FID,6,-1);
        HDR.NS = fread(HDR.FILE.FID,1,'uint16');
        status = fseek(HDR.FILE.FID,552,-1);
        HDR.SampleRate  = fread(HDR.FILE.FID,1,'uint16');
        status = fseek(HDR.FILE.FID,580,-1);
        tmp = fread(HDR.FILE.FID,[8,HDR.NS],'char');
        HDR.Label = char(tmp');
        
        cnt = 0;
        ev_cnt = 0;
        ev = [];
        
        % first pass, scan data
        totalsize = 0;
        tag = fread(HDR.FILE.FID,1,'uint32');
        while ~feof(HDR.FILE.FID) %& ~status,
                if tag == hex2dec('f0aa55'),
                        cnt = cnt + 1;
                        HDR.Block.Pos(cnt) = ftell(HDR.FILE.FID);
                        
                        % Read nchans and block length
                        tmp = fread(HDR.FILE.FID,34,'uint16');
                        
                        %fseek(HDR.FILE.FID,2,0);
                        nchans = tmp(2); %fread(HDR.FILE.FID,1,'uint16');
                        %fread(HDR.FILE.FID,1,'uint16');
                        block_size = 2^tmp(3); %fread(HDR.FILE.FID,1,'uint16');
                        %ndupsamp = fread(HDR.FILE.FID,1,'uint16');
                        %nrun = fread(HDR.FILE.FID,1,'uint16');
                        %err_detect = fread(HDR.FILE.FID,1,'uint16');
                        %nlost = fread(HDR.FILE.FID,1,'uint16');
                        HDR.EVENT.N = tmp(9); %fread(HDR.FILE.FID,1,'uint16');
                        %fseek(HDR.FILE.FID,50,0);

                        % Read events
                        HDR.EVENT.POS = repmat(nan,HDR.EVENT.N,1);
                        HDR.EVENT.TYP = repmat(nan,HDR.EVENT.N,1);
                        for i = 1:HDR.EVENT.N,
                                tmp = fread(HDR.FILE.FID,2,'uint8');
                                %cond_code = fread(HDR.FILE.FID,1,'uint8');
                                ev_code = fread(HDR.FILE.FID,1,'uint16');
                                ev_cnt  = ev_cnt + 1;
                                tmp2.sample_offset = tmp(1) + (cnt-1)*128;
                                tmp2.cond_code     = tmp(2);
                                tmp2.event_code    = ev_code;
                                if ~exist('OCTAVE_VERSION','builtin'), 	   
                                        ev{ev_cnt} = tmp2;
                                end;
                                HDR.EVENT.POS(ev_cnt) = tmp(1) + (cnt-1)*128;
                                HDR.EVENT.TYP(ev_cnt) = ev_code;
                        end;
                        status = fseek(HDR.FILE.FID,4*(110-HDR.EVENT.N)+2*nchans*block_size,0);
                else
                        [tmp, c] = fread(HDR.FILE.FID,3,'uint16');
			if (c > 2),
	                        nchans = tmp(2); %fread(HDR.FILE.FID,1,'uint16');
    		                block_size = 2^tmp(3); %fread(HDR.FILE.FID,1,'uint16');

            		        %fseek(HDR.FILE.FID,62+4*(110-HDR.EVENT.N)+2*nchans*block_size,0);
				sz = 62 + 4*110 + 2*nchans*block_size;
				status = -(sz>=(2^31));
				if ~status,
				        status = fseek(HDR.FILE.FID, sz, 0);
				end;	
			end;
                end
                tag = fread(HDR.FILE.FID,1,'uint32');
        end
        HDR.NRec = cnt;
        
        HDR.Events = ev;
        HDR.HeadLen = 0;
        HDR.FLAG.TRIGGERED = 1;	        
        HDR.FILE.POS = 0; 
        HDR.SPR = block_size;
        HDR.AS.bpb = HDR.SPR*HDR.NS*2;
        HDR.Dur = HDR.SPR/HDR.SampleRate;
	HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
        
        
elseif strcmp(HDR.TYPE,'LABVIEW'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        
        tmp = fread(HDR.FILE.FID,8,'uchar');
        HDR.VERSION = char(fread(HDR.FILE.FID,[1,8],'uchar'));
        HDR.AS.endpos = fread(HDR.FILE.FID,1,'int32'); % 4 first bytes = total header length
        
        HDR.HeadLen  = fread(HDR.FILE.FID,1,'int32'); % 4 first bytes = total header length
        HDR.NS       = fread(HDR.FILE.FID,1,'int32');  % 4 next bytes = channel list string length
        HDR.AS.endpos2 = fread(HDR.FILE.FID,1,'int32'); % 4 first bytes = total header length
        
        HDR.ChanList = fread(HDR.FILE.FID,HDR.NS,'uchar'); % channel string
        
        fclose(HDR.FILE.FID);
        %HDR.FILE.OPEN = 1;
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
        
        HDR.Cal = 1;
        
        
elseif strcmp(HDR.TYPE,'RG64'),
        fid = fopen(HDR.FileName,PERMISSION,'ieee-le');
        
        HDR.IDCODE=char(fread(fid,[1,4],'char'));	%
        if ~strcmp(HDR.IDCODE,'RG64') 
                fprintf(HDR.FILE.stderr,'\nError LOADRG64: %s not a valid RG64 - header file\n',HDR.FileName); 
                HDR.TYPE = 'unknown';
                fclose(fid);
                return;
        end; %end;
        
        tmp = fread(fid,2,'int32');
        HDR.VERSION = tmp(1)+tmp(2)/100;
        HDR.NS = fread(fid,1,'int32');
        HDR.SampleRate = fread(fid,1,'int32');
        HDR.SPR = fread(fid,1,'int32')/HDR.NS;
        AMPF = fread(fid,64,'int32');		
        fclose(fid);
        
        HDR.HeadLen = 0;
        HDR.PhysDim = 'uV';
        HDR.Cal = (5E6/2048)./AMPF;
        HDR.AS.endpos = HDR.SPR;
        HDR.AS.bpb    = HDR.NS*2;
        HDR.GDFTYP    = 'int16';
        
        EXT = HDR.FILE.Ext; 
        if upper(EXT(2))~='D',
                EXT(2) = EXT(2) - 'H' + 'D';
        end;
        FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',EXT]);
        
        HDR.FILE.FID=fopen(FILENAME,'rb','ieee-le');
        if HDR.FILE.FID<0,
                fprintf(HDR.FILE.stderr,'\nError LOADRG64: data file %s not found\n',FILENAME); 
                return;
        end;

        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal(1:HDR.NS),HDR.NS+1,HDR.NS);
        HDR.FILE.POS = 0; 
        HDR.FILE.OPEN= 1;
        
        
elseif strcmp(HDR.TYPE,'DDF'),
        
        % implementation of this format is not finished yet.
        fprintf(HDR.FILE.stderr,'Warning SOPEN: Implementing DASYLAB format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
        %HDR.FILE.FID = -1;
        %return;
        
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS = 0;
                %HDR.ID = fread(HDR.FILE.FID,5,'char');
                ds=fread(HDR.FILE.FID,[1,128],'char');
                HDR.ID = setstr(ds(1:5));
                DataSource = ds;
                k = 0;
                while ~(any(ds==26)),
                        ds = fread(HDR.FILE.FID,[1,128],'char');
                        DataSource = [DataSource,ds];
                        k = k+1;	
                end;	
                pos = find(ds==26)+k*128;
                DataSource = setstr(DataSource(6:pos));
                HDR.DDF.Source = DataSource;
                while ~isempty(DataSource),
                        [ds,DataSource] = strtok(setstr(DataSource),[10,13]);
                        [field,value] = strtok(ds,'=');
                        if strfind(field,'SAMPLE RATE');
                                [tmp1,tmp2] = strtok(value,'=');
                                HDR.SampleRate = str2double(tmp1);
                        elseif strfind(field,'DATA CHANNELS');
                                HDR.NS = str2double(value);
                        elseif strfind(field,'START TIME');
                                Time = value;
                        elseif strfind(field,'DATA FILE');
                                HDR.FILE.DATA = value;
                        end;			 	
                end;
                fseek(HDR.FILE.FID,pos,'bof'); 	% position file identifier
                if 0;%DataSource(length(DataSource))~=26,
                        fprintf(1,'Warning: DDF header seems to be incorrenct. Contact <alois.schloegl@tugraz.at> Subject: BIOSIG/DATAFORMAT/DDF  \n');
                end;
                HDR.DDF.CPUidentifier  = setstr(fread(HDR.FILE.FID,[1,2],'char'));
                HDR.HeadLen(1) = fread(HDR.FILE.FID,1,'uint16');
                tmp = fread(HDR.FILE.FID,1,'uint16');
                if tmp == 0, HDR.GDFTYP = 'uint16'; 		% streamer format (data are raw data in WORD=UINT16)
                elseif tmp == 1, HDR.GDFTYP = 'float32'; 	% Universal Format 1 (FLOAT)
                elseif tmp == 2, HDR.GDFTYP = 'float64'; 	% Universal Format 2 (DOUBLE)
                elseif tmp <= 1000, % reserved
                else		% unused
                end;
                HDR.FILE.Type  = tmp;
                HDR.VERSION    = fread(HDR.FILE.FID,1,'uint16');
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
                % fseek(HDR.FILE.FID,HDR.HeadLen(1),'bof');
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
                        HDR.pos1 = ftell(HDR.FILE.FID);
                        tmp = fread(HDR.FILE.FID,1,'uint16');	% size of this header 
                        if (tmp~=HDR.HeadLen(2)),
                                fprintf(HDR.FILE.stderr,'Error SOPEN DDF: error in header of file %s\n',HDR.FileName);
                        end;
                        HDR.U1G.NS = fread(HDR.FILE.FID,1,'uint16');	% number of channels
                        HDR.FLAG.multiplexed = fread(HDR.FILE.FID,1,'uint16');	% multiplexed: 0=no, 1=yes
                        HDR.DDF.array = fread(HDR.FILE.FID,[1,16],'uint16');	% array of channels collected on each input channel
                        
                        % channel header
                        for k = 1:HDR.NS,
                                filepos = ftell(HDR.FILE.FID);    
                                taglen = fread(HDR.FILE.FID,1,'uint16');	% size of this header
                                ch = fread(HDR.FILE.FID,1,'uint16');	% channel number
                                HDR.DDF.MAXSPR(ch+1)= fread(HDR.FILE.FID,1,'uint16');	% maximum size of block in samples
                                HDR.DDF.delay(ch+1) = fread(HDR.FILE.FID,1,'double');	% time delay between two samples
                                HDR.DDF.ChanType(ch+1) = fread(HDR.FILE.FID,1,'uint16');	% channel type 
                                HDR.DDF.ChanFlag(ch+1) = fread(HDR.FILE.FID,1,'uint16');	% channel flag 
                                unused = fread(HDR.FILE.FID,2,'double');	% must be 0.0 for future extension
                                tmp = fgets(HDR.FILE.FID);	% channel unit
                                HDR.PhysDim = strvcat(HDR.PhysDim, tmp);	% channel unit
                                tmp = fgets(HDR.FILE.FID);		% channel name 
                                HDR.Label = strvcat(HDR.Label, tmp);		% channel name 
                                fseek(HDR.FILE.FID,filepos+taglen,'bof');
                        end;
                        
                        % channel header
                        for k = 1:HDR.NS,
                                fread(HDR.FILE.FID,[1,4],'char');
                                fread(HDR.FILE.FID,1,'uint16');	% overall number of bytes in this header
                                HDR.BlockStartTime = fread(HDR.FILE.FID,1,'uint32');  % might be incorrect
                                unused = fread(HDR.FILE.FID,2,'double');	% must be 0.0 for future extension
                                ch = fread(HDR.FILE.FID,1,'uint32');  % channel number
                        end;    
                        fseek(HDR.FILE.FID,HDR.pos1+sum(HDR.HeadLen(2:3)),'bof');
                        
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
                %ftell(HDR.FILE.FID),
                tag=fread(HDR.FILE.FID,[1,4],'char');
        end;
        return;         

        
elseif strcmp(HDR.TYPE,'MIT')
        if any(PERMISSION=='r'),
                HDR.FileName = fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext]);
                
                HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-le');
                if HDR.FILE.FID<0,
			fprintf(HDR.FILE.stderr,'Error SOPEN: Couldnot open file %s\n',HDR.FileName);
			return;
		end;	
                
                fid = HDR.FILE.FID;
                z   = fgetl(fid);
                while strncmp(z,'#',1) | isempty(z),
                        z   = fgetl(fid);
                end;
                tmpfile = strtok(z,' /');
                if ~strcmpi(HDR.FILE.Name,tmpfile),
                        fprintf(HDR.FILE.stderr,'Warning: RecordName %s does not fit filename %s\n',tmpfile,HDR.FILE.Name);
                end;	
                
                %A = sscanf(z, '%*s %d %d %d',[1,3]);
                [tmp,z] = strtok(z); 
                [tmp,z] = strtok(z);
                HDR.NS  = str2double(tmp);   % number of signals
                [tmp,z] = strtok(z); 
                HDR.SampleRate = str2double(tmp);   % sample rate of data
                [tmp,z] = strtok(z,' ()');
                HDR.SPR   = str2double(tmp);   % sample rate of data
                HDR.NRec  = 1;
                
                HDR.MIT.gain = zeros(1,HDR.NS);
		HDR.MIT.zerovalue  = repmat(NaN,1,HDR.NS);
		HDR.MIT.firstvalue = repmat(NaN,1,HDR.NS);
                for k = 1:HDR.NS,
                        z = fgetl(fid);
                        [HDR.FILE.DAT,z]=strtok(z);
                        for k0 = 1:7,
                                [tmp,z] = strtok(z);
                                if k0 == 1, 
                                        [tmp, tmp1] = strtok(tmp,'x:');
                                        [tmp, status] = str2double(tmp); 
                                        HDR.MIT.dformat(k,1) = tmp;
                                        if isempty(tmp1)
                                                HDR.AS.SPR(k) = 1; 
                                        elseif tmp1(1)=='x'
                                                HDR.AS.SPR(k) = str2double(tmp1(2:end)); 
                                        else
                                                HDR.AS.SPR(k) = 1; 
                                        end                                                
                                elseif k0==2,  
                                        % EC13*.HEA files have special gain values like "200(23456)/uV". 
                                        [tmp, tmp2] = strtok(tmp,'/');
					tmp2 = [tmp2(2:end),' '];
                                        HDR.PhysDim(k,1:length(tmp2)) = tmp2;
                                        [tmp, tmp1] = strtok(tmp,' ()');
                                        [tmp, status] = str2double(tmp); 
                                        if isempty(tmp), tmp = 0; end;   % gain
                                        if isnan(tmp),   tmp = 0; end;
                                        HDR.MIT.gain(1,k) = tmp;
                                elseif k0==3,
                                        [tmp, status] = str2double(tmp); 
                                        if isempty(tmp), tmp = NaN; end; 
                                        if isnan(tmp),   tmp = NaN; end;
                                        HDR.MIT.bitres(1,k) = tmp;
                                elseif k0==4,
                                        [tmp, status] = str2double(tmp);
                                        if isempty(tmp), tmp = 0; end;
                                        if isnan(tmp),   tmp = 0; end;
                                        HDR.MIT.zerovalue(1,k) = tmp; 
                                elseif k0==5, 
                                        [tmp, status] = str2double(tmp);
                                        if isempty(tmp), tmp = NaN; end; 
                                        if isnan(tmp),   tmp = NaN; end;
                                        HDR.MIT.firstvalue(1,k) = tmp;        % first integer value of signal (to test for errors)
                                else
                                        
                                end;
                        end;
                        HDR.Label(k,1:length(z)+1) = [z,' ']; 
                end;
                
                HDR.MIT.gain(HDR.MIT.gain==0) = 200;    % default gain 
                HDR.Calib = sparse([HDR.MIT.zerovalue; eye(HDR.NS)]*diag(1./HDR.MIT.gain(:)));
                HDR.Label = char(HDR.Label);
                
                z = char(fread(fid,[1,inf],'char'));
                ix1 = [strfind(upper(z),'AGE:')+4, strfind(upper(z),'AGE>:')+5];
                if ~isempty(ix1),
                        [tmp,z]=strtok(z(ix1(1):length(z)));
                        HDR.Patient.Age = str2double(tmp);
                end;
                ix1 = [strfind(upper(z),'SEX:')+4, strfind(upper(z),'SEX>:')+5];
                if ~isempty(ix1),
                        [HDR.Patient.Sex,z]=strtok(z(ix1(1):length(z)));
                end;
                ix1 = [strfind(upper(z),'DIAGNOSIS:')+10; strfind(upper(z),'DIAGNOSIS>:')+11];
                if ~isempty(ix1),
                        [HDR.Patient.Diagnosis,z]=strtok(z(ix1(1):length(z)),char([10,13,abs('#<>')]));
                end;
                ix1 = [strfind(upper(z),'MEDICATIONS:')+12, strfind(upper(z),'MEDICATIONS>:')+13];
                if ~isempty(ix1),
                        [HDR.Patient.Medication,z]=strtok(z(ix1(1):length(z)),char([10,13,abs('#<>')]));
                end;
                fclose(fid);

		%------ LOAD ATR FILE ---------------------------------------------------                        
		tmp = fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.atr']);
		if ~exist(tmp,'file'),
			tmp = fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.ATR']);
		end;
		if exist(tmp,'file'),
	                H = sopen(tmp);
			HDR.EVENT = H.EVENT; 
			HDR.EVENT.SampleRate = HDR.SampleRate;
		end;

                %------ LOAD BINARY DATA --------------------------------------------------
                if ~HDR.NS, 
                        return; 
                end;
                HDR.AS.spb = sum(HDR.AS.SPR);
                HDR.AS.bi = [0;cumsum(HDR.AS.SPR(:))]; 
                HDR.AS.MAXSPR = HDR.AS.SPR(1);
                for k = 2:HDR.NS,
                        HDR.AS.MAXSPR = lcm(HDR.AS.MAXSPR,HDR.AS.SPR(k));
                end;
                HDR.AS.SampleRate = HDR.SampleRate*HDR.AS.SPR;
                HDR.SampleRate = HDR.SampleRate*HDR.AS.MAXSPR;
                
                if all(HDR.MIT.dformat==HDR.MIT.dformat(1)),
                        HDR.VERSION = HDR.MIT.dformat(1);
                else
                        fprintf(HDR.FILE.stderr,'different DFORMATs not supported.\n');
                        HDR.FILE.FID = -1;
                        return;
                end;
                if 0,
                        
                elseif HDR.VERSION == 212, 
                        if mod(HDR.AS.spb,2) 
                                HDR.AS.spb = HDR.AS.spb*2;
                        end
                        HDR.AS.bpb = HDR.AS.spb*3/2;
                elseif HDR.VERSION == 310, 
                        if mod(HDR.AS.spb,3) 
                                HDR.AS.spb = HDR.AS.spb*2/3;
                        end
                        HDR.AS.bpb = HDR.AS.spb*2;
                elseif HDR.VERSION == 311, 
                        if mod(HDR.AS.spb,3) 
                                HDR.AS.spb = HDR.AS.spb*3;
                        end
                        HDR.AS.bpb = HDR.AS.spb*4;
                elseif HDR.VERSION == 8, 
                        HDR.AS.bpb = HDR.AS.spb;
                elseif HDR.VERSION == 80, 
                        HDR.AS.bpb = HDR.AS.spb;
                elseif HDR.VERSION == 160, 
                        HDR.AS.bpb = HDR.AS.spb;
                elseif HDR.VERSION == 16, 
                        HDR.AS.bpb = HDR.AS.spb;
                elseif HDR.VERSION == 61, 
                        HDR.AS.bpb = HDR.AS.spb;
                end;
                HDR.Dur = HDR.AS.MAXSPR/HDR.SampleRate;

                if HDR.VERSION ==61,
                        MACHINE_FORMAT='ieee-be';
                else
                        MACHINE_FORMAT='ieee-le';
                end;
                
                tmpfile = fullfile(HDR.FILE.Path,HDR.FILE.DAT);
                if  ~exist(tmpfile,'file'), 
                        HDR.FILE.DAT = upper(HDR.FILE.DAT);
                        tmpfile = fullfile(HDR.FILE.Path,HDR.FILE.DAT);
                end;
                if  ~exist(tmpfile,'file'), 
                        HDR.FILE.DAT = lower(HDR.FILE.DAT);
                        tmpfile = fullfile(HDR.FILE.Path,HDR.FILE.DAT);
                end;
                HDR.FILE.FID = fopen(tmpfile,'rb',MACHINE_FORMAT);
                if HDR.FILE.FID<0,
			fprintf(HDR.FILE.stderr,'Error SOPEN: Couldnot open file %s\n',tmpfile);
			return;
		end;	
		
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS = 0;
                HDR.HeadLen  = 0;
                fseek(HDR.FILE.FID,0,'eof');
                tmp = ftell(HDR.FILE.FID);
                try
                        HDR.AS.endpos = tmp/HDR.AS.bpb;
                catch
                        fprintf(HDR.FILE.stderr,'Warning 2003 SOPEN: FTELL does not return numeric value (Octave > 2.1.52).\nHDR.AS.endpos not completed.\n');
                end;
                fseek(HDR.FILE.FID,0,'bof');
                
                HDR.InChanSelect = 1:HDR.NS;
                FLAG_UCAL = HDR.FLAG.UCAL;	
                HDR.FLAG.UCAL = 1;
                S = NaN;
                [S,HDR] = sread(HDR,HDR.AS.MAXSPR/HDR.SampleRate); % load 1st sample
                if (HDR.VERSION>0) & (any(S(1,:) - HDR.MIT.firstvalue)), 
                        fprintf(HDR.FILE.stderr,'Warning SOPEN MIT-ECG: First values of header and datablock do not fit.\n\tHeader:\t'); 
                        fprintf(HDR.FILE.stderr,'\t%5i',HDR.MIT.firstvalue);
                        fprintf(HDR.FILE.stderr,'\n\tData 1:\t');
                        fprintf(HDR.FILE.stderr,'\t%5i',S(1,:));
                        fprintf(HDR.FILE.stderr,'\n');
                end;
                HDR.FLAG.UCAL = FLAG_UCAL ;	
                fseek(HDR.FILE.FID,0,'bof');	% reset file pointer
        end;
        
	
elseif strcmp(HDR.TYPE,'MIT-ATR'),
                %------ LOAD ATTRIBUTES DATA ----------------------------------------------
                fid = fopen(HDR.FileName,'rb','ieee-le');
                if fid<0,
                        A = []; c = 0;
                else
                        [A,c] = fread(fid, inf, 'uint16');
                        fclose(fid);
                end;
                
                ATRTIME = zeros(c,1);
                ANNOT   = zeros(c,1);
                K  = 0;
                i  = 1;
		ch = 0; 
		accu = 0; 
                while i<=size(A,1),
                        %annoth = bitshift(A(i,2),-2);
                        annoth = floor(A(i)/1024);
			L = rem(A(i),1024);
                        if A(i)==0,  % end of file
			  
			elseif annoth==60
				%[60,L,A(i)]
                                % nothing to do!
                        elseif annoth==61
				%[61,L,A(i)]
			        % nothing to do!
                        elseif annoth==62
				ch = L; 
                                % nothing to do!
                        elseif annoth==63
                        	i = i + ceil(rem(A(i),1024)/2);
                        else
				if annoth==59,
					if logical(L), 
						[59,L,A(i)]
					%	warning('ATR'); 
					end;
	                                ANNOT(K) = annoth; %bitshift(A(i+3,2),-2);
					L = (2.^[0,16])*[A(i+2);A(i+1)];
            		                i = i + 2;
                    		end;
                                K = K+1;
                                ATRTIME(K) = L; %256*rem(A(i,2),4)+A(i,1);
                                ANNOT(K)   = annoth; %bitshift(A(i,2),-2);
				accu = accu + L; 

				EVENT.TYP(K,1) = hex2dec('0501'); 
				EVENT.POS(K,1) = accu; 
				EVENT.CHN(K,1) = ch; 
				EVENT.DUR(K,1) = 0; 
                        end;
                        i = i + 1;
                end;
		HDR.EVENT = EVENT;
		HDR.TYPE = 'EVENT';
                
        
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
                HDR.Calib = sparse([HDR.Off';(diag(HDR.Cal))]);
                HDR.HeadLen = 217 + HDR.NS*136;
                
        end;
        
        
elseif 0,strcmp(HDR.TYPE,'DAQ'),
        HDR = daqopen(HDR,PERMISSION,CHAN);
        
        
elseif strcmp(HDR.TYPE,'MAT4') & any(PERMISSION=='r'),
    		HDR.FILE.FID = fopen(HDR.FileName,'rb',HDR.MAT4.opentyp);
                k=0; NB=0;
                %type = fread(HDR.FILE.FID,4,'uchar'); 	% 4-byte header
                type = fread(HDR.FILE.FID,1,'uint32'); 	% 4-byte header
                while ~isempty(type),
                        type = sprintf('%04i',type)';
                        type = type - abs('0');
                        k = k + 1;
                        [mrows,c] = fread(HDR.FILE.FID,1,'uint32'); 	% tag, datatype
                        ncols = fread(HDR.FILE.FID,1,'uint32'); 	% tag, datatype
                        imagf = fread(HDR.FILE.FID,1,'uint32'); 	% tag, datatype
                        namelen  = fread(HDR.FILE.FID,1,'uint32'); 	% tag, datatype
                        if namelen>HDR.FILE.size,
			%	fclose(HDR.FILE.FID);
				HDR.ERROR.status  = -1; 
				HDR.ERROR.message = sprintf('Error SOPEN (MAT4): Could not open %s\n',HDR.FileName);
				return;
			end;
			[name,c] = fread(HDR.FILE.FID,namelen,'char'); 
                        
                        if imagf, 
				HDR.ERROR.status=-1; 
				fprintf(HDR.FILE.stderr,'Warning %s: Imaginary data not tested\n',mfilename); 
			end;
                        if type(4)==2,
                                HDR.ERROR.status=-1;
                                fprintf(HDR.FILE.stderr,'Error %s: sparse data not supported\n',mfilename);
                        elseif type(4)>2, 
                                type(4)=rem(type(4),2);
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
                        if strfind(HDR.Var(k).Name,'data_block'),
                                HDR.ADI.DB(str2double(HDR.Var(k).Name(11:length(HDR.Var(k).Name))))=k;
                        elseif strfind(HDR.Var(k).Name,'ticktimes_block'),
                                HDR.ADI.TB(str2double(HDR.Var(k).Name(16:length(HDR.Var(k).Name))))=k;
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
                        
                        %type = fread(HDR.FILE.FID,4,'uchar');  	% 4-byte header
                        type = fread(HDR.FILE.FID,1,'uint32'); 	% 4-byte header
	        end;
	        HDR.FILE.OPEN = 1;
	        HDR.FILE.POS = 0;
        
	
    	if isfield(HDR,'ADI')
                HDR.TYPE = 'ADI', % ADICHT-data, converted into a Matlab 4 file
    		
	        fprintf(HDR.FILE.stderr,'Format not tested yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',PERMISSION);	

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
                                fprintf(HDR.FILE.stderr,'Warning MATOPEN: Units are different from block to block\n');
                        elseif any(any(HDR.ADI.units{k-1}~=tmp))
                                fprintf(HDR.FILE.stderr,'Warning MATOPEN: Units are different from block to block\n');
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
	        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,ones(1,HDR.NS));
        else        
                fclose(HDR.FILE.FID);
                HDR.FILE.FID = -1;
		return; 
        end;
        
        
elseif strcmp(HDR.TYPE,'BCI2003_Ia+b');
        % BCI competition 2003, dataset 1a+b (Tuebingen)
        data = load('-ascii',HDR.FileName);
        if strfind(HDR.FileName,'Testdata'),
                HDR.Classlabel = repmat(NaN,size(data,1),1);
        else
                HDR.Classlabel = data(:,1);
                data = data(:,2:end);
        end;
        
        HDR.NRec = length(HDR.Classlabel);
        HDR.FLAG.TRIGGERED = HDR.NRec>1; 
        HDR.PhysDim = 'µV';
        HDR.SampleRate = 256; 
        
        if strfind(HDR.FILE.Path,'a34lkt') 
                HDR.INFO='BCI competition 2003, dataset 1a (Tuebingen)';
                HDR.Dur = 3.5; 
                HDR.Label = {'A1-Cz';'A2-Cz';'C3f';'C3p';'C4f';'C4p'};
                HDR.TriggerOffset = -2; %[s]
        end;
        
        if strfind(HDR.FILE.Path,'egl2ln')
                HDR.INFO='BCI competition 2003, dataset 1b (Tuebingen)';
                HDR.Dur = 4.5; 
                HDR.Label = {'A1-Cz';'A2-Cz';'C3f';'C3p';'vEOG';'C4f';'C4p'};
                HDR.TriggerOffset = -2; %[s]
        end;
        HDR.SPR = HDR.SampleRate*HDR.Dur;
        HDR.NS  = length(HDR.Label);
        HDR.data = reshape(permute(reshape(data, [HDR.NRec, HDR.SPR, HDR.NS]),[2,1,3]),[HDR.SPR*HDR.NRec,HDR.NS]);
        if any(CHAN), 
                HDR.data = HDR.data(:,chan);
        end;
        HDR.TYPE = 'native'; 
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
        
        
elseif strcmp(HDR.TYPE,'BCI2003_III');
        % BCI competition 2003, dataset III (Graz)
        tmp = load(HDR.FileName);
        HDR.data = tmp*50;
        if strcmp(HDR.FILE.Name,'x_train'),
                tmp = fullfile(HDR.FILE.Path,'y_train');
                if exist(tmp,'file')
                        HDR.Classlabel = load(tmp);
                end;
        elseif strcmp(HDR.FILE.Name,'x_test'),
                HDR.Classlabel = repmat(NaN,140,1);        
        end;
                
        %elseif isfield(tmp,'x_train') & isfield(tmp,'y_train') & isfield(tmp,'x_test');	
        HDR.INFO  = 'BCI competition 2003, dataset 3 (Graz)'; 
        HDR.Label = {'C3a-C3p'; 'Cza-Czp'; 'C4a-C4p'};
        HDR.SampleRate = 128; 
        HDR.NRec = length(HDR.Classlabel);
        HDR.FLAG.TRIGGERED = 1; 
        HDR.Dur = 9; 
        HDR.NS  = 3;
        HDR.SPR = HDR.SampleRate*HDR.Dur;
        
        sz = [HDR.NS, HDR.SPR, HDR.NRec];
        HDR.data = reshape(permute(HDR.data,[2,1,3]),sz(1),sz(2)*sz(3))';
        HDR.TYPE = 'native'; 
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
                
                
elseif strncmp(HDR.TYPE,'MAT',3),
        status = warning;
        warning('off');
        tmp = load('-mat',HDR.FileName);
        warning(status);
        if isfield(tmp,'HDR')
                HDR = tmp.HDR; 
                if isfield(HDR,'data');
                        HDR.TYPE = 'native'; 
                end; 
                
        elseif isfield(tmp,'y'),		% Guger, Mueller, Scherer
                HDR.NS = size(tmp.y,2);
                HDR.NRec = 1; 
                if ~isfield(tmp,'SampleRate')
                        %fprintf(HDR.FILE.stderr,['Samplerate not known in ',HDR.FileName,'. 125Hz is chosen']);
                        HDR.SampleRate=125;
                else
                        HDR.SampleRate=tmp.SampleRate;
                end;
                fprintf(HDR.FILE.stderr,'Sensitivity not known in %s.\n',HDR.FileName);
                if any(CHAN),
                        HDR.data = tmp.y(:,CHAN);
                else
        	        HDR.data = tmp.y;
                end;
                HDR.TYPE = 'native'; 
                
                
        elseif [isfield(tmp,'cnt') | isfield(tmp,'X') ] & isfield(tmp,'nfo')
        	if isfield(tmp,'cnt') 
                        HDR.data = tmp.cnt;
                        [HDR.SPR,HDR.NS] = size(tmp.cnt);
			HDR.INFO='BCI competition 2005, dataset IV (Berlin)'; 
			HDR.Filter.LowPass = 0.05; 
			HDR.Filter.HighPass = 200; 
			HDR.Cal   = 0.1; 
			HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,.1);
		elseif isfield(tmp,'X'),
                        HDR.data = tmp.X;
			[HDR.SPR,HDR.NS] = size(tmp.X);
			HDR.INFO='BCI competition 2005, dataset V (IDIAP)'; 
			HDR.Filter.LowPass = 0; 
			HDR.Filter.HighPass = 256; 
			if isfield(tmp,'Y'),
				HDR.Classlabel = tmp.Y(:);
                        else
				HDR.Classlabel = repmat(NaN,size(tmp.X,1),1);
			end;	
			HDR.Cal   = 1; 
			HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
		else
		
		end;
                
		HDR.PhysDim = 'uV';
		HDR.SampleRate = tmp.nfo.fs; 
		%HDR.Dur = HDR.SPR/HDR.SampleRate;
		if isfield(tmp,'mrk')
			HDR.TRIG  = tmp.mrk.pos; 
			HDR.EVENT.POS = tmp.mrk.pos(:); 
			HDR.EVENT.TYP = zeros(size(HDR.EVENT.POS));
			HDR.EVENT.CHN = zeros(size(HDR.EVENT.POS));
                        if ~isempty(strfind(HDR.INFO,'Berlin')),cuelen=3.5; 
                        elseif ~isempty(strfind(HDR.INFO,'IDIAP')),cuelen=20; 
                        end;
			HDR.EVENT.DUR = repmat(cuelen*HDR.SampleRate,size(HDR.EVENT.POS));
			if isfield(tmp.mrk,'y'),
				HDR.Classlabel = tmp.mrk.y; 
			else	
				HDR.Classlabel = repmat(NaN,size(HDR.TRIG));
			end;
			if isfield(tmp.mrk,'className'),
				HDR.EVENT.TeegType = tmp.mrk.className;
                                HDR.EVENT.TYP(isnan(HDR.Classlabel)) = hex2dec('030f');  % unknown/undefined
				ix = strmatch('left',tmp.mrk.className); 
				if ~isempty(ix),
					HDR.EVENT.TYP(HDR.Classlabel==ix) = hex2dec('0301');  % left
				end;	
				ix = strmatch('right',tmp.mrk.className); 
				if ~isempty(ix),
					HDR.EVENT.TYP(HDR.Classlabel==ix) = hex2dec('0302');  % right
				end;	
				ix = strmatch('foot',tmp.mrk.className); 
				if ~isempty(ix),
					HDR.EVENT.TYP(HDR.Classlabel==ix) = hex2dec('0303');  % foot
				end;	
				ix = strmatch('tongue',tmp.mrk.className); 
				if ~isempty(ix),
					HDR.EVENT.TYP(HDR.Classlabel==ix) = hex2dec('0304');  % tongue
				end;	
			end;
		end;
		HDR.Label = tmp.nfo.clab';
		HDR.ELPOS = [tmp.nfo.xpos,tmp.nfo.ypos];
                HDR.NRec = 1; 
		HDR.FILE.POS = 0; 
                HDR.TYPE = 'native'; 
                clear tmp; 
		
                
        elseif isfield(tmp,'Signal') & isfield(tmp,'Flashing') & isfield(tmp,'StimulusCode')
                HDR.INFO = 'BCI competition 2005, dataset II (Albany)'; 
                HDR.SampleRate = 240; 
		HDR.Filter.LowPass   = 60;
		HDR.Filter.HighPass  = 0.1;
                [HDR.NRec,HDR.SPR,HDR.NS] = size(tmp.Signal); 
		HDR.BCI2000.Flashing = tmp.Flashing;
		HDR.BCI2000.StimulusCode = tmp.StimulusCode;
		if isfield(tmp,'TargetChar')
			HDR.BCI2000.TargetChar = tmp.TargetChar;
		end;	
		if isfield(tmp,'StimulusType')
			HDR.BCI2000.StimulusType = tmp.StimulusType;
		end;	

		HDR.FILE.POS = 0; 
                HDR.TYPE = 'native'; 
		HDR.data = reshape(tmp.Signal,[HDR.NRec*HDR.SPR, HDR.NS]);
		clear tmp;
		
                
        elseif isfield(tmp,'run') & isfield(tmp,'trial') & isfield(tmp,'sample') & isfield(tmp,'signal') & isfield(tmp,'TargetCode');
                HDR.INFO='BCI competition 2003, dataset 2a (Albany)'; 
                HDR.SampleRate = 160; 
                HDR.NRec = 1; 
		[HDR.SPR,HDR.NS]=size(tmp.signal);
                if CHAN>0,
                        HDR.data = tmp.signal(:,CHAN); 
                else
                        HDR.data = tmp.signal; 
                end
                HDR.EVENT.POS = [0;find(diff(tmp.trial)>0)-1];
                HDR.EVENT.TYP = ones(length(HDR.EVENT.POS),1)*hex2dec('0300'); % trial onset; 
                
                if 0,
                        EVENT.POS = [find(diff(tmp.trial)>0);length(tmp.trial)];
                        EVENT.TYP = ones(length(EVENT.POS),1)*hex2dec('8300'); % trial offset; 
                        HDR.EVENT.POS = [HDR.EVENT.POS; EVENT.POS];
                        HDR.EVENT.TYP = [HDR.EVENT.TYP; EVENT.TYP];
                        [HDR.EVENT.POS,ix]=sort(HDR.EVENT.POS);
                        HDR.EVENT.TYP = HDR.EVENT.TYP(ix);
                end;
                
                HDR.EVENT.N = length(HDR.EVENT.POS);
                ix = find((tmp.TargetCode(1:end-1)==0) & (tmp.TargetCode(2:end)>0));
                HDR.Classlabel = tmp.TargetCode(ix+1); 
                HDR.TYPE = 'native'; 

                
        elseif isfield(tmp,'runnr') & isfield(tmp,'trialnr') & isfield(tmp,'samplenr') & isfield(tmp,'signal') & isfield(tmp,'StimulusCode');
                HDR.INFO = 'BCI competition 2003, dataset 2b (Albany)'; 
                HDR.SampleRate = 240; 
                HDR.NRec = 1; 
		[HDR.SPR,HDR.NS]=size(tmp.signal);
                if CHAN>0,
                        HDR.data = tmp.signal(:,CHAN); 
                else
                        HDR.data = tmp.signal; 
                end
                HDR.EVENT.POS = [0;find(diff(tmp.trialnr)>0)-1];
                HDR.EVENT.TYP = ones(length(HDR.EVENT.POS),1)*hex2dec('0300'); % trial onset; 

                if 0,
                        EVENT.POS = [find(diff(tmp.trial)>0);length(tmp.trial)];
                        EVENT.TYP = ones(length(EVENT.POS),1)*hex2dec('8300'); % trial offset; 
                        HDR.EVENT.POS = [HDR.EVENT.POS; EVENT.POS];
                        HDR.EVENT.TYP = [HDR.EVENT.TYP; EVENT.TYP];
                        [HDR.EVENT.POS,ix]=sort(HDR.EVENT.POS);
                        HDR.EVENT.TYP = HDR.EVENT.TYP(ix);
                end;
                
                HDR.EVENT.N = length(HDR.EVENT.POS);
                ix = find((tmp.StimulusCode(1:end-1)==0) & (tmp.StimulusCode(2:end)>0));
                HDR.Classlabel = tmp.StimulusCode(ix+1); 
                HDR.TYPE = 'native'; 
                
                
        elseif isfield(tmp,'clab') & isfield(tmp,'x_train') & isfield(tmp,'y_train') & isfield(tmp,'x_test');	
                HDR.INFO  = 'BCI competition 2003, dataset 4 (Berlin)'; 
                HDR.Label = tmp.clab;        
                HDR.Classlabel = [repmat(nan,size(tmp.x_test,3),1);tmp.y_train';repmat(nan,size(tmp.x_test,3),1)];
                HDR.NRec  = length(HDR.Classlabel);
                
                HDR.SampleRate = 1000;
                HDR.Dur = 0.5; 
                HDR.NS  = size(tmp.x_test,2);
                HDR.SPR = HDR.SampleRate*HDR.Dur;
                HDR.FLAG.TRIGGERED = 1; 
                sz = [HDR.NS,HDR.SPR,HDR.NRec];
                
                HDR.data = reshape(permute(cat(3,tmp.x_test,tmp.x_train,tmp.x_test),[2,1,3]),sz(1),sz(2)*sz(3))';
                HDR.TYPE = 'native'; 
                
       elseif isfield(tmp,'x_train') & isfield(tmp,'y_train') & isfield(tmp,'x_test');	
                HDR.INFO  = 'BCI competition 2003, dataset 3 (Graz)'; 
                HDR.Label = {'C3a-C3p'; 'Cza-Czp'; 'C4a-C4p'};
                HDR.SampleRate = 128; 
                HDR.Classlabel = [tmp.y_train-1; repmat(nan,size(tmp.x_test,3),1)];
                HDR.data = cat(3, tmp.x_test, tmp.x_train)*50;
                
                HDR.NRec = length(HDR.Classlabel);
                HDR.FLAG.TRIGGERED = 1; 
                HDR.SampleRate = 128;
                HDR.Dur = 9; 
                HDR.NS  = 3;
                HDR.SPR = HDR.SampleRate*HDR.Dur;
                
                sz = [HDR.NS, HDR.SPR, HDR.NRec];
                HDR.data = reshape(permute(HDR.data,[2,1,3]),sz(1),sz(2)*sz(3))';
                HDR.TYPE = 'native'; 
                
        elseif isfield(tmp,'RAW_SIGNALS')    % TFM Matlab export 
                HDR.Label = fieldnames(tmp.RAW_SIGNALS);
                HDR.SampleRate = 1000; 
                HDR.TFM.SampleRate = 1000./[10,20,5,1,2];
                signal = [];
                for k1 = 4;1:length(HDR.Label);
                        s = getfield(tmp.RAW_SIGNALS,HDR.Label{k1});
                        ix = [];
                        for k2 = 1:length(s);
                                ix = [ix;length(s{k2})];   
                        end;
                        HDR.EVENT.POS(:,k1) = cumsum(ix);
                        HDR.data = cat(1,s{k1})';
                end;
                HDR.TYPE = 'native'; 

                
        elseif isfield(tmp,'eeg');	% Scherer
                fprintf(HDR.FILE.stderr,'Warning SLOAD: Sensitivity not known in %s,\n',HDR.FileName);
                HDR.NS=size(tmp.eeg,2);
                HDR.NRec = 1; 
                if ~isfield(tmp,'SampleRate')
                        %fprintf(HDR.FILE.stderr,['Samplerate not known in ',HDR.FileName,'. 125Hz is chosen']);
                        HDR.SampleRate=125;
                else
                        HDR.SampleRate=tmp.SampleRate;
                end;
                if any(CHAN),
                        HDR.data = tmp.eeg(:,CHAN);
                else
        	        HDR.data = tmp.eeg;
                end;
                if isfield(tmp,'classlabel'),
                	HDR.Classlabel = tmp.classlabel;
                end;        
                HDR.TYPE = 'native'; 

                
        elseif isfield(tmp,'data');
                if isfield(tmp,'readme') & iscell(tmp.data) ;	%Zachary A. Keirn, Purdue University, 1988. 
                        HDR.Label = {'C3'; 'C4'; 'P3'; 'P4'; 'O1'; 'O2'; 'EOG'};                               
                        HDR.SampleRate = 250; 
                        HDR.FLAG.TRIGGERED  = 1; 
                        HDR.DUR = 10; 
                        HDR.SPR = 2500;
                        HDR.FILTER.LowPass  = 0.1;
                        HDR.FILTER.HighPass = 100; 
                        HDR.NRec = length(tmp.data);
                        
                        x = cat(1,tmp.data{:});
                        [b,i,CL] = unique({x{:,1}}');
                        [HDR.EVENT.TeegDesc,i,CL(:,2)] = unique({x{:,2}}');
                        HDR.Classlabel = CL; 
                        HDR.data = [x{:,4}]';
                        HDR.NS   = size(HDR.data,2); 
                        HDR.Calib= sparse(2:8,1:7,1);
                        HDR.TYPE = 'native'; 
                        
                else        	% Mueller, Scherer ? 
                        HDR.NS = size(tmp.data,2);
                        HDR.NRec = 1; 
                        fprintf(HDR.FILE.stderr,'Warning SLOAD: Sensitivity not known in %s,\n',HDR.FileName);
                        if ~isfield(tmp,'SampleRate')
                                fprintf(HDR.FILE.stderr,'Warning SLOAD: Samplerate not known in %s. 125Hz is chosen\n',HDR.FileName);
                                HDR.SampleRate=125;
                        else
                                HDR.SampleRate=tmp.SampleRate;
                        end;
                        if any(CHAN),
                                HDR.data = tmp.data(:,CHAN);
                        else
                                HDR.data = tmp.data;
                        end;
                        if isfield(tmp,'classlabel'),
                                HDR.Classlabel = tmp.classlabel;
                        end;        
                        if isfield(tmp,'artifact'),
                                HDR.ArtifactSelection = zeros(size(tmp.classlabel));
                                HDR.ArtifactSelection(tmp.artifact)=1;
                        end;        
                        HDR.TYPE = 'native'; 
                end;
                
                
        elseif isfield(tmp,'EEGdata');  % Telemonitoring Daten (Reinhold Scherer)
                HDR.NS = size(tmp.EEGdata,2);
                HDR.NRec = 1; 
                HDR.Classlabel = tmp.classlabel;
                if ~isfield(tmp,'SampleRate')
                        fprintf(HDR.FILE.stderr,'Warning SLOAD: Samplerate not known in %s. 125Hz is chosen\n',HDR.FileName);
                        HDR.SampleRate=125;
                else
                        HDR.SampleRate=tmp.SampleRate;
                end;
                HDR.PhysDim = 'µV';
                fprintf(HDR.FILE.stderr,'Sensitivity not known in %s. 50µV is chosen\n',HDR.FileName);
                if any(CHAN),
                        HDR.data = tmp.EEGdata(:,CHAN)*50;
                else
                        HDR.data = tmp.EEGdata*50;
                end;
                HDR.TYPE = 'native'; 
                
        elseif isfield(tmp,'daten');	% EP Daten von Michael Woertz
                HDR.NS = size(tmp.daten.raw,2)-1;
                HDR.NRec = 1; 
                if ~isfield(tmp,'SampleRate')
                        fprintf(HDR.FILE.stderr,'Warning SLOAD: Samplerate not known in %s. 2000Hz is chosen\n',HDR.FileName);
                        HDR.SampleRate=2000;
                else
                        HDR.SampleRate=tmp.SampleRate;
                end;
                HDR.PhysDim = 'µV';
                fprintf(HDR.FILE.stderr,'Sensitivity not known in %s. 100µV is chosen\n',HDR.FileName);
                %signal=tmp.daten.raw(:,1:HDR.NS)*100;
                if any(CHAN),
                        HDR.data = tmp.daten.raw(:,CHAN)*100;
                else
                        HDR.data = tmp.daten.raw*100;
                end;
                HDR.TYPE = 'native'; 
                
        elseif isfield(tmp,'neun') & isfield(tmp,'zehn') & isfield(tmp,'trig');	% guger, 
                HDR.NS=3;
                HDR.NRec = 1; 
                if ~isfield(tmp,'SampleRate')
                        fprintf(HDR.FILE.stderr,'Warning SLOAD: Samplerate not known in %s. 125Hz is chosen\n',HDR.FileName);
                        HDR.SampleRate=125;
                else
                        HDR.SampleRate=tmp.SampleRate;
                end;
                fprintf(HDR.FILE.stderr,'Sensitivity not known in %s. \n',HDR.FileName);
                signal  = [tmp.neun;tmp.zehn;tmp.trig];
                HDR.Label = {'Neun','Zehn','TRIG'};
                if any(CHAN),
                        HDR.data=signal(:,CHAN);
                end;        
                HDR.TYPE = 'native'; 
                
                
        elseif isfield(tmp,'Recorder1')    % Nicolet NRF format converted into Matlab 
                for k = 1:length(s.Recorder1.Channels.ChannelInfos);
                        HDR.Label{k} = s.Recorder1.Channels.ChannelInfos(k).ChannelInfo.Name;
                        HDR.PhysDim{k} = s.Recorder1.Channels.ChannelInfos(k).ChannelInfo.YUnits;
                end;
                signal = [];
                T = [];
                for k = 1:length(s.Recorder1.Channels.Segments)
                        tmp = s.Recorder1.Channels.Segments(k).Data;
                        sz = size(tmp.Samples);
                        signal = [signal; repmat(nan,100,sz(1)); tmp.Samples'];
                        T = [T;repmat(nan,100,1);tmp.dX0+(1:sz(2))'*tmp.dXstep ]
                        fs = 1./tmp.dXstep;
                        if k==1,
                                HDR.SampleRate = fs;
                        elseif HDR.SampleRate ~= fs; 
                                fprintf(2,'Error SLOAD (NRF): different Sampling rates not supported, yet.\n');
                        end;
                end;
                HDR.data = signal; 
                HDR.TYPE = 'native'; 

                
        elseif isfield(tmp,'ECoGdata') & isfield(tmp,'dataset')  %Michigan ECoG dataset 
                HDR.data = tmp.ECoGdata';
                HDR.T0 = datevec(datenum(tmp.dataset.filetype.timestamp));
                HDR.SampleRate = tmp.dataset.specs.sample_rate;
                HDR.Filter.HighPass = tmp.dataset.specs.filters.lowcut;
                HDR.Filter.LowPass = tmp.dataset.specs.filters.highcut;
                if isfield(tmp.dataset.specs.filters,'notch60');
                        HDR.FILTER.Notch = tmp.dataset.specs.filters.notch60*60;
                end;
                HDR.Patient.Sex = tmp.dataset.subject_info.gender; 
                HDR.Patient.Age = tmp.dataset.subject_info.age; 
                HDR.Label = tmp.dataset.electrode.names;
                HDR.NS    = tmp.dataset.electrode.number;

                trigchancode = getfield(tmp.dataset.electrode.options,'TRIGGER');
                HDR.AS.TRIGCHAN = find(tmp.dataset.electrode.region==trigchancode);
                HDR.TRIG  = tmp.dataset.trigger.trigs_all;
                
                HDR.FLAG.TRIGGERED = 0;
                HDR.NRec  = 1; 
                HDR.SPR = size(HDR.data,1);
                HDR.Dur = HDR.SPR/HDR.SampleRate;
                HDR.TYPE  = 'native'; 
                HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
                clear tmp; 
                
                
        elseif isfield(tmp,'P_C_S');	% G.Tec Ver 1.02, 1.5x data format
                HDR.FILE.POS = 0; 
                if isa(tmp.P_C_S,'data'), %isfield(tmp.P_C_S,'version'); % without BS.analyze	
                        if any(tmp.P_C_S.Version==[1.02, 1.5, 1.52]),
                        else
                                fprintf(HDR.FILE.stderr,'Warning: PCS-Version is %4.2f.\n',tmp.P_C_S.Version);
                        end;
                        HDR.Filter.LowPass  = tmp.P_C_S.LowPass;
                        HDR.Filter.HighPass = tmp.P_C_S.HighPass;
                        HDR.Filter.Notch    = tmp.P_C_S.Notch;
                        HDR.SampleRate      = tmp.P_C_S.SamplingFrequency;
                        HDR.gBS.Attribute   = tmp.P_C_S.Attribute;
                        HDR.gBS.AttributeName = tmp.P_C_S.AttributeName;
                        HDR.Label = tmp.P_C_S.ChannelName;
                        HDR.gBS.EpochingSelect = tmp.P_C_S.EpochingSelect;
                        HDR.gBS.EpochingName = tmp.P_C_S.EpochingName;

                        HDR.data = double(tmp.P_C_S.Data);
                        
                else %if isfield(tmp.P_C_S,'Version'),	% with BS.analyze software, ML6.5
                        if any(tmp.P_C_S.version==[1.02, 1.5, 1.52]),
                        else
                                fprintf(HDR.FILE.stderr,'Warning: PCS-Version is %4.2f.\n',tmp.P_C_S.version);
                        end;        
                        HDR.Filter.LowPass  = tmp.P_C_S.lowpass;
                        HDR.Filter.HighPass = tmp.P_C_S.highpass;
                        HDR.Filter.Notch    = tmp.P_C_S.notch;
                        HDR.SampleRate      = tmp.P_C_S.samplingfrequency;
                        HDR.gBS.Attribute   = tmp.P_C_S.attribute;
                        HDR.gBS.AttributeName = tmp.P_C_S.attributename;
                        HDR.Label = tmp.P_C_S.channelname;
                        HDR.gBS.EpochingSelect = tmp.P_C_S.epochingselect;
                        HDR.gBS.EpochingName = tmp.P_C_S.epochingname;
                        
                        HDR.data = double(tmp.P_C_S.data);
                end;
                tmp = []; % clear memory

                sz     = size(HDR.data);
                HDR.NRec = sz(1);
                HDR.SPR  = sz(2);
                HDR.Dur  = sz(2)/HDR.SampleRate;
                HDR.NS   = sz(3);
                HDR.FLAG.TRIGGERED = HDR.NRec>1;
                
                if any(CHAN),
                        sz(3)= length(CHAN);
                else
                        CHAN = 1:HDR.NS;
                end;
                HDR.data  = reshape(permute(HDR.data(:,:,CHAN),[2,1,3]),[sz(1)*sz(2),sz(3)]);

                % Selection of trials with artifacts
                ch = strmatch('ARTIFACT',HDR.gBS.AttributeName);
                if ~isempty(ch)
                        HDR.ArtifactSelection = HDR.gBS.Attribute(ch,:);
                end;
                
                % Convert gBS-epochings into BIOSIG - Events
                map = zeros(size(HDR.gBS.EpochingName,1),1);
                map(strmatch('AUGE',HDR.gBS.EpochingName))=hex2dec('0101');
                map(strmatch('EOG',HDR.gBS.EpochingName))=hex2dec('0101');
                map(strmatch('MUSKEL',HDR.gBS.EpochingName))=hex2dec('0103');
                map(strmatch('MUSCLE',HDR.gBS.EpochingName))=hex2dec('0103');
                map(strmatch('ELECTRODE',HDR.gBS.EpochingName))=hex2dec('0105');

                map(strmatch('SLEEPSTAGE1',HDR.gBS.EpochingName))=hex2dec('0411');
                map(strmatch('SLEEPSTAGE2',HDR.gBS.EpochingName))=hex2dec('0412');
                map(strmatch('SLEEPSTAGE3',HDR.gBS.EpochingName))=hex2dec('0413');
                map(strmatch('SLEEPSTAGE4',HDR.gBS.EpochingName))=hex2dec('0414');
                map(strmatch('REM',HDR.gBS.EpochingName))=hex2dec('0415');

                if ~isempty(HDR.gBS.EpochingSelect),
                        HDR.EVENT.TYP = map([HDR.gBS.EpochingSelect{:,9}]');
                        HDR.EVENT.POS = [HDR.gBS.EpochingSelect{:,1}]';
                        HDR.EVENT.CHN = [HDR.gBS.EpochingSelect{:,3}]';
                        HDR.EVENT.DUR = [HDR.gBS.EpochingSelect{:,4}]';
                end;
                HDR.TYPE = 'native'; 
                
	elseif isfield(tmp,'P_C_DAQ_S');
                if ~isempty(tmp.P_C_DAQ_S.data),
                        HDR.data = double(tmp.P_C_DAQ_S.data{1});
                        
                elseif ~isempty(tmp.P_C_DAQ_S.daqboard),
                        [tmppfad,file,ext] = fileparts(tmp.P_C_DAQ_S.daqboard{1}.ObjInfo.LogFileName),
                        file = [file,ext];
                        if exist(file,'file')
                                HDR.data=daqread(file);        
                                HDR.info=daqread(file,'info');        
                        else
                                fprintf(HDR.FILE.stderr,'Error SLOAD: no data file found\n');
                                return;
                        end;
                        
                else
                        fprintf(HDR.FILE.stderr,'Error SLOAD: no data file found\n');
                        return;
                end;
                
                HDR.NS = size(HDR.data,2);
                HDR.Cal = tmp.P_C_DAQ_S.sens*(2.^(1-tmp.P_C_DAQ_S.daqboard{1}.HwInfo.Bits));
                HDR.Calib = sparse(2:HDR.NS,1:HDR.NS,HDR.Cal);
                
                if all(tmp.P_C_DAQ_S.unit==1)
                        HDR.PhysDim='uV';
                else
                        HDR.PhysDim='[?]';
                end;
                
                HDR.SampleRate = tmp.P_C_DAQ_S.samplingfrequency;
                sz     = size(HDR.data);
                if length(sz)==2, sz=[1,sz]; end;
                HDR.NRec = sz(1);
                HDR.Dur  = sz(2)/HDR.SampleRate;
                HDR.NS   = sz(3);
                HDR.FLAG.TRIGGERED = HDR.NRec>1;
                HDR.Filter.LowPass = tmp.P_C_DAQ_S.lowpass;
                HDR.Filter.HighPass = tmp.P_C_DAQ_S.highpass;
                HDR.Filter.Notch = tmp.P_C_DAQ_S.notch;
                HDR.TYPE = 'native'; 
                
                
        elseif isfield(tmp,'eventmatrix') & isfield(tmp,'samplerate') 
                %%% F. Einspieler's Event information 
                HDR.EVENT.POS = tmp.eventmatrix(:,1);
                HDR.EVENT.TYP = tmp.eventmatrix(:,2);
                HDR.EVENT.CHN = tmp.eventmatrix(:,3);
                HDR.EVENT.DUR = tmp.eventmatrix(:,4);
                HDR.SampleRate = tmp.samplerate;
                HDR.TYPE = 'EVENT';
                
        else 
                HDR.Calib = 1; 
                CHAN = 1; 
        end;
        if strcmp(HDR.TYPE,'native'),
                if ~isfield(HDR,'NS');
                        HDR.NS = size(HDR.data,2);
                end;
                if ~isfield(HDR,'SPR');
                        HDR.SPR = size(HDR.data,1);
                end;
                if ~isfield(HDR,'Calib');
                        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
                end;
                if ~isfield(HDR.FILE,'POS');
                        HDR.FILE.POS = 0;
                end;
        end;

        
elseif strcmp(HDR.TYPE,'BCI2000'),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
                
                HDR.Header = fread(HDR.FILE.FID,HDR.HeadLen,'char');
		[tline,rr] = strtok(char(HDR.Header'),[10,13]);
		STATUSFLAG = 0;
		while length(rr), 
			tline = tline(1:min([length(tline),strfind(tline,[47,47])-1]));

			if ~isempty(strfind(tline,'[ State Vector Definition ]'))
				STATUSFLAG = 1;
				STATECOUNT = 0; 

			elseif ~isempty(strfind(tline,'[ Parameter Definition ]'))
				STATUSFLAG = 2;

			elseif strncmp(tline,'[',1)
				STATUSFLAG = 3;
			
			elseif STATUSFLAG==1, 
				[t,r] = strtok(tline);
				val = str2double(r);
				%HDR.BCI2000 = setfield(HDR.BCI2000,t,val);
				STATECOUNT = STATECOUNT + 1; 
				HDR.BCI2000.StateVector(STATECOUNT,:) = val; 
				HDR.BCI2000.StateDef{STATECOUNT,1} = t; 
		    
			elseif STATUSFLAG==2, 
				[tag,r] = strtok(tline,'=');
				[val,r] = strtok(r,'=');
				if ~isempty(strfind(tag,'SamplingRate'))
					[tmp,status] = str2double(val);
					HDR.SampleRate = tmp(1);
				elseif ~isempty(strfind(tag,'SourceChGain'))
					[tmp,status] = str2double(val);
					HDR.Cal = tmp(2:tmp(1)+1);
				elseif ~isempty(strfind(tag,'SourceChOffset'))
					[tmp,status] = str2double(val);
					HDR.Off = tmp(2:tmp(1)+1);
				elseif ~isempty(strfind(tag,'SourceMin'))
					[tmp,status] = str2double(val);
					HDR.DigMin = tmp(1);
				elseif ~isempty(strfind(tag,'SourceMax'))
					[tmp,status] = str2double(val);
					HDR.DigMax = tmp(1);
				end;
			end;	
			[tline,rr] = strtok(rr,[10,13]);
		end;
                HDR.PhysDim = 'µV';
                HDR.Calib = [HDR.Off(1)*ones(1,HDR.NS);eye(HDR.NS)]*HDR.Cal(1);
                
		% decode State Vector Definition 
		X = repmat(NaN,1,HDR.BCI2000.StateVectorLength*8);
		for k = 1:STATECOUNT,
			for k1 = 1:HDR.BCI2000.StateVector(k,1),
				X(HDR.BCI2000.StateVector(k,3:4)*[8;1]+k1) = k;
			end;		
		end;
		HDR.BCI2000.X = X;
		
		% convert EVENT information
		status = fseek(HDR.FILE.FID,HDR.HeadLen+2*HDR.NS,'bof');
		tmp = fread(HDR.FILE.FID,[HDR.BCI2000.StateVectorLength,inf],[int2str(HDR.BCI2000.StateVectorLength),'*uchar'],HDR.NS*2)';
		ix = find(any(diff(tmp,[],1),2));
		HDR.BCI2000.STATUS = tmp([ix;length(tmp)],:);
		tmp = HDR.BCI2000.STATUS';
		HDR.BCI2000.BINARYSTATUS = reshape(dec2bin(tmp(:),8)',8*HDR.BCI2000.StateVectorLength,size(HDR.BCI2000.STATUS,1))';
		HDR.EVENT.POS = [0; ix+1];
		HDR.EVENT.CHN = zeros(size(HDR.EVENT.POS));
		HDR.EVENT.DUR = diff([0;ix+1;size(tmp,1)]);
		HDR.EVENT.TYP = repmat(NaN,size(HDR.EVENT.POS)); 	% should be extracted from BCI2000.BINARYSTATUS
		fprintf(2,'Warning SOPEN (BCI2000): HDR.EVENT.TYP information need to be extracted from HDR.BCI2000.BINARYSTATUS\n');

		% finalize header definition 		
		status = fseek(HDR.FILE.FID,HDR.HeadLen,'bof');
		HDR.AS.bpb = 2*HDR.NS + HDR.BCI2000.StateVectorLength;
		HDR.SPR = (HDR.FILE.size - HDR.HeadLen)/HDR.AS.bpb;
		HDR.AS.endpos = HDR.SPR;
		HDR.GDFTYP = [int2str(HDR.NS),'*int16=>int16'];
		HDR.NRec = 1; 
		
                HDR.FILE.OPEN = 1;
		HDR.FILE.POS = 0; 
        end;

        
elseif strcmp(HDR.TYPE,'CFWB'),		% Chart For Windows Binary data, defined by ADInstruments. 
        CHANNEL_TITLE_LEN = 32;
        UNITS_LEN = 32;
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
                
                fprintf(HDR.FILE.stderr,'Format not tested yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',PERMISSION);	
                
                HDR.FILE.OPEN = 1;
                fseek(HDR.FILE.FID,4,'bof');
                HDR.VERSION = fread(HDR.FILE.FID,1,'int32');
                HDR.Dur = fread(HDR.FILE.FID,1,'double');
                HDR.SampleRate = 1/HDR.Dur;
                HDR.T0 = fread(HDR.FILE.FID,5,'int32');
                tmp = fread(HDR.FILE.FID,2,'double');
                HDR.T0(6) = tmp(1);
                HDR.CFWB.preTrigger = tmp(2);
                HDR.NS = fread(HDR.FILE.FID,1,'int32');
                HDR.SPR = fread(HDR.FILE.FID,1,'int32');
                HDR.NRec = 1;
                HDR.FLAG.TRIGGERED = 0;	        
                
                HDR.FLAG.TimeChannel = fread(HDR.FILE.FID,1,'int32');
                tmp = fread(HDR.FILE.FID,1,'int32');
                if tmp == 1, 
                        HDR.GDFTYP = 'float64';
                        HDR.AS.bpb = HDR.NS * 8;
                elseif tmp == 2, 
                        HDR.GDFTYP = 'float32';
                        HDR.AS.bpb = HDR.NS * 4;
                elseif tmp == 3, 
                        HDR.GDFTYP = 'int16';
                        HDR.AS.bpb = HDR.NS * 2;
                end;
                for k = 1:HDR.NS,
                        HDR.Label(k,:) = fread(HDR.FILE.FID,[1, CHANNEL_TITLE_LEN],'char');   
                        HDR.PhysDim(k,:) = fread(HDR.FILE.FID,[1, UNITS_LEN],'char');   
                        HDR.Cal(k,1) = fread(HDR.FILE.FID,1,'double');   
                        HDR.Off(k,1) = fread(HDR.FILE.FID,1,'double');   
                        HDR.PhysMax(1,k) = fread(HDR.FILE.FID,1,'double');   
                        HDR.PhysMin(1,k) = fread(HDR.FILE.FID,1,'double');   
                end;

                
        elseif any(PERMISSION=='w'),
                HDR.VERSION   = 1;
                if ~isfield(HDR,'NS'),
                        HDR.NS = 0; 	% unknown channel number ...
                        fprintf(HDR.FILE.stderr,'Error SOPEN-W CFWB: number of channels HDR.NS undefined.\n');
                        return;
                end;
                if ~isfield(HDR,'SPR'),
                        HDR.SPR = 0; 	% Unknown - Value will be fixed when file is closed. 
                else
                        HDR.SPR = HDR.SPR(1);
                end;
                if ~isfield(HDR,'SampleRate'),
                        HDR.SampleRate = 1; 	% Unknown - Value will be fixed when file is closed. 
                        fprintf(HDR.FILE.stderr,'Warning SOPEN-W CFWB: samplerate undefined.\n');
                end;
                if any([HDR.SPR==0]), 	% if any unknown, ...				HDR.FILE.OPEN = 3;			%	... fix header when file is closed. 
                end;
                if ~isfield(HDR,'CFWB'),
                        HDR.CFWB.preTrigger = 0; 	% Unknown - Value will be fixed when file is closed. 
                end;
                if ~isfield(HDR.CFWB,'preTrigger'),
                        HDR.CFWB.preTrigger = 0; 	% Unknown - Value will be fixed when file is closed. 
                end;
                if ~isfield(HDR,'FLAG'),
                        HDR.FLAG.TimeChannel = 0;
                else
                        if ~isfield(HDR.FLAG,'TimeChannel'),
                                HDR.Flag.TimeChannel = 0;
                        end;
                end;
                if strcmp(gdfdatatype(HDR.GDFTYP),'float64');
                        tmp = 1;
                        HDR.AS.bpb = HDR.NS * 8;
                        HDR.Cal = ones(HDR.NS,1);
                        HDR.Off = zeros(HDR.NS,1);
                elseif strcmp(gdfdatatype(HDR.GDFTYP),'float32');
                        tmp = 2; 
                        HDR.AS.bpb = HDR.NS * 4;
                        HDR.Cal = ones(HDR.NS,1);
                        HDR.Off = zeros(HDR.NS,1);
                elseif strcmp(gdfdatatype(HDR.GDFTYP),'int16');
                        tmp = 3;
                        HDR.AS.bpb = HDR.NS * 2;
                end;
                HDR.PhysMax = repmat(NaN,HDR.NS,1);
                HDR.PhysMin = repmat(NaN,HDR.NS,1);
                if ~isfield(HDR,'Cal'),
                        fprintf(HDR.FILE.stderr,'Warning SOPEN-W CFWB: undefined scaling factor\n');			
                        HDR.Cal = ones(HDR.NS,1);
                end;
                if ~isfield(HDR,'Off'),
                        fprintf(HDR.FILE.stderr,'Warning SOPEN-W CFWB: undefined offset\n');			
                        HDR.Off = zeros(HDR.NS,1);
                end;
                if ~isfield(HDR,'Label'),
                        for k = 1:HDR.NS,
                                tmp = sprintf('channel %i',k);
                                HDR.Label(k,:) = [tmp,setstr(repmat(32,1,max(0,CHANNEL_TITLE_LEN-length(tmp))))];
                        end;
                elseif iscell(HDR.Label)
                        Label = [];
                        for k = 1:HDR.NS,
                                tmp = [HDR.Label{k}, setstr(reshape(32,1,max(0,CHANNEL_TITLE_LEN-size(HDR.Label, 2))))];
                                Label(k,:) = tmp(1:CHANNEL_TITLE_LEN);
                        end;
                        HDR.Label = Label;
                else
                        HDR.Label = [HDR.Label,setstr(repmat(32,size(HDR.Label,1),max(0,CHANNEL_TITLE_LEN-size(HDR.Label,2))))];
                        HDR.Label = [HDR.Label;setstr(repmat(32,max(0,HDR.NS-size(HDR.Label,1)),size(HDR.Label,2)))];
                end;
                
                if ~isfield(HDR,'PhysDim'),
                        HDR.PhysDim = setstr(repmat(32,HDR.NS,UNITS_LEN));
                end;
                
                if size(HDR.PhysDim,1)==1,
                        HDR.PhysDim = HDR.PhysDim(ones(HDR.NS,1),:);
                end;		
                if iscell(HDR.PhysDim)
                        Label = [];
                        for k = 1:size(HDR.PhysDim,1),
                                tmp = [HDR.PhysDim{k}, setstr(reshape(32,1,max(0,UNITS_LEN-size(HDR.PhysDim, 2))))];
                                Label(k,:) = tmp(1:UNITS_LEN);
                        end;
                        HDR.PhysDim = setstr(Label);
                else
                        HDR.PhysDim = [HDR.PhysDim, setstr(repmat(32,size(HDR.PhysDim,1),max(0,UNITS_LEN-size(HDR.PhysDim,2))))];
                        HDR.PhysDim = [HDR.PhysDim; setstr(repmat(32,max(0,HDR.NS-size(HDR.PhysDim,1)),size(HDR.PhysDim,2)))];
                end;
                
                
                %%%%% write fixed header
                HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
                if HDR.FILE.FID<0, 
                        fprintf(HDR.FILE.stderr,'Error SOPEN-W CFWB: could not open file %s .\n',HDR.FileName);
                        return;
                else
                        HDR.FILE.OPEN = 2;		
                end;
                fwrite(HDR.FILE.FID,'CFWB','char');
                fwrite(HDR.FILE.FID,HDR.VERSION,'int32');
                fwrite(HDR.FILE.FID,1/HDR.SampleRate(1),'double');
                fwrite(HDR.FILE.FID,HDR.T0(1:5),'int32');
                fwrite(HDR.FILE.FID,HDR.T0(6),'double');
                fwrite(HDR.FILE.FID,HDR.preTrigger,'double');
                fwrite(HDR.FILE.FID,[HDR.NS,HDR.SPR,HDR.Flag.TimeChannel],'int32');
                fwrite(HDR.FILE.FID,tmp,'int32');
                HDR.HeadLen = ftell(HDR.FILE.FID);
                if (HDR.HeadLen~=68),
                        fprintf(HDR.FILE.stderr,'Error SOPEN CFWB: size of header1 does not fit in file %s\n',HDR.FileName);
                end;
                
                %%%%% write channel header
                for k = 1:HDR.NS,
                        fwrite(HDR.FILE.FID,HDR.Label(k,1:32),'char');
                        fwrite(HDR.FILE.FID,setstr(HDR.PhysDim(k,1:32)),'char');
                        fwrite(HDR.FILE.FID,[HDR.Cal(k),HDR.Off(k)],'double');
                        fwrite(HDR.FILE.FID,[HDR.PhysMax(k),HDR.PhysMin(k)],'double');
                end;
                %HDR.HeadLen = (68+HDR.NS*96); %
                HDR.HeadLen = ftell(HDR.FILE.FID);
                if (HDR.HeadLen~=(68+HDR.NS*96))
                        fprintf(HDR.FILE.stderr,'Error SOPEN CFWB: size of header2 does not fit in file %s\n',HDR.FileName);
                end;
        end;
        HDR.Calib = [HDR.Off';speye(HDR.NS)]*spdiags(1:HDR.NS,1:HDR.NS,HDR.Cal);
        HDR.Label = setstr(HDR.Label);
        HDR.PhysDim = setstr(HDR.PhysDim);
        
        HDR.HeadLen = ftell(HDR.FILE.FID);
        HDR.FILE.POS = 0; 
        HDR.AS.endpos = HDR.SPR; 
        
        
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
                
                HDR.Cal = eye(AmplitudeResolution(HDR.InChanSelect))/1000;
                HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal,HDR.NS+1,HDR.NS);
                HDR.PhysDim = 'uV';
                HDR.AS.bpb = 2*HDR.NS;
                HDR.GDFTYP = 'int16';
                HDR.AS.endpos = 8+2+512+HDR.variable_length_block+HDR.NS*2*HDR.SPR;
                HDR.FLAG.TRIGGERED = 0;	% Trigger Flag
                
        else
                fprintf(HDR.FILE.stderr,'PERMISSION %s not supported\n',PERMISSION);	
        end;			
        
        
elseif strcmp(HDR.TYPE,'NEX'),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
                if HDR.FILE.FID<0,
                        return;
                end
                
                %HDR.FILE.OPEN = 1;
                HDR.FILE.POS  = 0;
                HDR.NEX.magic = fread(HDR.FILE.FID,1,'int32');
                HDR.VERSION = fread(HDR.FILE.FID,1,'int32');
                HDR.NEX.comment = char(fread(HDR.FILE.FID,[1,256],'char'));
                HDR.SampleRate = fread(HDR.FILE.FID, 1, 'double');
                HDR.NEX.begintime = fread(HDR.FILE.FID, 1, 'int32');
                HDR.NEX.endtime = fread(HDR.FILE.FID, 1, 'int32');
                HDR.NS = fread(HDR.FILE.FID, 1, 'int32');
                fseek(HDR.FILE.FID, 260, 'cof');

                for k = 1:HDR.NS,
                        HDR.NEX.pos0(k) = ftell(HDR.FILE.FID);
                        HDR.NEX.type(k) = fread(HDR.FILE.FID, 1, 'int32');
                        HDR.NEX.version(k) = fread(HDR.FILE.FID, 1, 'int32');
                        Label(k,:) = fread(HDR.FILE.FID, [1 64], 'char');
                        HDR.NEX.offset(k)  = fread(HDR.FILE.FID, 1, 'int32');
                        HDR.NEX.nf(k)  = fread(HDR.FILE.FID, 1, 'int32');
                        reserved(k,:) = char(fread(HDR.FILE.FID, [1 32], 'char'));
                        HDR.NEX.adfreq(k) = fread(HDR.FILE.FID, 1, 'double');
                        HDR.Cal(k) = fread(HDR.FILE.FID, 1, 'double');
                        HDR.NEX.n(k) = fread(HDR.FILE.FID, 1, 'int32');
                        nm = fread(HDR.FILE.FID, 1, 'int32');
                        nl = fread(HDR.FILE.FID, 1, 'int32');

                        HDR.NEX.pos(k) = ftell(HDR.FILE.FID);
                        if HDR.NEX.type(k)==2,  % interval
                                fseek(HDR.FILE.FID, HDR.NEX.offset(k), 'bof');
                                tmp = fread(HDR.FILE.FID, [HDR.NEX.nf(k),2], 'int32');
                                HDR.EVENT.POS = tmp(:,1);
                                HDR.EVENT.DUR = tmp(:,2) - tmp(:,1);
                                HDR.EVENT.N = HDR.NEX.nf(k);
                                
                        elseif HDR.NEX.type(k)==3,  % waveform 
                                HDR.HeadLen = HDR.NEX.offset(k);
                                num = k; 
                                fseek(HDR.FILE.FID, HDR.NEX.offset(k), 'bof');
                                HDR.NEX6.t = fread(HDR.FILE.FID, [1, HDR.NEX.nf(num)], 'int32')/HDR.SampleRate;
                                HDR.NEX6.data = fread(HDR.FILE.FID, [HDR.NEX.n(num), HDR.NEX.nf(num)], 'int16');

                        elseif HDR.NEX.type(k)==5, % continous variable  
                                fseek(HDR.FILE.FID, HDR.NEX.offset(k), 'bof');
                                HDR.NEX5.ts = fread(HDR.FILE.FID, [HDR.NEX.nf(k), 2], 'int32');
                                HDR.NEX5.data = fread(HDR.FILE.FID, [HDR.NEX.nf(k), 1], 'int16');
                                
                        elseif HDR.NEX.type(k)==6,  % marker
                                fseek(HDR.FILE.FID, HDR.NEX.offset(k), 'bof');
                                ts = fread(HDR.FILE.FID, [1,HDR.NEX.nf(k)], 'int32');
                                names = zeros(1,64);
                                m = zeros(HDR.NEX.n(k), nl, nm);
                                for j=1:nm
                                        names(j, :) = fread(HDR.FILE.FID, [1 64], 'char');
                                        for p = 1:HDR.NEX.n(k)
                                                m(p, :, j) = fread(HDR.FILE.FID, [1 nl], 'char');
                                        end
                                end
                                HDR.NEX.names = names;
                                HDR.NEX.m = m;
                        end;
                        fseek(HDR.FILE.FID, HDR.NEX.pos0(k)+208,'bof');
                end
                HDR.Label = char(Label);
                fseek(HDR.FILE.FID, HDR.HeadLen, 'bof');

                HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal);
                HDR.PhysDim = 'mV';

                fclose(HDR.FILE.FID);
                HDR.FILE.FID = -1;
                fprintf(HDR.FILE.stderr,'Format %s not tested yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',HDR.TYPE);	
        end;			
        
        
elseif strcmp(HDR.TYPE,'Nicolet'),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
                if HDR.FILE.FID<0,
                        return;
                end
                
                HDR.FILE.POS  = 0;
                HDR.FILE.OPEN = 1; 
                HDR.AS.endpos = HDR.SPR;
                HDR.AS.bpb = 2*HDR.NS;
                HDR.GDFTYP = 'int16';
                HDR.HeadLen = 0; 
        else
                fprintf(HDR.FILE.stderr,'PERMISSION %s not supported\n',PERMISSION);	
        end;			
        
        
elseif strncmp(HDR.TYPE,'SEG2',4),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb',HDR.Endianity);
                
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
                HDR.FILE.OPEN = 0;
        end;		
        
        
elseif strncmp(HDR.TYPE,'SIGIF',5),
        if any(PERMISSION=='r'),
                HDR.FILE.FID  = fopen(HDR.FileName,'rb','ieee-le');
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
                HDR.T0(1)  = str2double(tmp1);
                if length(tmp1)<3, HDR.T0(1) = 1900+HDR.T0(1); end;
                [tmp1,tmp] = strtok(tmp,'-/'); 
                HDR.T0(2)  = str2double(tmp1);
                [tmp1,tmp] = strtok(tmp,'-/'); 
                HDR.T0(3)  = str2double(tmp1);
                
                HDR.SIG.Type   = fgetl(HDR.FILE.FID);		% 6 simultaneous or serial sampling
                Source = fgetl(HDR.FILE.FID);		% 7 - obsolete
                HDR.NS     = str2double(fgetl(HDR.FILE.FID));  	% 8 number of channels
                HDR.NRec   = str2double(fgetl(HDR.FILE.FID)); % 9 number of segments
                NFrames= str2double(fgetl(HDR.FILE.FID));  % 10 number of frames per segment - obsolete
                
                %HDR.SPR    = str2double(fgetl(HDR.FILE.FID));  			% 11 	number of samples per frame
                HDR.AS.spb  = str2double(fgetl(HDR.FILE.FID));  			% 11 	number of samples per frame
                H1.Bytes_per_Sample = str2double(fgetl(HDR.FILE.FID));	% 12 number of bytes per samples
                HDR.AS.bpb = HDR.AS.spb * H1.Bytes_per_Sample;
                HDR.Sampling_order    = str2double(fgetl(HDR.FILE.FID));  	% 13
                HDR.FLAG.INTEL_format = str2double(fgetl(HDR.FILE.FID));  	% 14
                HDR.FormatCode = str2double(fgetl(HDR.FILE.FID));  	% 15
                
                HDR.CompressTechnique = fgetl(HDR.FILE.FID);  		% 16
                HDR.SignalType = fgetl(HDR.FILE.FID);  			% 17
                
                for k=1:HDR.NS,
                        chandata = fgetl(HDR.FILE.FID);			% 18
                        [tmp,chandata] = strtok(chandata,' ,;:');  
                        HDR.Label(k,1:length(tmp)) = tmp;
                        [tmp,chandata] = strtok(chandata,' ,;:');  
                        HDR.Cal(k) = str2double(tmp);  
                        
                        [tmp,chandata] = strtok(chandata,' ,;:');
                        HDR.SampleRate(k) = str2double(tmp);
                        
                        %[tmp,chandata] = strtok(chandata);  
                        HDR.Variable{k} = chandata;  
                        
                        while  ~isempty(chandata)
                                [tmp,chandata] = strtok(chandata,' ,;:'); 
                                if strcmp(tmp,'G')
                                        [HDR.PhysDim{k},chandata] = strtok(chandata,' ,;:');  
                                end;        
                        end;
                end;
                HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal,HDR.NS+1,HDR.NS);
                HDR.Segment_separator = fgetl(HDR.FILE.FID);  		% 19
                %HDR.Segment_separator = hex2dec(fgetl(HDR.FILE.FID));  
                
                HDR.FLAG.TimeStamp = str2double(fgetl(HDR.FILE.FID));  	% 20
                
                if HDR.VERSION>=3,
                        HDR.FLAG.SegmentLength = str2double(fgetl(HDR.FILE.FID));	% 21  
                        HDR.AppStartMark = fgetl(HDR.FILE.FID);  		% 22
                        HDR.AppInfo = fgetl(HDR.FILE.FID);  			% 23
                else
                        HDR.FLAG.SegmentLength = 0;    
                end;        
                HDR.footer = fgets(HDR.FILE.FID,6);			% 24
                
                if ~strcmp(HDR.footer,'oFSvAI')
                        fprintf(HDR.FILE.stderr,'Warning LOADSIG in %s: Footer not found\n',  HDR.FileName);  
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
                                fprintf(HDR.FILE.stderr,'Warning LOADSIG: FormatCode %i not implemented\n',HDR.FormatCode);
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
                        HDR.T0(4) = str2double(tmp(1:2));
                        HDR.T0(5) = str2double(tmp(3:4));
                        HDR.T0(6) = str2double([tmp(5:6),'.',tmp(7:9)]);
                end;
        end;
        
elseif strcmp(HDR.TYPE,'CTF'),
        if any(PERMISSION=='r'),
                HDR.FILE.FID  = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.res4']),'rb','ieee-be');
		if HDR.FILE.FID<0,
			return
		end;
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS  = 0;
		fseek(HDR.FILE.FID,778,'bof');
		tmp = char(fread(HDR.FILE.FID,255,'char')');
                tmp(tmp==':')=' ';
                tmp = str2double(tmp);
                if length(tmp)==3,
                        HDR.T0(4:6)=tmp;
                end;
		tmp = char(fread(HDR.FILE.FID,255,'char')');
                tmp(tmp=='/')=' ';
                tmp = str2double(tmp);
                if length(tmp)==3,
                        HDR.T0(1:3) = tmp;
                end;
                
		HDR.SPR = fread(HDR.FILE.FID,1,'int32');
		HDR.NS = fread(HDR.FILE.FID,1,'int16');
		HDR.CTF.NS2 = fread(HDR.FILE.FID,1,'int16');
		HDR.SampleRate = fread(HDR.FILE.FID,1,'double');
		HDR.Dur = fread(HDR.FILE.FID,1,'double');
		HDR.NRec = fread(HDR.FILE.FID,1,'int16');
		HDR.CTF.NRec2 = fread(HDR.FILE.FID,1,'int16');
		HDR.TriggerOffset = fread(HDR.FILE.FID,1,'int32');
		
		fseek(HDR.FILE.FID,1712,'bof');
		HDR.PID = char(fread(HDR.FILE.FID,32,'char')');
		HDR.Operator = char(fread(HDR.FILE.FID,32,'char')');
		HDR.FILE.SensorFileName = char(fread(HDR.FILE.FID,60,'char')');

		%fseek(HDR.FILE.FID,1836,'bof');
		HDR.CTF.RunSize = fread(HDR.FILE.FID,1,'int32');
		HDR.CTF.RunSize2 = fread(HDR.FILE.FID,1,'int32');
		HDR.CTF.RunDescription = char(fread(HDR.FILE.FID,HDR.CTF.RunSize,'char')');
		HDR.CTF.NumberOfFilters = fread(HDR.FILE.FID,1,'int16');
		
		for k = 1:HDR.CTF.NumberOfFilters,
			F.Freq = fread(HDR.FILE.FID,1,'double');
			F.Class = fread(HDR.FILE.FID,1,'int32');
			F.Type = fread(HDR.FILE.FID,1,'int32');
			F.NA = fread(HDR.FILE.FID,1,'int16');
			F.A = fread(HDR.FILE.FID,[1,F.NA],'double');
			HDR.CTF.Filter(k) = F; 
		end;
		
		tmp = fread(HDR.FILE.FID,[32,HDR.NS],'char');
		tmp(tmp<0) = 0;
		tmp(tmp>127) = 0;
		tmp(cumsum(tmp==0)>0)=0;
		HDR.Label = char(tmp');
		
		for k = 1:HDR.NS,
			info.index(k,:) = fread(HDR.FILE.FID,1,'int16');
			info.extra(k,:) = fread(HDR.FILE.FID,1,'int16');
			info.ix(k,:) = fread(HDR.FILE.FID,1,'int32');
			info.gain(k,:) = fread(HDR.FILE.FID,[1,4],'double');

			info.index2(k,:) = fread(HDR.FILE.FID,1,'int16');
			info.extra2(k,:) = fread(HDR.FILE.FID,1,'int16');
			info.ix2(k,:) = fread(HDR.FILE.FID,1,'int32');

			fseek(HDR.FILE.FID,1280,'cof');
		end;
		fclose(HDR.FILE.FID);
                
                %%%%% read Markerfile %%%%%
                fid = fopen(fullfile(HDR.FILE.Path,'MarkerFile.mrk'),'rb','ieee-be');
                if fid > 0,
                        while ~feof(fid),
                                s = fgetl(fid);
                                if ~isempty(strmatch('PATH OF DATASET:',s))
                                        file = fgetl(fid);
                                        
                                elseif 0, 
                                        
                                elseif ~isempty(strmatch('TRIAL NUMBER',s))
                                        N = 0; 
                                        x = fgetl(fid);
                                        while ~isempty(x),
                                                tmp = str2double(x);
                                                N = N+1;
                                                HDR.EVENT.POS(N,1) = tmp(1)*HDR.SPR+tmp(2)*HDR.SampleRate;
                                                HDR.EVENT.TYP(N,1) = 1;
                                                x = fgetl(fid);
                                        end
                                else
                                        
                                end
                        end
                        fclose(fid);
                end;
                
		HDR.CTF.info = info;
		ix = (info.index==0) | (info.index==1) | (info.index==9);
		ix0 = find(ix);
		HDR.Cal(ix0) = 1./(info.gain(ix0,1) .* info.gain(ix0,2));
		ix0 = find(~ix);
		HDR.Cal(ix0) = 1./info.gain(ix0,2);
		HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal);
		HDR.FLAG.TRIGGERED = HDR.NRec > 1;
		HDR.AS.spb = HDR.NRec * HDR.NS;
		HDR.AS.bpb = HDR.AS.spb * 4; 
		
		HDR.CHANTYP = char(repmat(32,HDR.NS,1));
		HDR.CHANTYP(info.index==9) = 'E';
		HDR.CHANTYP(info.index==5) = 'M';
		HDR.CHANTYP(info.index==1) = 'R';
		HDR.CHANTYP(info.index==0) = 'R';

		if 0,

		elseif strcmpi(CHAN,'MEG'),
			CHAN = find(info.index==5); 
		elseif strcmpi(CHAN,'EEG'),
			CHAN = find(info.index==9); 
		elseif strcmpi(CHAN,'REF'),
			CHAN = find((info.index==0) | (info.index==1)); 
		elseif strcmpi(CHAN,'other'),
			CHAN = find((info.index~=0) & (info.index~=1) & (info.index~=5) & (info.index~=9)); 
		end;	
		
                HDR.FILE.FID = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.meg4']),'rb','ieee-be');
		HDR.VERSION = char(fread(HDR.FILE.FID,[1,8],'char'));
		HDR.HeadLen = ftell(HDR.FILE.FID);
		fseek(HDR.FILE.FID,0,'eof');
		HDR.AS.endpos = ftell(HDR.FILE.FID);
		fseek(HDR.FILE.FID,HDR.HeadLen,'bof');
        end;

        
elseif strcmp(HDR.TYPE,'BrainVision'),
        % get the header information from the VHDR ascii file
        fid = fopen(HDR.FileName,'rt');
        if fid<0,
                fprintf('Error SOPEN: could not open file %s\n',HDR.FileName);
                return;
        end; 
        tline = fgetl(fid);
        HDR.BV = [];
        UCAL = 0; 
        flag = 1; 
        while ~feof(fid), 
                tline = fgetl(fid);
                if isempty(tline),
                elseif tline(1)==';',
                elseif tline(1)==10, 
                elseif tline(1)==13,    % needed for Octave 
                elseif strncmp(tline,'[Common Infos]',14)
                        flag = 2;     
                elseif strncmp(tline,'[Binary Infos]',14)
                        flag = 3;     
                elseif strncmp(tline,'[Channel Infos]',14)
                        flag = 4;     
                elseif strncmp(tline,'[Coordinates]',12)
                        flag = 5;     
                elseif strncmp(tline,'[Marker Infos]',12)
                        flag = 6;     
                elseif strncmp(tline,'[Comment]',9)
                        flag = 7;     
                elseif strncmp(tline,'[',1)     % any other segment
                        flag = 8;     
                        
                elseif any(flag==[2,3]),
                        [t1,r] = strtok(tline,'=');
                        [t2,r] = strtok(r,['=,',char([10,13])]);
                        if ~isempty(t2),
                                HDR.BV = setfield(HDR.BV,t1,t2);
                        end;
                elseif flag==4,        
                        [t1,r] = strtok(tline,'=');
                        [t2,r] = strtok(r, ['=',char([10,13])]);
                        ix = [find(t2==','),length(t2)];
                        [chan, stat1] = str2double(t1(3:end));
                        HDR.Label{chan,1} = t2(1:ix(1)-1);        
                        HDR.BV.reference{chan,1} = t2(ix(1)+1:ix(2)-1);
                        [v, stat] = str2double(t2(ix(2)+1:end));          % in microvolt
                        if (prod(size(v))==1) & ~any(stat)
                                HDR.Cal(chan) = v;                                
                        else
                                UCAL = 1; 
                                HDR.Cal(chan) = 1;
                        end;
                elseif flag==5,   
                        [t1,r] = strtok(tline,'=');
                        chan = str2double(t1(3:end));
                        [v, stat] = str2double(r(2:end));
                        HDR.ElPos(chan,:) = v;
                end
        end
        fclose(fid);

        % convert the header information to BIOSIG standards
        HDR.NS = str2double(HDR.BV.NumberOfChannels);
        HDR.SampleRate = 1e6/str2double(HDR.BV.SamplingInterval);      % sampling rate in Hz
        if UCAL & ~strncmp(HDR.BV.BinaryFormat,'IEEE_FLOAT',10),
                fprintf(2,'Warning SOPEN (BV): missing calibration values\n');
                HDR.FLAG.UCAL = 1; 
        end;
        HDR.NRec = 1;                   % it is a continuous datafile, therefore one record
        HDR.Calib = [zeros(1,HDR.NS) ; diag(HDR.Cal)];  % is this correct?
        HDR.PhysDim = 'uV';
        HDR.FLAG.TRIGGERED = 0; 
        HDR.Filter.LowPass = repmat(NaN,HDR.NS,1);
        HDR.Filter.HighPass = repmat(NaN,HDR.NS,1);
        HDR.Filter.Notch = repmat(NaN,HDR.NS,1);
        
        if strncmpi(HDR.BV.BinaryFormat, 'int_16',6)
                HDR.GDFTYP = 'int16'; 
                HDR.AS.bpb = HDR.NS * 2; 
        elseif strncmpi(HDR.BV.BinaryFormat, 'ieee_float_32',13)
                HDR.GDFTYP = 'float32'; 
                HDR.AS.bpb = HDR.NS * 4; 
        elseif strncmpi(HDR.BV.BinaryFormat, 'ieee_float_64',13)
                HDR.GDFTYP = 'float64'; 
                HDR.AS.bpb = HDR.NS * 8; 
        end
        
        %read event file 
        fid = fopen(fullfile(HDR.FILE.Path, HDR.BV.MarkerFile),'rt');
        if fid>0,
                while ~feof(fid),
                        s = fgetl(fid);
                        if strncmp(s,'Mk',2),
                                [N,s] = strtok(s(3:end),'=');
                                ix = find(s==',');
                                ix(length(ix)+1)=length(s)+1;
                                N = str2double(N);
                                HDR.EVENT.POS(N,1) = str2double(s(ix(2)+1:ix(3)-1));
                                HDR.EVENT.TYP(N,1) = 0;
                                HDR.EVENT.DUR(N,1) = str2double(s(ix(3)+1:ix(4)-1));
                                HDR.EVENT.CHN(N,1) = str2double(s(ix(4)+1:ix(5)-1));
                                HDR.EVENT.TeegType{N,1} = s(2:ix(1)-1);
                                HDR.EVENT.TeegDesc{N,1} = s(ix(1)+1:ix(2)-1);
                        end;
                end
                fclose(fid);
        end

        %open data file 
        if strncmpi(HDR.BV.DataFormat, 'binary',5)
                PERMISSION='rb';
        elseif strncmpi(HDR.BV.DataFormat, 'ascii',5)                 
                PERMISSION='rt';
        end;

        HDR.FILE.FID = fopen(fullfile(HDR.FILE.Path,HDR.BV.DataFile),PERMISSION,'ieee-le');
        if HDR.FILE.FID < 0,
                fprintf(HDR.FILE.stderr,'ERROR SOPEN BV: could not open file %s\n',fullfile(HDR.FILE.Path,HDR.BV.DataFile));
                return;
        end;
        
        HDR.FILE.OPEN= 1; 
        HDR.FILE.POS = 0; 
        HDR.HeadLen  = 0; 
        if strncmpi(HDR.BV.DataFormat, 'binary',5)
                fseek(HDR.FILE.FID,0,'eof');
                HDR.AS.endpos = ftell(HDR.FILE.FID);
                fseek(HDR.FILE.FID,0,'bof');
                HDR.AS.endpos = HDR.AS.endpos/HDR.AS.bpb;
                
        elseif strncmpi(HDR.BV.DataFormat, 'ascii',5)  
                s = char(sread(HDR.FILE.FID,inf,'char')');
                s(s==',')='.';
                [tmp,status] = str2double(s);
                if strncmpi(HDR.BV.DataOrientation, 'multiplexed',6),
                        HDR.BV.data = tmp;
                elseif strncmpi(HDR.BV.DataOrientation, 'vectorized',6),
                        HDR.BV.data = tmp';
                end
                HDR.AS.endpos = size(HDR.BV.data,1);
                if ~any(HDR.NS ~= size(tmp));
                        fprintf(HDR.FILE.stderr,'ERROR SOPEN BV-ascii: number of channels inconsistency\n');
                end;
        end
        HDR.SPR = HDR.AS.endpos;
        
        
elseif strcmp(HDR.TYPE,'EEProbe-CNT'),

        if 1, %try
                % Read the first sample of the file with a mex function
                % this also gives back header information, which is needed here
                tmp = read_eep_cnt(HDR.FileName, 1, 1);

                % convert the header information to BIOSIG standards
                HDR.FILE.FID = 1;               % ?
                HDR.FILE.POS = 0;
                HDR.NS = tmp.nchan;             % number of channels
                HDR.SampleRate = tmp.rate;      % sampling rate
                HDR.NRec = 1;                   % it is always continuous data, therefore one record
                HDR.FLAG.TRIGGERED = 0; 
                HDR.SPR = tmp.nsample;          % total number of samples in the file
                HDR.Dur = tmp.nsample/tmp.rate; % total duration in seconds
                HDR.Calib = [zeros(1,HDR.NS) ; eye(HDR.NS, HDR.NS)];  % is this correct?
                HDR.Label = char(tmp.label);
                HDR.PhysDim = 'uV';
                HDR.AS.endpos = HDR.SPR;
                HDR.Label = tmp.label;
                
        else %catch
                fprintf(HDR.FILE.stderr,'Warning SOPEN (EEProbe): only experimental version. \n');

                HDR.FILE.FID = fopen(HDR.FileName,'rb');
                H = openiff(HDR.FILE.FID);
                if isfield(H,'RIFF');
                        HDR.FILE.OPEN = 1; 
                        HDR.RIFF = H.RIFF;
                        HDR.Label = {};
                        HDR.PhysDim = {};
                        if isfield(HDR.RIFF,'CNT');
                                if isfield(HDR.RIFF.CNT,'eeph');
				if ~isstruct(HDR.RIFF.CNT.eeph);
                                        rest = char(HDR.RIFF.CNT.eeph');                        
                                        while ~isempty(rest), 
                                                [tline,rest] = strtok(rest,[10,13]);
                                                if isempty(tline),
                                                        
                                                elseif strncmp(tline,'[Sampling Rate]',13)
                                                        [tline,rest] = strtok(rest,[10,13]);
                                                        [HDR.SampleRate,status] = str2double(tline);
                                                elseif strncmp(tline,'[Samples]',7)
                                                        [tline,rest] = strtok(rest,[10,13]);
                                                        [HDR.SPR,status] = str2double(tline);
                                                elseif strncmp(tline,'[Channels]',8)
                                                        [tline,rest] = strtok(rest,[10,13]);
                                                        [HDR.NS,status] = str2double(tline);
                                                elseif strncmp(tline,'[Basic Channel Data]',16)
                                                        while rest(2)==';'
                                                                [tline,rest] = strtok(rest,[10,13]);
                                                        end;
                                                        k = 1; 
                                                        while k<=HDR.NS,
                                                                [tline,rest] = strtok(rest,[10,13]);
                                                                [HDR.Label{k,1}, R] = strtok(tline,[9,10,13,32]);    
                                                                [Dig , R]  = strtok(R,[9,10,13,32]);    
                                                                [Phys, R]  = strtok(R,[9,10,13,32]);    
                                                                HDR.Cal(k) = str2double(Phys)/str2double(Dig); 
                                                                [HDR.PhysDim{k,1}, R] = strtok(R,[9,10,13,32]);    
                                                                k = k + 1;
                                                        end;
                                                        HDR.Calib = [zeros(1,HDR.NS);diag(HDR.Cal)];
                                                elseif strncmp(tline,'[History]',9)
                                                else
                                                end
                                        end;
                                end
				end;
                        end
                end
        end

        fid = fopen(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.trg']),'rt');
        if fid>0,
                header = fgetl(fid);
                N = 0; 
                while ~feof(fid),
                        tmp = fscanf(fid, '%f %d %s', 3);
                        if ~isempty(tmp)
                                N = N + 1; 
                                HDR.EVENT.POS(N,1) = tmp(1)*HDR.SampleRate;
                                HDR.EVENT.TYP(N,1) = 0;
                                %HDR.EVENT.DUR(N,1) = 0;
                                %HDR.EVENT.CHN(N,1) = 0;
                                
                                HDR.EVENT.TeegType{N,1}   = char(tmp(3:end));		% string
                                HDR.EVENT.TYP(N,1) = str2double(HDR.EVENT.TeegType{N,1});		% numeric
                        end
                end
                fclose(fid);
        end;
        
                
elseif strcmp(HDR.TYPE,'EEProbe-AVR'),
        % it appears to be a EEProbe file with an averaged ERP
        try
                tmp = read_eep_avr(HDR.FileName);
        catch
                fprintf(HDR.FILE.stderr,'ERROR SOPEN (EEProbe): Cannot open EEProbe-file, because read_eep_avr.mex not installed. \n');
                fprintf(HDR.FILE.stderr,'ERROR SOPEN (EEProbe): see http://www.smi.auc.dk/~roberto/eeprobe/\n');
                return;
        end

        % convert the header information to BIOSIG standards
        HDR.FILE.FID = 1;               % ?
        HDR.FILE.POS = 0;
        HDR.NS = tmp.nchan;             % number of channels
        HDR.SampleRate = tmp.rate;      % sampling rate
        HDR.NRec  = 1;                   % it is an averaged ERP, therefore one record
        HDR.SPR   = tmp.npnt;             % total number of samples in the file
        HDR.Dur   = tmp.npnt/tmp.rate;    % total duration in seconds
        HDR.Calib = [zeros(1,HDR.NS) ; eye(HDR.NS, HDR.NS)];  % is this correct?
        HDR.Label = char(tmp.label);
        HDR.PhysDim   = 'uV';
        HDR.FLAG.UCAL = 1;
        HDR.FILE.POS  = 0; 
        HDR.AS.endpos = HDR.SPR;
        HDR.Label = tmp.label;
        HDR.TriggerOffset = 0; 
        
        HDR.EEP.data = tmp.data';
        
        
elseif strncmp(HDR.TYPE,'FIF',3),
        if any(exist('rawdata')==[3,6]),
		if isempty(FLAG_NUMBER_OF_OPEN_FIF_FILES)
			FLAG_NUMBER_OF_OPEN_FIF_FILES = 0;
		end;	    
                if ~any(FLAG_NUMBER_OF_OPEN_FIF_FILES==[0,1])
                        fprintf(HDR.FILE.stderr,'ERROR SOPEN (FIF): number of open FIF files must be zero or one\n\t Perhaps, you forgot to SCLOSE(HDR) the previous FIF-file.\n');
                        return;
                end;

                try
                        rawdata('any',HDR.FileName);  % opens file 
                catch
                        tmp = which('rawdata');
                        [p,f,e]=fileparts(tmp);
                        fprintf(HDR.FILE.stderr,'ERROR SOPEN (FIF): Maybe you forgot to do \"export LD_LIBRARY_PATH=%s/i386 \" before you started Matlab. \n',p);
                        return
                end
                HDR.FILE.FID = 1;
                HDR.SampleRate = rawdata('sf');
                HDR.AS.endpos = rawdata('samples');
                [HDR.MinMax,HDR.Cal] = rawdata('range');
                [HDR.Label, type, number] = channames(HDR.FileName);
        
                rawdata('goto', 0);
                [buf, status] = rawdata('next'); 
                HDR.Dur = rawdata('t');
                [HDR.NS,HDR.SPR] = size(buf);
                HDR.AS.bpb = HDR.NS * 2;
                HDR.Calib = [zeros(1,HDR.NS);diag(HDR.Cal)]; 
                
                rawdata('goto', 0);
                HDR.FILE.POS = 0; 
                HDR.FILE.OPEN = 1; 
                FLAG_NUMBER_OF_OPEN_FIF_FILES = FLAG_NUMBER_OF_OPEN_FIF_FILES+1; 
                
        else
                fprintf(HDR.FILE.stderr,'ERROR SOPEN (FIF): NeuroMag FIFF access functions not available. \n');
                fprintf(HDR.FILE.stderr,'\tOnline available at: http://www.kolumbus.fi/kuutela/programs/meg-pd/ \n');
                return;
        end
        
        
elseif strncmp(HDR.TYPE,'FS3',3),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-be');
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS  = 0;
                HDR.Date = fgets(HDR.FILE.FID);
                HDR.Info = fgets(HDR.FILE.FID);
                HDR.SURF.N = fread(HDR.FILE.FID,1,'int32');
                HDR.FACE.N = fread(HDR.FILE.FID,1,'int32');
                HDR.VERTEX.COORD =   fread(HDR.FILE.FID,3*HDR.SURF.N,'float32');
                
                HDR.FACES = fread(HDR.FILE.FID,[3,HDR.FACE.N],'int32')';
                fclose(HDR.FILE.FID);
        end
        
        
elseif strncmp(HDR.TYPE,'FS4',3),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-be');
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS  = 0;
                
                tmp = fread(HDR.FILE.FID,[1,3],'uint8');
                HDR.SURF.N = tmp*(2.^[16;8;1]);
                tmp = fread(HDR.FILE.FID,[1,3],'uint8');
                HDR.FACE.N = tmp*(2.^[16;8;1]);
                HDR.VERTEX.COORD = fread(HDR.FILE.FID,3*HDR.SURF.N,'int16')./100;
                tmp = fread(HDR.FILE.FID,[4*HDR.FACE.N,3],'uint8')*(2.^[16;8;1]);
                HDR.FACES = reshape(tmp,4,HDR.FACE.N)';
                fclose(HDR.FILE.FID);
        end
        
        
elseif strncmp(HDR.TYPE,'TRI',3),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS  = 0;
                
                HDR.ID = fread(HDR.FILE.FID,1,'int32');
                HDR.type = fread(HDR.FILE.FID,1,'short');
                HDR.VERSION = fread(HDR.FILE.FID,1,'short');
                HDR.ELEC.Thickness = fread(HDR.FILE.FID,1,'float');
                HDR.ELEC.Diameter = fread(HDR.FILE.FID,1,'float');
                HDR.reserved = fread(HDR.FILE.FID,4080,'char');
                
                HDR.FACE.N = fread(HDR.FILE.FID,1,'short');
                HDR.SURF.N = fread(HDR.FILE.FID,1,'short');
                
                HDR.centroid = fread(HDR.FILE.FID,[4,HDR.FACE.N],'float')';
                HDR.VERTICES = fread(HDR.FILE.FID,[4,HDR.SURF.N],'float')';
                HDR.FACES = fread(HDR.FILE.FID,[3,HDR.FACE.N],'short')';
                
                HDR.ELEC.N = fread(HDR.FILE.FID,1,'ushort');
                for k = 1:HDR.ELEC.N,
                        HDR.elec(k).Label = fread(HDR.FILE.FID,10,'char');	
                        HDR.elec(k).Key = fread(HDR.FILE.FID,1,'short');	
                        tmp = fread(HDR.FILE.FID,[1,3],'float');	
                        HDR.elec(k).POS = tmp(:);	
                        HDR.ELEC.XYZ(k,:) = tmp;
                        HDR.elec(k).idx = fread(HDR.FILE.FID,1,'ushort');	
                end;
                fclose(HDR.FILE.FID);
        end
        
        
elseif strcmp(HDR.TYPE,'DICOM'),
	HDR = opendicom(HDR,PERMISSION,CHAN);
        
        
elseif 0, strcmp(HDR.TYPE,'DXF'),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rt','ieee-le');
                
		while ~feof(HDR.FILE.FID),
	                line1 = fgetl(HDR.FILE.FID);
    		        line2 = fgetl(HDR.FILE.FID);
			
			[val,status] = str2double(line1);
			
			if any(status),
				error('SOPEN (DXF)');
			elseif val==999, 
			
			elseif val==0, 
			
			elseif val==1, 
			
			elseif val==2, 
			
			else
			
			end;
		end;
		
                fclose(HDR.FILE.FID);
        end


elseif strcmp(HDR.TYPE,'STX'),
        if any(PERMISSION=='r'),
                fid = fopen(HDR.FileName,'rt','ieee-le');
                FileInfo = fread(fid,20,'char');
                HDR.Label = fread(fid,50,'char');
                tmp = fread(fid,6,'int');
                HDR.NRec = tmp(1);
		HDR.SPR = 1; 
		
		tmp = fread(fid,5,'long');
		HDR.HeadLen = 116;

                fclose(HDR.FILE.FID);
        end

        
elseif strcmp(HDR.TYPE,'BIFF'),
	try, 
                [HDR.TFM.S,HDR.TFM.E] = xlsread(HDR.FileName,'Beat-To-Beat');
                if size(HDR.TFM.S,1)+1==size(HDR.TFM.E,1),
                        HDR.TFM.S = [repmat(NaN,1,size(HDR.TFM.S,2));HDR.TFM.S];
                end;
                HDR.TYPE = 'TFM_EXCEL_Beat_to_Beat'; 
                HDR.T0 = datevec(HDR.TFM.S(2,1)+HDR.TFM.S(2,2)-1);
                HDR.T0(1) = HDR.T0(1)+1900;
        catch
	end; 	

        if strcmp(HDR.TYPE, 'TFM_EXCEL_Beat_to_Beat');
                if ~isempty(strfind(HDR.TFM.E{3,1},'---'))
                        HDR.TFM.S(3,:) = [];    
                        HDR.TFM.E(3,:) = [];    
                end;
                
                HDR.Label   = strvcat(HDR.TFM.E(4,:)');
                HDR.PhysDim = strvcat(HDR.TFM.E(5,:)');
           
                HDR.TFM.S = HDR.TFM.S(6:end,:);
                HDR.TFM.E = HDR.TFM.E(6:end,:);
                
                ix = find(isnan(HDR.TFM.S(:,2)) & ~isnan(HDR.TFM.S(:,1)));
                HDR.EVENT.Desc = HDR.TFM.E(ix,2);
                HDR.EVENT.TYP  = zeros(length(ix),1);
                HDR.EVENT.POS  = ix(:);
                
		[HDR.SPR,HDR.NS] = size(HDR.TFM.S);
                if any(CHAN),
			HDR.TFM.S = HDR.TFM.S(:,CHAN);
			HDR.TFM.E = HDR.TFM.E(:,CHAN);
		end;
		HDR.NRec = 1;
		HDR.THRESHOLD  = repmat([0,NaN],HDR.NS,1); 	% Underflow Detection 
		HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
        end;


elseif strncmp(HDR.TYPE,'XML',3),
        if any(PERMISSION=='r'),
                fid = fopen(HDR.FileName,'rb','ieee-le');
                if strcmp(HDR.TYPE,'XML-UTF16'),
                        magic = char(fread(fid,1,'uint16'));
                        HDR.XML = char(fread(fid,[1,inf],'uint16'));
                elseif strcmp(HDR.TYPE,'XML-UTF8'),
                        HDR.XML = char(fread(fid,[1,inf],'char'));
                end;
                fclose(fid);
                HDR.FILE.FID = fid;
                if 1, try,
                        XML = xmltree(HDR.XML);
                        XML = convert(XML);
                        HDR.XML  =  XML; 
			HDR.TYPE = 'XML';
                catch
                        fprintf(HDR.FILE.stderr,'ERROR SOPEN (XML): XML-toolbox missing or invalid XML file.\n');
                        return;
                end;
                end;
		
		
                try,    % SierraECG  1.03  *.open.xml from PHILIPS
                        HDR.SampleRate = str2double(HDR.XML.dataacquisition.signalcharacteristics.samplingrate);
                        HDR.NS  = str2double(HDR.XML.dataacquisition.signalcharacteristics.numberchannelsvalid);
                        HDR.Cal = str2double(HDR.XML.reportinfo.reportgain.amplitudegain.overallgain);
                        HDR.PhysDim = 'uV';
                        HDR.Filter.HighPass = str2double(HDR.XML.reportinfo.reportbandwidth.highpassfiltersetting);
                        HDR.Filter.LowPass  = str2double(HDR.XML.reportinfo.reportbandwidth.lowpassfiltersetting);
                        HDR.Filter.Notch    = str2double(HDR.XML.reportinfo.reportbandwidth.notchfiltersetting);
                        
                        t = HDR.XML.reportinfo.reportformat.waveformformat.mainwaveformformat;
                        k = 0; 
                        HDR.Label=[];
                        while ~isempty(t),
                                [s,t] = strtok(t,' ');
                                k = k+1; 
                                HDR.Label{k, 1} = s;
                        end;
                        
                        HDR.VERSION = HDR.XML.documentinfo.documentversion;
                        HDR.TYPE = HDR.XML.documentinfo.documenttype;
                catch
                        
                try,    % FDA-XML Format
                        tmp   = HDR.XML.component.series.derivation;
                        if isfield(tmp,'Series');
                                tmp = tmp.Series.component.sequenceSet.component;
                        else    % Dovermed.CO.IL version of format
                                tmp = tmp.derivedSeries.component.sequenceSet.component;
                        end;
                        HDR.NS = length(tmp)-1;
                        HDR.NRec = 1; 
                        HDR.Cal = 1;
                        HDR.PhysDim = ' ';
                        HDR.SampleRate = 1;
                        HDR.TYPE = 'XML-FDA';     % that's an FDA XML file 
                catch
                        fprintf(HDR.FILE.stderr,'Warning SOPEN (XML): File %s is not supported.\n',HDR.FileName);
                        return;
                end;
                end
                
                HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal);
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS  = 0;
        end;
        
        
elseif strcmp(HDR.TYPE,'unknown'),
        HDR.ERROR.status = -1;
	%HDR.ERROR.message = sprintf('ERROR SOPEN: File %s could not be opened - unknown type.\n',HDR.FileName);
	%fprintf(HDR.FILE.stderr,'ERROR SOPEN: File %s could not be opened - unknown type.\n',HDR.FileName);
        HDR.FILE.FID = -1;
        return;
        
else
        %fprintf(HDR.FILE.stderr,'SOPEN does not support your data format yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
        HDR.FILE.FID = -1;	% this indicates that file could not be opened. 
        return;
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%	General Postprecessing for all formats of Header information 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% add trigger information for triggered data
if HDR.FLAG.TRIGGERED & isempty(HDR.EVENT.POS)
	HDR.EVENT.POS = [0:HDR.NRec-1]'*HDR.SPR+1;
	HDR.EVENT.TYP = repmat(hex2dec('0300'),HDR.NRec,1);
	HDR.EVENT.CHN = repmat(0,HDR.NRec,1);
	HDR.EVENT.DUR = repmat(0,HDR.NRec,1);
end;

% apply channel selections to EVENT table
if any(CHAN) & ~isempty(HDR.EVENT.POS) & isfield(HDR.EVENT,'CHN'),	% only if channels are selected. 
	sel = (HDR.EVENT.CHN(:)==0);	% memory allocation, select all general events
	for k = find(~sel'),		% select channel specific elements
		sel(k) = any(HDR.EVENT.CHN(k)==CHAN);
	end;
	HDR.EVENT.POS = HDR.EVENT.POS(sel);
	HDR.EVENT.TYP = HDR.EVENT.TYP(sel);
	HDR.EVENT.DUR = HDR.EVENT.DUR(sel);	% if EVENT.CHN available, also EVENT.DUR is defined. 
	HDR.EVENT.CHN = HDR.EVENT.CHN(sel);
	% assigning new channel number 
	a = zeros(1,HDR.NS);
	for k = 1:length(CHAN),		% select channel specific elements
		a(CHAN(k)) = k;		% assigning to new channel number. 
	end;
	ix = HDR.EVENT.CHN>0;
	HDR.EVENT.CHN(ix) = a(HDR.EVENT.CHN(ix));	% assigning new channel number
end;	

if any(PERMISSION=='r') & ~isnan(HDR.NS);
        if exist('ReRefMx','var'),
                % fix size if ReRefMx
                sz = size(ReRefMx); 	                 
                if sz(1) > HDR.NS, 	 
                        fprintf(HDR.FILE.stderr,'ERROR: size of ReRefMx [%i,%i] exceeds Number of Channels (%i)\n',size(ReRefMx),HDR.NS); 	 
                        fclose(HDR.FILE.FID); 	 
                        HDR.FILE.FID = -1; 	 
                        return; 	 
                else        
                        ReRefMx = [ReRefMx; zeros(HDR.NS-sz(1),sz(2))];
                end; 	 
                HDR.Calib = HDR.Calib*ReRefMx;
        else
                if CHAN==0,
                        CHAN=1:HDR.NS;
                elseif any(CHAN > HDR.NS),
                        fprintf(HDR.FILE.stderr,'ERROR: selected channels exceed Number of Channels %i\n',HDR.NS);
                        fclose(HDR.FILE.FID); 
                        HDR.FILE.FID = -1;	
                        return;
                end;
		HDR.Calib = HDR.Calib(:,CHAN(:));
        end;
        HDR.InChanSelect = find(any(HDR.Calib(2:HDR.NS+1,:),2));
end;
