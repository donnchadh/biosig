function [HDR,H1,h2] = sopen(arg1,PERMISSION,CHAN,MODE,arg5,arg6)
% SOPEN opens signal files for reading and writing and returns 
%       the header information. 
%       Many different data formats are supported.
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
%   [S,HDR] = sread(HDR, NoR, StartPos);
%   HDR = sclose(HDR);
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

%	$Revision: 1.41 $
%	$Id: sopen.m,v 1.41 2004-03-30 16:15:46 schloegl Exp $
%	(C) 1997-2004 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


if isnan(str2double('1, 3'));
        fprintf(2,'Warning BIOSIG: incorrect version of STR2DOUBLE.\n');
        fprintf(2,'- Its recommended to update STR2DOUBLE. Contact Alois!\n');
end;

if nargin<2, 
        PERMISSION='rb'; 
elseif ~any(PERMISSION=='b');
        PERMISSION = [PERMISSION,'b']; % force binary open. Needed for Octave
end;
if nargin<3, CHAN = 0; end; 
if all(size(CHAN)>1) | any(floor(CHAN)~=CHAN) | (any(CHAN<0) & prod(size(CHAN))>1),
        ReRefMx = CHAN; 
        CHAN = find(any(CHAN,2));
end
if nargin<4, MODE = ''; end;

if ~isstruct(arg1),
        HDR.FileName = arg1;
else
        HDR = arg1;
end;

if isempty(MODE), MODE=' '; end;	% Make sure MODE is not empty -> FINDSTR

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
        HDR.FILE.FID  = -1;
        HDR.TYPE = 'unknown';
        fid = fopen(HDR.FileName,'rb','ieee-le');
        if fid < 0,
                if exist(HDR.FileName)==7, % isdir(...)
                        
                else
                        fprintf(HDR.FILE.stderr,'Error SOPEN: file %s not found.\n',HDR.FileName);    
                        return;
                end;
        else
                [s,c] = fread(fid,[1,132],'uchar');
                if c < 132,
                        s = [s, repmat(0,1,132-c)];
                end;
                if c,
                        %%%% file type check based on magic numbers %%%
                        type_mat4=str2double(char(abs(sprintf('%04i',s(1:4)*[1;10;100;1000]))'));
                        ss = setstr(s);
                        if all(s(1:2)==[207,0]); 
                                HDR.TYPE='BKR';
                        elseif strncmp(ss,'Version 3.0',11); 
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
                                HDR.TYPE='FEF';
                                HDR.VERSION   = str2double(ss(9:16));
                                HDR.Encoding  = str2double(ss(17:24));
                                if any(str2double(ss(25:32))),
                                        HDR.Endianity = 'ieee-be';
                                else
                                        HDR.Endianity = 'ieee-le';
                                end;
                                
                        elseif strncmp(ss,'MEG41CP',7); 
                                HDR.TYPE='CTF';
                        elseif strncmp(ss,'MEG41RS',7) | strncmp(ss,'MEG4RES',7); 
                                HDR.TYPE='CTF';
                        elseif strncmp(ss,'MEG4',4); 
                                HDR.TYPE='CTF';
                        elseif strncmp(ss,'CTF_MRI_FORMAT VER 2.2',22); 
                                HDR.TYPE='CTF';
                        elseif strncmp(ss,'PATH OF DATASET:',16); 
                                HDR.TYPE='CTF';
                                
                                
                        elseif strcmp(ss(1:8),'@  MFER '); 
                                HDR.TYPE='MFER';
                        elseif strcmp(ss(1:6),'@ MFR '); 
                                HDR.TYPE='MFER';
                        elseif all(s(17:22)=='SCPECG'); 
                                HDR.TYPE='SCPECG';
                        elseif strncmp(ss,'POLY_SAM',8);	% Poly5/TMS32 sample file format.
                                HDR.TYPE='TMS32';
                        elseif strncmp(ss,'"Snap-Master Data File"',23);	% Snap-Master Data File .
                                HDR.TYPE='SMA';
                        elseif all(s([1:2,20])==[1,0,0]) & any(s(19)==[2,4]); 
                                HDR.TYPE='TEAM';	% Nicolet TEAM file format
                        elseif strncmp(ss,HDR.FILE.Name,length(HDR.FILE.Name)); 
                                HDR.TYPE='MIT';
                        elseif strncmp(ss,'DEMG',4);	% www.Delsys.com
                                HDR.TYPE='DEMG';
                                
                        elseif strncmp(ss,'.snd',4); 
                                HDR.TYPE='SND';
                                HDR.Endianity = 'ieee-be';
                        elseif strncmp(ss,'dns.',4); 
                                HDR.TYPE='SND';
                                HDR.Endianity = 'ieee-le';
                        elseif strcmp(ss([1:4,9:12]),'RIFFWAVE'); 
                                HDR.TYPE='WAV';
                        elseif strcmp(ss([1:4,9:11]),'FORMAIF'); 
                                HDR.TYPE='AIF';
                        elseif strcmp(ss([1:4,9:12]),'RIFFAVI '); 
                                HDR.TYPE='AVI';
                        elseif strcmp(ss([5:8]),'moov'); 	% QuickTime movie 
                                HDR.TYPE='QTFF';
                        elseif all(s(1:16)==hex2dec(reshape('3026b2758e66cf11a6d900aa0062ce6c',2,16)')')
                                %'75B22630668e11cfa6d900aa0062ce6c'
                                HDR.TYPE='ASF';
                                
                        elseif strncmp(ss,'MPv4',4); 
                                HDR.TYPE='MPv4';
                                HDR.Date = ss(65:87);
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

                        elseif strcmp(ss([1:4,9:16]),'RIFFCNT LIST'); 
                                HDR.TYPE='EEProbe';     % EEGProbe ? 
                                
                        elseif strncmp(ss,'ISHNE1.0',8);        % ISHNE Holter standard output file.
                                HDR.TYPE='ISHNE';
                        elseif strncmp(ss,'rhdE',4);	% Holter Excel 2 file, not supported yet. 
                                HDR.TYPE='rhdE';          
                                
                        elseif strncmp(ss,'RRI',3);	% R-R interval format % Holter Excel 2 file, not supported yet. 
                                HDR.TYPE='RRI';          
                        elseif strncmp(ss,'Repo',4);	% Repo Holter Excel 2 file, not supported yet. 
                                HDR.TYPE='REPO';          
                        elseif strncmp(ss,'Beat',4);	% Beat file % Holter Excel 2 file, not supported yet. 
                                HDR.TYPE='Beat';          
                        elseif strncmp(ss,'Evnt',4);	% Event file % Holter Excel 2 file, not supported yet. 
                                HDR.TYPE='EVNT';          
                                
                        elseif strncmp(ss,'CFWB',4); 	% Chart For Windows Binary data, defined by ADInstruments. 
                                HDR.TYPE='CFWB';
                                
                        elseif any(s(3:6)*(2.^[0;8;16;24]) == (30:40))
                                HDR.TYPE='ACQ';
                        elseif all(s(1:2)==[hex2dec('55'),hex2dec('AA')]);
                                HDR.TYPE='RDF';
                                
                        elseif all(s(1:2)==[hex2dec('55'),hex2dec('3A')]);      % little endian 
                                HDR.TYPE='SEG2';
                                HDR.Endianity = 'ieee-le';
                        elseif all(s(1:2)==[hex2dec('3A'),hex2dec('55')]);      % big endian 
                                HDR.TYPE='SEG2 b';
                                HDR.Endianity = 'ieee-be';
                                
                        elseif strncmp(ss,'MATLAB Data Acquisition File.',29);  % Matlab Data Acquisition File 
                                HDR.TYPE='DAQ';
                        elseif strncmp(ss,'MATLAB 5.0 MAT-file',19); 
                                HDR.TYPE='MAT5';
                                fseek(fid,126,'bof');
                                tmp = fread(fid,1,'uint16');
                                if tmp==(abs('MI').*[256,1])
                                        HDR.Endianity = 'ieee-le';
                                elseif tmp==(abs('IM').*[256,1])
                                        HDR.Endianity = 'ieee-be';
                                end;
                                
                        elseif any(s(1)==[49:51]) & all(s([2:4,6])==[0,50,0,0]) & any(s(5)==[49:50]),
                                HDR.TYPE = 'WFT';	% nicolet 	
                                
                        elseif all(s(1:3)==[255,255,254]); 	% FREESURVER TRIANGLE_FILE_MAGIC_NUMBER
                                HDR.TYPE='FS3';
                        elseif all(s(1:3)==[255,255,255]); 	% FREESURVER QUAD_FILE_MAGIC_NUMBER or CURVATURE
                                HDR.TYPE='FS4';
                        elseif all(s(1:3)==[0,1,134]) & any(s(4)==[162:164]); 	% SCAN *.TRI file 
                                HDR.TYPE='TRI';
                        elseif all((s(1:4)*(2.^[24;16;8;1]))==1229801286); 	% GE 5.X format image 
                                HDR.TYPE='5.X';
                                
                        elseif all(s(1:2)==[102,105]); 
                                HDR.TYPE='669';
                        elseif all(s(1:2)==[234,96]); 
                                HDR.TYPE='ARJ';
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
                        elseif strncmp(ss,'CPT9FILE',8);        % Corel PhotoPaint Format
                                HDR.TYPE='CPT9';        
                                
                        elseif all(s(21:28)==abs('ACR-NEMA')); 
                                HDR.TYPE='ACR-NEMA';
                        elseif all(s(1:132)==[zeros(1,128),abs('DICM')]); 
                                HDR.TYPE='DICOM';
                        elseif all(s(1:8)==[8,0,5,0,10,0,0,0]); % another DICOM format ? 
                                HDR.TYPE='DICOM';
                        elseif all(s(1:8)==[8,0,0,0,4,0,0,0]);	% another DICOM format ? 
                                HDR.TYPE='DICOM';
                                
                        elseif all(s(1:2)==[255,254]) & all(s(4:2:end)==0)
                                HDR.TYPE='XML-UTF16';
                        elseif ~isempty(findstr(ss,'?xml version'))
                                HDR.TYPE='XML-UTF8';
                                
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
                                HDR.TYPE='MPG2MOV';
                        elseif strcmp(ss([3:5,7]),'-lh-'); 
                                HDR.TYPE='LZH';
                        elseif strcmp(ss([3:5,7]),'-lz-'); 
                                HDR.TYPE='LZH';
                        elseif strcmp(ss(1:4),'MThd'); 
                                HDR.TYPE='MIDI';
                        elseif (s(1)==255) & any(s(2)>=224); 
                                HDR.TYPE='MPEG';
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
                        elseif strcmpi(HDR.FILE.Name,ss(1:length(HDR.FILE.Name)));
                                HDR.TYPE='HEA';
                                
                                %elseif all(~type_mat4),  % should be last, otherwise to many false detections
                        elseif all(s(1:4)==0),  % should be last, otherwise to many false detections
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
                                fseek(fid,3228,-1);
                                s=fread(fid,[1,4],'uint8'); 
                                if all((s(1:4)*(2.^[24;16;8;1]))==1229801286); 	% GE LX2 format image 
                                        HDR.TYPE='LX2';
                                else
                                        %TYPE='unknown';
                                        HDR.TYPE='unknown';
                                end;
                        end;
                end;
                fclose(fid);

                if strcmp(HDR.TYPE,'unknown'),
                        % alpha-TRACE Medical software
                        if (strcmpi(HDR.FILE.Name,'rawdata') | strcmpi(HDR.FILE.Name,'rawhead')) & isempty(HDR.FILE.Ext),
                                if exist(fullfile(HDR.FILE.Path,'digin')) & exist(fullfile(HDR.FILE.Path,'r_info'));	
                                        HDR.TYPE = 'alpha'; %alpha trace medical software 
                                end;
                        end;
                        TYPE = [];
                        
                        %%% this is the file type check based on the file extionsion, only.  
                        if 0, 
                                
                        elseif strcmpi(HDR.FILE.Ext,'ds') & (exist(HDR.FileName)==7), % .. & isdir(HDR.FileName)
                                f1 = fullfile(HDR.FileName,[HDR.FILE.Name,'.meg4']);
                                f2 = fullfile(HDR.FileName,[HDR.FILE.Name,'.res4']);
                                f3 = fullfile(HDR.FileName,['MarkerFile.mrk']);
                                % and check further files, 
                                if (exist(f1)==2) & (exist(f2)==2) & (exist(f3)==2)
                                        HDR.FILE.Path = HDR.FileName; 
                                        HDR.FILE.Ext  = 'MEG4'; 
                                        HDR.TYPE = 'CTF';
                                end;
                                
                        elseif strcmpi(HDR.FILE.Ext,'hdm')
                                
                        elseif strcmpi(HDR.FILE.Ext,'hc')
                                
                        elseif strcmpi(HDR.FILE.Ext,'shape')
                                
                        elseif strcmpi(HDR.FILE.Ext,'shape_info')
                                
                        elseif strcmpi(HDR.FILE.Ext,'trg')
                                
                        elseif strcmpi(HDR.FILE.Ext,'rej')
                                
                        elseif strcmpi(HDR.FILE.Ext,'elc')
                                
                        elseif strcmpi(HDR.FILE.Ext,'vol')
                                
                        elseif strcmpi(HDR.FILE.Ext,'bnd')
                                
                        elseif strcmpi(HDR.FILE.Ext,'msm')
                                
                        elseif strcmpi(HDR.FILE.Ext,'msr')
                                HDR.TYPE = 'ASA2';    % Brainvision ?
                                
                        elseif strcmpi(HDR.FILE.Ext,'dip')
                                
                        elseif strcmpi(HDR.FILE.Ext,'mri')
                                
                        elseif strcmpi(HDR.FILE.Ext,'iso')
                                
                                
                        elseif strcmpi(HDR.FILE.Ext,'hdr')
                                
                        elseif strcmpi(HDR.FILE.Ext,'img')
                                
                                
                        elseif strcmpi(HDR.FILE.Ext,'vhdr')
                                TYPE = 'BrainVision';    % Brainvision ?
                                
                        elseif strcmpi(HDR.FILE.Ext,'vmrk')
                                TYPE = 'BrainVision';    % Brainvision ?
                                
                        elseif strcmpi(HDR.FILE.Ext,'vabs')
                                TYPE = 'BrainVision';    % Brainvision ?
                                
                        elseif strcmpi(HDR.FILE.Ext,'eeg')
                                TYPE = 'BrainVision';    % Brainvision ?
                                
                        elseif strcmpi(HDR.FILE.Ext,'seg')
                                
                        elseif strcmpi(HDR.FILE.Ext,'fif')
                                HDR.TYPE = 'FIF'; 
                                
                        elseif strcmpi(HDR.FILE.Ext,'bdip')
                                
                        elseif strcmpi(HDR.FILE.Ext,'elp')
                                
                        elseif strcmpi(HDR.FILE.Ext,'sfp')
                                
                        elseif strcmpi(HDR.FILE.Ext,'ela')
                                
                        elseif strcmpi(HDR.FILE.Ext,'trl')
                                
                        end;
                        
                        if strcmp(TYPE,'BrainVision')
                                if (exist(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.eeg']))==2) & (exist(fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.vhdr']))==2),
                                        HDR.TYPE=TYPE;
                                end;
                        end;
                end;
                
                if strcmp(HDR.TYPE,'BKR'),
                        if ~isempty(findstr(lower(HDR.FILE.Ext),'cnt')),
                                fprintf(HDR.FILE.stderr,'Warning (dedicated to BB & CB): %s IS A BKR, not a CNT file\n',HDR.FileName);
                        end;
                end;
        end;
        
        % initialize Header 
        HDR.NS = NaN; 
        HDR.SampleRate = NaN; 
        HDR.Label = []; 
        HDR.T0 = repmat(nan,1,6);
        HDR.PhysDim = '';
        
        HDR.Filter.LowPass  = NaN; 
        HDR.Filter.HighPass = NaN; 
        HDR.FLAG.UCAL = ~isempty(findstr(MODE,'UCAL'));   % FLAG for UN-CALIBRATING
        HDR.FLAG.FILT = 0; 	% FLAG if any filter is applied; 
        
        HDR.EVENT.N   = 0; 
        HDR.EVENT.TYP = []; 
        HDR.EVENT.POS = []; 
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

        %ReRefMx = EDF.SIE.REG*EDF.SIE.ReRefMx;
        
        
elseif strcmp(HDR.TYPE,'BKR'),
        HDR = bkropen(HDR,PERMISSION,CHAN);
        
        
elseif strmatch(HDR.TYPE,['CNT';'AVG';'EEG']),
        if any(PERMISSION=='r');
                [HDR,H1,h2] = cntopen(HDR,PERMISSION,CHAN);
                
        elseif any(PERMISSION=='w');
                % check header information
                if ~isfield(HDR,'NS'),
                        HDR.NS = 0;
                end;
                if ~isfinite(HDR.NS) | (HDR.NS<0)
                        fprintf(2,'Error SOPEN CNT-Write: HDR.NS not defined\n');
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
                        fprintf(2,'Error SOPEN CNT-Write: Headersize does not fit\n');
                end;
        end;
        
        
elseif strcmp(HDR.TYPE,'FEF'),		% FEF/Vital format included
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
        fseek(HDR.FILE.FID,32,'bof'); 	% skip preamble
        
        if exist('fefopen')==2,
                HDR = fefopen(HDR,PERMISSION);
        end;
        
        fprintf(2,'Warning SOPEN: Implementing Vital/FEF format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
        fclose(HDR.FILE.FID);
        
        
elseif strcmp(HDR.TYPE,'SCPECG'),	%
        HDR = scpopen(HDR,PERMISSION);        
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
        
        
elseif strcmp(HDR.TYPE,'EBS'),
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-be');
        
        fprintf(2,'Warning SOPEN: Implementing EBS format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
        
        %%%%% (1) Fixed Header (32 bytes) %%%%%
        HDR.VERSION = fread(HDR.FILE.FID,[1,8],'char');	%
        if setstr(HDR.VERSION(1:3)')~='EBS' 
                fprintf(2,'Error LOADEBS: %s not an EBS-File',HDR.FileName); 
                if any(HDR.VERSION(4:8)~=hex2dec(['94';'0a';'13';'1a';'0d'])'); 
                        fprintf(2,'Warning SOPEN EBS: %s may be corrupted',HDR.FileName); 
                end; 
        end;
        HDR.EncodingId = fread(HDR.FILE.FID,1,'int32');	%
        HDR.NS  = fread(HDR.FILE.FID,1,'uint32');	% Number of Channels
        HDR.SPR = fread(HDR.FILE.FID,2,'uint32');	% Number of Samples
        if HDR.SPR(1)==0,
                HDR.SPR=HDR.SPR(2)
        else 
                fprintf(2,'Error SOPEN: EBS-FILE %s too large',HDR.FileName); 
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
        
        
elseif strcmp(HDR.TYPE,'FEF'),		% FEF/Vital format included
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
                HDR.FILE.OPEN = 1; 
        end;
        
        % Holter Excel 2 file, not supported yet. 
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
        
        fseek(HDR.FILE.FID,0,'eof');
        HDR.AS.endpos = ftell(HDR.FILE.FID);
        HDR.NRec = (HDR.AS.endpos-HDR.HeadLen)/1024;
        fseek(HDR.FILE.FID,HDR.HeadLen,'bof');
        
        fprintf(1,'Warning SOPEN HolterExcel2: is under construction.\n');
        
        if (nargout>1),	% just for testing
                H1 = fread(HDR.FILE.FID,[1,inf],'uchar')';
        end;
        fclose(HDR.FILE.FID);
        
        
elseif strcmp(HDR.TYPE,'alpha'),
        HDR.FILE.FID = -1;      % make sure SLOAD does not call SREAD;
        
        % The header files are text files (not binary).
        if exist('OCTAVE_VERSION')>2,
                PERMISSION = 'r';	% Octave default is text, Octave does not support Mode='rt', 
        else
                PERMISSION = 'rt';	% MatLAB default is binary, force Mode='rt';
        end;
        
        fid = fopen(fullfile(HDR.FILE.Path,'rawhead'),PERMISSION);	
        if fid < 0,
                fprintf(2,'Error SOPEN alpha-trace: couldnot open RAWHEAD\n');
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
        
        fid = fopen(fullfile(HDR.FILE.Path,'cal_res'),PERMISSION);
        if fid < 0,
                fprintf(2,'Warning SOPEN alpha-trace: could not open CAL_RES. Data is uncalibrated.\n');
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
                        fprintf(2,'Warning SLOAD alpha-trace: calibration not valid for some channels\n');
                end;
                HDR.Cal(find(~OK)) = NaN;
                HDR.Calib = sparse([-HDR.Off';eye(HDR.NS*[1,1])])*sparse(1:HDR.NS,1:HDR.NS,HDR.Cal);
                HDR.PhysDim = 'µV';
        end;        
        
        fid = fopen(fullfile(HDR.FILE.Path,'r_info'),PERMISSION);
        if fid < 0,
                fprintf(2,'Warning SOPEN alpha-trace: couldnot open R_INFO\n');
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
                fprintf(2,'Warning SOPEN alpha-trace: couldnot open DIGIN - no event information included\n');
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
                                        fprintf(2,'Warning SOPEN: alpha: invalid Event type\n');
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
                        HDR.GDFTYP = 16;        % float
                        HDR.Cal = 1; 
                        HDR.Off = 0; 
                        HDR.AS.bpb = 4*HDR.NS;
                elseif HDR.VERSION==2,
                        HDR.GDFTYP = 4;         % uint16
                        HDR.Cal = (HDR.PhysMax-HDR.PhysMin)/(2^HDR.bits-1);
                        HDR.Off = HDR.PhysMin;
                        HDR.AS.bpb = 2*HDR.NS;
                else    
                        fprintf(2,'Error SOPEN DEMG: invalid version number.\n');
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
                fprintf(2,'Warning SOPEN DEMG: writing not implemented, yet.\n');
        end;
        
        
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
        HiLite    = fread(HDR.FILE.FID,2,'uint8');
        HDR.FirstTimeOffset = fread(HDR.FILE.FID,1,'float64');
        
        if     HDR.VERSION < 34, offset = 150;
        elseif HDR.VERSION < 35, offset = 164; 
        elseif HDR.VERSION < 36, offset = 326; 
        elseif HDR.VERSION < 38, offset = 886; 
        else   offset = 1894; 
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
        HDR.Calib = [HDR.Cal(:);diag(HDR.Off)];
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
        HDR.EVENT.N   = fread(HDR.FILE.FID,1,'int32');
        HDR.EVENT.POS = repmat(nan,HDR.EVENT.N  ,1);
        HDR.EVENT.TYP = repmat(nan,HDR.EVENT.N  ,1);
        for k = 1:HDR.EVENT.N  , 
                %HDR.Event(k).Sample = fread(HDR.FILE.FID,1,'int32');
                HDR.EVENT.POS(k) = fread(HDR.FILE.FID,1,'int32');
                tmp = fread(HDR.FILE.FID,4,'int16');
                HDR.Event(k).selected = tmp(1); 
                HDR.Event(k).TextLocked = tmp(2); 
                HDR.Event(k).PositionLocked = tmp(3); 
                textlen = tmp(4);
                HDR.Event(k).Text = fread(HDR.FILE.FID,textlen,'char');
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
                
        elseif HDR.FILE.TYPE==11, 
                HDR.GDFTYP =  2;
                HDR.bits   =  8;
        elseif HDR.FILE.TYPE==12, 
                HDR.GDFTYP =  4;
                HDR.bits   = 16;
        elseif HDR.FILE.TYPE==13, 
                HDR.GDFTYP = 511+24;
                HDR.bits   = 24;
        elseif HDR.FILE.TYPE==14, 
                HDR.GDFTYP =  6;
                HDR.bits   = 32;
                
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
        
        
elseif strcmp(HDR.TYPE,'MPEG'),
        % http://www.dv.co.yu/mpgscript/mpeghdr.htm
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
                HDR.FILE.OPEN = 1; 
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
                        fprintf(2,'SOPEN: corrupted MPEG file %s ',HDR.FileName);
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
                fprintf(2,'Warning SOPEN: MPEG not ready for use\n');
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
        if exist('asfopen')==2,
                HDR = asfopen(HDR,PERMISSION);
        else
                fprintf(1,'SOPEN ASF-File: Microsoft claims that its illegal to implement the ASF format.\n');
                fprintf(1,'     Anyway Microsoft provides the specification at http://www.microsoft.com/windows/windowsmedia/format/asfspec.aspx \n');
                fprintf(1,'     So, you can implement it and use it for your own purpose.\n');
        end; 
        
        
elseif strcmp(HDR.TYPE,'AIF') | strcmp(HDR.TYPE,'WAV') | strcmp(HDR.TYPE,'AVI') ,
        if strcmp(HDR.TYPE,'AIF') 
                HDR.Endianity = 'ieee-be';
        elseif  strcmp(HDR.TYPE,'WAV') | strcmp(HDR.TYPE,'AVI')  ,
                HDR.Endianity = 'ieee-le';
        end;
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
        
        if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
                HDR.FILE.OPEN = 1; 
                
                tmp = setstr(fread(HDR.FILE.FID,[1,4],'char'));        
                if ~strcmpi(tmp,'FORM') & ~strcmpi(tmp,'RIFF'),
                        fprintf(2,'Warning SOPEN AIF/WAV-format: file %s might be corrupted 1\n',HDR.FileName);
                end;
                tagsize  = fread(HDR.FILE.FID,1,'uint32');        % which size
                tagsize0 = tagsize + rem(tagsize,2); 
                tmp = setstr(fread(HDR.FILE.FID,[1,4],'char'));        
                if ~strncmpi(tmp,'AIF',3) & ~strncmpi(tmp,'WAVE',4) & ~strncmpi(tmp,'AVI ',4), % not (AIFF or AIFC or WAVE)
                        fprintf(2,'Warning SOPEN AIF/WAF-format: file %s might be corrupted 2\n',HDR.FileName);
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
                                        
                                        fprintf(2,'Error SOPEN AIF: incorrect tag size\n');
                                        return;
                                end;
                                HDR.NS   = fread(HDR.FILE.FID,1,'uint16');
                                HDR.SPR  = fread(HDR.FILE.FID,1,'uint32');
                                HDR.AS.endpos = HDR.SPR;
                                HDR.bits = fread(HDR.FILE.FID,1,'uint16');
                                HDR.GDFTYP = ceil(HDR.bits/8)*2-1; % unsigned integer of approbriate size;
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
                                                HDR.GDFTYP = 16;
                                                HDR.Cal = 1;
                                        elseif strcmpi(HDR.AIF.CompressionType,'fl64');
                                                HDR.GDFTYP = 17;
                                                HDR.Cal = 1;
                                        elseif strcmpi(HDR.AIF.CompressionType,'alaw');
                                                HDR.GDFTYP = 2;
                                                HDR.AS.bpb = HDR.NS;
                                                %HDR.FILE.TYPE = 1;
                                                fprintf(2,'Warning SOPEN AIFC-format: data not scaled because of CompressionType ALAW\n');
                                                HDR.FLAG.UCAL = 1;
                                        elseif strcmpi(HDR.AIF.CompressionType,'ulaw');
                                                HDR.GDFTYP = 2;
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
                                                fprintf(2,'Warning SOPEN AIFC-format: CompressionType %s is not supported\n', HDR.AIF.CompressionType);
                                        end;
                                end;	
                                
                        elseif strcmpi(tag,'SSND');
                                HDR.AIF.offset   = fread(HDR.FILE.FID,1,'int32');
                                HDR.AIF.blocksize= fread(HDR.FILE.FID,1,'int32');
                                
                                %tmp = (tagsize-8)/HDR.AS.bpb;
                                if 0,tmp~=HDR.SPR,
                                        fprintf(2,'Warning SOPEN AIF: Number of samples do not fit %i vs %i\n',tmp,HDR.SPR);
                                end;
                                
                                HDR.HeadLen = filepos+8; 
                                %HDR.AIF.sounddata= fread(HDR.FILE.FID,tagsize-8,'uint8');
                                
                        elseif strcmpi(tag,'FVER');
                                if tagsize<4, 
                                        fprintf(2,'Error SOPEN WAV: incorrect tag size\n');
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
                                        HDR.Off = 0;
                                        HDR.Cal = 2^(1-8*ceil(HDR.bits/8));
                                        if HDR.bits<=8,
                                                HDR.GDFTYP = 0; 
                                                HDR.Off =  1;
                                                %HDR.Cal = HDR.Cal*2;
                                        elseif HDR.bits<=16,
                                                HDR.GDFTYP = 3; 
                                        elseif HDR.bits<=24,
                                                HDR.GDFTYP = 255+24; 
                                        elseif HDR.bits<=32,
                                                HDR.GDFTYP = 5; 
                                        end;
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
                                HDR.RIFF.FACT = setstr(tmp);
                                
                        elseif strcmpi(tag,'disp');
                                if tagsize<8, 
                                        fprintf(2,'Error SOPEN WAV: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.DISP = setstr(tmp);
                                if ~all(tmp(1:8)==[0,1,0,0,0,0,1,1])
                                        HDR.RIFF.DISPTEXT = setstr(tmp(5:length(tmp)));
                                end;
                                
                        elseif strcmpi(tag,'list');
                                if tagsize<4, 
                                        fprintf(2,'Error SOPEN WAV: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c]  = fread(HDR.FILE.FID,[1,tagsize],'char');
                                %HDR.RIFF.list = setstr(tmp);
                                
                                if ~isfield(HDR,'RIFF');
                                        HDR.RIFF.N1 = 1;
                                elseif ~isfield(HDR.RIFF,'N');
                                        HDR.RIFF.N1 = 1;
                                else
                                        HDR.RIFF.N1 = HDR.RIFF.N1+1;
                                end;
                                
                                tag = setstr(tmp(1:4));
                                val = setstr(tmp(5:length(tmp)));
                                if strcmp(tag,'INFO'),
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
                                        fprintf(2,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.movi = setstr(tmp);
                                
                        elseif strcmp(tag,'idx1');
                                if tagsize<4, 
                                        fprintf(2,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.idx1 = setstr(tmp);
                                
                        elseif strcmpi(tag,'junk');
                                if tagsize<4, 
                                        fprintf(2,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.junk = setstr(tmp);
                                
                        elseif strcmpi(tag,'MARK');
                                if tagsize<4, 
                                        fprintf(2,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.MARK = setstr(tmp);
                                
                        elseif strcmpi(tag,'AUTH');
                                if tagsize<4, 
                                        fprintf(2,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.AUTH = setstr(tmp);
                                
                        elseif strcmpi(tag,'NAME');
                                if tagsize<4, 
                                        fprintf(2,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.NAME = setstr(tmp);
                                
                        elseif strcmpi(tag,'afsp');
                                if tagsize<4, 
                                        fprintf(2,'Error SOPEN AVI: incorrect tag size\n');
                                        return;
                                end;
                                [tmp,c] = fread(HDR.FILE.FID,[1,tagsize],'uchar');
                                HDR.RIFF.afsp = setstr(tmp);
                                
                        elseif ~isempty(tagsize)
                                fprintf(HDR.FILE.stderr,'Warning SOPEN AIF/WAV: unknown TAG: %s(%i) \n',tag,tagsize);
                                [tmp,c] = fread(HDR.FILE.FID,[1,min(100,tagsize)],'uchar');
                                fprintf(HDR.FILE.stderr,'%s\n',char(tmp));
                        end;
                        
                        if ~isempty(tagsize)
                                fseek(HDR.FILE.FID,filepos+tagsize0,'bof');
                        end;
                        [tmp,c] = fread(HDR.FILE.FID,[1,4],'char');
                end;
                
                if ~isfield(HDR,'HeadLen')
                        fprintf(2,'Warning SOPEN AIF/WAV: missing data section\n');
                else
                        fseek(HDR.FILE.FID, HDR.HeadLen,'bof');
                end;
                
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
                        
                        HDR.GDFTYP = ceil(HDR.bits/8)*2-1; % unsigned integer of appropriate size;
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
                                HDR.GDFTYP = 0; 
                                HDR.Off =  1;
                                %HDR.Cal = HDR.Cal*2;
                        elseif HDR.bits<=16,
                                HDR.GDFTYP = 3; 
                        elseif HDR.bits<=24,
                                HDR.GDFTYP = 255+24; 
                        elseif HDR.bits<=32,
                                HDR.GDFTYP = 5; 
                        end;
                        
                        HDR.AS.bpb = HDR.NS * ceil(HDR.bits/8);
                        tagsize = HDR.SPR*HDR.AS.bpb;
                        HDR.Dur = HDR.SPR/HDR.SampleRate;
                        HDR.AS.endpos = HDR.SPR;
                        
                        fwrite(HDR.FILE.FID,'data','char');	
                        HDR.WAV.posis=[4,ftell(HDR.FILE.FID)];
                        fwrite(HDR.FILE.FID,tagsize,'uint32');	
                        
                        if rem(tagsize,2)
                                fprintf(2,'Error SOPEN WAV: data section has odd number of samples.\n. This violates the WAV specification\n');
                                fclose(HDR.FILE.FID);
                                HDR.FILE.OPEN = 0;
                                return;  
                        end;
                        
                        HDR.HeadLen = ftell(HDR.FILE.FID);
                end;
                
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
        
        HDR.EVENT.N    = 0;
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
                HDR.datatype = 'int16';
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
        
        tmp = fread(HDR.FILE.FID,[4,HDR.EVENT.N],'uchar');
        HDR.EGI.eventcode = reshape(tmp,[4,HDR.EVENT.N])';
        HDR.EVENT.TYP = HDR.EGI.eventcode*(2.^[24;16;8;0]);
        
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
        fprintf(2,'Warning SOPEN: Implementing Nicolet TEAM file format not completed yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
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
        HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
        
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
        tag = fread(HDR.FILE.FID,1,'uint32');
        while(~feof(HDR.FILE.FID)),
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
                                if exist('OCTAVE_VERSION')<5, 	   
                                        ev{ev_cnt} = tmp2;
                                end;
                                HDR.EVENT.POS(ev_cnt) = tmp(1) + (cnt-1)*128;
                                HDR.EVENT.TYP(ev_cnt) = ev_code;
                        end;
                        fseek(HDR.FILE.FID,4*(110-HDR.EVENT.N)+2*nchans*block_size,0);
                        tag = fread(HDR.FILE.FID,1,'uint32');
                else
                        tmp = fread(HDR.FILE.FID,3,'uint16');
                        nchans = tmp(2); %fread(HDR.FILE.FID,1,'uint16');
                        block_size = 2^tmp(3); %fread(HDR.FILE.FID,1,'uint16');
                        
                        fseek(HDR.FILE.FID,62+4*(110-HDR.EVENT.N)+2*nchans*block_size,0);
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
        
        HDR.Cal = 1;
        
        
elseif strcmp(HDR.TYPE,'RG64'),
        if strcmpi(HDR.FILE.Ext,'rhf'),
                FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext]);
        elseif strcmp(HDR.FILE.Ext,'rdf'),
                FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext(1),'h',HDR.FILE.Ext(3)]);
        elseif strcmp(HDR.FILE.Ext,'RDF'),
                FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext(1),'H',HDR.FILE.Ext(3)]);
        end;
        HDR.FILE.FID = fopen(FILENAME,PERMISSION,'ieee-le');
        
        HDR.FILE.OPEN=1;
        HDR.IDCODE=char(fread(HDR.FILE.FID,[1,4],'char'));	%
        if ~strcmp(HDR.IDCODE,'RG64') 
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
        
        if strcmpi(HDR.FILE.Ext,'rdf'),
                FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext]);
        elseif strcmp(HDR.FILE.Ext,'rhf'),
                FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext(1),'d',HDR.FILE.Ext(3)]);
        elseif strcmp(HDR.FILE.Ext,'RHF'),
                FILENAME=fullfile(HDR.FILE.Path,[HDR.FILE.Name,'.',HDR.FILE.Ext(1),'D',HDR.FILE.Ext(3)]);
        end;
        
        HDR.FILE.FID=fopen(FILENAME,'rb','ieee-le');
        if HDR.FILE.FID<0,
                fprintf(2,'\nError LOADRG64: %s not found\n',FILENAME); 
                return;
        end;
        
        HDR.Cal = diag(AMPF(HDR.InChanSelect));
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal,HDR.NS+1,HDR.NS);
        HDR.FILE.POS = 0; 
        
        
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
                        if findstr(field,'SAMPLE RATE');
                                [tmp1,tmp2] = strtok(value,'=');
                                HDR.SampleRate = str2double(tmp1);
                        elseif findstr(field,'DATA CHANNELS');
                                HDR.NS = str2double(value);
                        elseif findstr(field,'START TIME');
                                Time = value;
                        elseif findstr(field,'DATA FILE');
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
                if tmp == 0, HDR.GDFTYP = 4; 		% streamer format (data are raw data in WORD=UINT16)
                elseif tmp == 1, HDR.GDFTYP = 16; 	% Universal Format 1 (FLOAT)
                elseif tmp == 2, HDR.GDFTYP = 17; 	% Universal Format 2 (DOUBLE)
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
                                fprintf(2,'Error SOPEN DDF: error in header of file %s\n',HDR.FileName);
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
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS = 0;
                
                fid = HDR.FILE.FID;
                z = fgetl(fid);
                tmpfile = strtok(z,' /');
                if ~strcmpi(file,tmpfile),
                        fprintf(2,'Warning: RecordName %s does not fit filename %s\n',tmpfile,file);
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
                
                for k=1:HDR.NS,
                        z = fgetl(fid);
                        [HDR.FILE.DAT,z]=strtok(z);
                        for k0 = 1:7,
                                [tmp,z] = strtok(z);
                                if k0 == 2,  
                                        % EC13*.HEA files have special gain values like "200(23456)/uV". 
                                        [tmp, tmp1] = strtok(tmp,' ()');
                                end;
                                A(k0) = str2double(tmp); 
                        end;
                        HDR.Label(k,1:length(z)) = z; 
                        dformat(k,1) = A(1);         % format; 
                        HDR.gain(k,1) = A(2);              % number of integers per mV
                        bitres(k,1) = A(3);            % bitresolution
                        HDR.zerovalue(k,1)  = A(4);         % integer value of ECG zero point
                        HDR.firstvalue(1,k) = A(5);        % first integer value of signal (to test for errors)
                end;
                HDR.Calib = sparse([HDR.zerovalue(:).';eye(HDR.NS)]*diag(1./HDR.gain(:)));
                
                z   = char(fread(fid,[1,inf],'char'));
                ix1 = [findstr('AGE:',upper(z))+4, findstr('AGE>:',upper(z))+5];
                if ~isempty(ix1),
                        [tmp,z]=strtok(z(ix1(1):length(z)));
                        HDR.Patient.Age = str2double(tmp);
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
                        %annoth = bitshift(A(i,2),-2);
                        annoth = floor(A(i,2)/4);
                        if annoth==59,
                                K = K + 1;
                                ANNOT(K) = annoth; %bitshift(A(i+3,2),-2);
                                ATRTIME(K) = (256.^[0:3])*[A(i+2,1);A(i+2,2);A(i+1,1);+A(i+1,2)];
                                i = i + 3;
                        elseif annoth==60
                                % nothing to do!
                        elseif annoth==61
                                % nothing to do!
                        elseif annoth==62
                                % nothing to do!
                        elseif annoth==63
                                hilfe = 256*rem(A(i,2),4)+A(i,1);
                                hilfe = hilfe + mod(hilfe,2);
                                i = i + hilfe / 2;
                        else
                                K = K+1;
                                ATRTIME(K) = 256*rem(A(i,2),4)+A(i,1);
                                ANNOT(K)   = annoth; %bitshift(A(i,2),-2);
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
                try
                        HDR.AS.endpos = tmp/HDR.AS.bpb;
                catch
                        fprintf(HDR.FILE.stderr,'Warning 2003 SOPEN: FTELL does not return numeric value (Octave > 2.1.52).\nHDR.AS.endpos not completed.\n');
                end;
                fseek(HDR.FILE.FID,0,'bof');
                
                HDR.InChanSelect = 1:HDR.NS;
                FLAG_UCAL = HDR.FLAG.UCAL;	
                HDR.FLAG.UCAL = 1;
                [S,HDR] = sread(HDR,1/HDR.SampleRate); % load 1st sample
                if any(S(1,:) ~= HDR.firstvalue), 
                        fprintf(2,'ERROR SOPEN MIT-ECG: inconsistency in the first values\n'); 
                end;
                HDR.FLAG.UCAL = FLAG_UCAL ;	
                fseek(HDR.FILE.FID,0,'bof');	% reset file pointer
                HDR.FILE.POS = 0;
                
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
                HDR.Calib = sparse([HDR.Off';(diag(HDR.Cal))]);
                HDR.HeadLen = 217 + HDR.NS*136;
                
        end;
        
        
elseif 0,strcmp(HDR.TYPE,'DAQ'),
        HDR = daqopen(HDR,PERMISSION,CHAN);
        
        
elseif strcmp(HDR.TYPE,'MAT4'),
        if any(PERMISSION=='r'),
                HDR.FILE.FID = fopen(HDR.FileName,'rb',HDR.MAT4.opentyp);
                
                fprintf(HDR.FILE.stderr,'Format not tested yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',PERMISSION);	
                
                HDR.FILE.OPEN = 1;
                HDR.FILE.POS = 0;
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
                        [name,c] = fread(HDR.FILE.FID,namelen,'char'); 
                        
                        if imagf, HDR.ErrNo=-1; fprintf(HDR.FILE.stderr,'Warning %s: Imaginary data not tested\n',mfilename); end;
                        if type(4)==2,
                                HDR.ErrNo=-1;
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
                        if findstr(HDR.Var(k).Name,'data_block'),
                                HDR.ADI.DB(str2double(HDR.Var(k).Name(11:length(HDR.Var(k).Name))))=k;
                        elseif findstr(HDR.Var(k).Name,'ticktimes_block'),
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
                        HDR.GDFTYP = 17;	% double
                        HDR.AS.bpb = HDR.NS * 8;
                elseif tmp == 2, 
                        HDR.GDFTYP = 16;	% float
                        HDR.AS.bpb = HDR.NS * 4;
                elseif tmp == 3, 
                        HDR.GDFTYP =  3;	% int16
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
                        fprintf(2,'Error SOPEN-W CFWB: number of channels HDR.NS undefined.\n');
                        return;
                end;
                if ~isfield(HDR,'SPR'),
                        HDR.SPR = 0; 	% Unknown - Value will be fixed when file is closed. 
                else
                        HDR.SPR = HDR.SPR(1);
                end;
                if ~isfield(HDR,'SampleRate'),
                        HDR.SampleRate = 1; 	% Unknown - Value will be fixed when file is closed. 
                        fprintf(2,'Warning SOPEN-W CFWB: samplerate undefined.\n');
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
                if HDR.GDFTYP == 17;	% doubletmp == 1, 
                        tmp = 1;
                        HDR.AS.bpb = HDR.NS * 8;
                        HDR.Cal = ones(HDR.NS,1);
                        HDR.Off = zeros(HDR.NS,1);
                elseif HDR.GDFTYP == 16;	% float
                        tmp = 2; 
                        HDR.AS.bpb = HDR.NS * 4;
                        HDR.Cal = ones(HDR.NS,1);
                        HDR.Off = zeros(HDR.NS,1);
                elseif HDR.GDFTYP == 3;	% int16
                        tmp = 3;
                        HDR.AS.bpb = HDR.NS * 2;
                end;
                HDR.PhysMax = repmat(NaN,HDR.NS,1);
                HDR.PhysMin = repmat(NaN,HDR.NS,1);
                if ~isfield(HDR,'Cal'),
                        fprintf(2,'Warning SOPEN-W CFWB: undefined scaling factor\n');			
                        HDR.Cal = ones(HDR.NS,1);
                end;
                if ~isfield(HDR,'Off'),
                        fprintf(2,'Warning SOPEN-W CFWB: undefined offset\n');			
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
                        fprintf(2,'Error SOPEN-W CFWB: could not open file %s .\n',HDR.FileName);
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
                        fprintf(2,'Error SOPEN CFWB: size of header1 does not fit in file %s\n',HDR.FileName);
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
                        fprintf(2,'Error SOPEN CFWB: size of header2 does not fit in file %s\n',HDR.FileName);
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
                HDR.Sampling_order   = str2double(fgetl(HDR.FILE.FID));  	% 13
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
                        fprintf(2,'Warning LOADSIG in %s: Footer not found\n',  HDR.FileName);  
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
                        HDR.T0(4) = str2double(tmp(1:2));
                        HDR.T0(5) = str2double(tmp(3:4));
                        HDR.T0(6) = str2double([tmp(5:6),'.',tmp(7:9)]);
                end;
        end;
        
        
elseif strncmp(HDR.TYPE,'BrainVision',3),
        %%%% #### FIX ME ####
        if any(PERMISSION=='r'),
                if any(exist('read_brainvision_vhdr')==[2:6]), 
                        hdr = read_brainvision_vhdr(HDR.FileName,1,1);
                        HDR.SampleRate = hdr.Fs;
                        HDR.NS = hdr.nchan; 
                        HDR.NRec = 1; 
                        HDR.FLAG.TRIGGERED = 0;
                        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
                else
                        fprintf(2,'Error SOPEN (EEProbe): reading function READ_BRAINVISION_VHDR not available\n');
                end
                
                fprintf(HDR.FILE.stderr,'Warning: Implementing Format %s not completed yet. \n',HDR.TYPE);	
                return;
        end
        
        
elseif strncmp(HDR.TYPE,'CTF',3),
        %%%% #### FIX ME ####
        if any(PERMISSION=='r'),
                if any(exist('read_ctf_res4')==[2,3,6]), 
                        hdr = read_ctf_res4(HDR.FileName,1,1);
                        HDR.SampleRate = hdr.Fs;
                        HDR.NS = hdr.nchan; 
                        HDR.NRec = 1; 
                        HDR.FLAG.TRIGGERED = 0;
                        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
                else
                        fprintf(2,'Error SOPEN (EEProbe): reading function READ_CTF_RES4 not available\n');
                end
                
                fprintf(HDR.FILE.stderr,'Warning: Implementing Format %s not completed yet. \n',HDR.TYPE);	
                return;
        else
                
        end
        
        
elseif strncmp(HDR.TYPE,'EEProbe',3),
        %%%% #### FIX ME ####
        if any(PERMISSION=='r'),
                if any(exist('read_eep_cnt')==[2,3,6]), 
                        hdr = read_eep_cnt(HDR.FileName,1,1);
                        HDR.SampleRate = hdr.Fs;
                        HDR.NS = hdr.nchan; 
                        HDR.NRec = 1; 
                        HDR.FLAG.TRIGGERED = 0;
                        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
                else
                        fprintf(2,'Error SOPEN (EEProbe): reading function READ_EEP_CNT not available\n');
                end
                
                fprintf(HDR.FILE.stderr,'Warning: Implementing Format %s not completed yet. \n',HDR.TYPE);	
                return;
        end
        
        
elseif strncmp(HDR.TYPE,'FIF',3),
        %%%% #### FIX ME ####
        if any(exist('rawdata')==[3,6]),
                rawdata('any',HDR.FileName);
                HDR.FILE.FID = 1;
                HDR.SampleRate = rawdata(HDR.FileName,'sf');
                HDR.T0 = rawdata(HDR.FileName,'t');
                HDR.SPR = rawdata(HDR.FileName,'samples');
                [HDR.MinMax,HDR.Cal] = rawdata(HDR.FileName,'range');
                [HDR.Label, type, number] = channames(headerfile);
                HDR.Calib = 1; 
        else
                fprintf(HDR.FILE.stderr,'ERROR SOPEN (FIF): Cannot open FIF-file, because rawdata.mex not installed. \n',HDR.NS);
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
                HDR.VERTEX.COORD = fread(HDR.FILE.FID,3*HDR.SURF.N,'float32');
                
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
        
        
elseif strcmp(HDR.TYPE,'XML-UTF16'),
        if any(PERMISSION=='r'),
                fid = fopen(HDR.FileName,'rb','ieee-le');
                magic = fread(fid,2,'uint8');
                HDR.XML = char(fread(fid,[1,inf],'uint16'));
                fclose(fid);
                try,
                        %HDR.XML = xml_parser(HDR.XML);
                        HDR.XML = xmltree(char(HDR.XML));
                        HDR.XML = convert(HDR.XML);
                        HDR.TYPE= 'XML';
                catch
                end;
        end;
        fprintf(HDR.FILE.stderr,'Warning SOPEN (XML): Implementation of XML-Files not completed.\n',HDR.NS);
        return;
        
        
elseif strcmp(HDR.TYPE,'XML-UTF8'),
        if any(PERMISSION=='r'),
                fid = fopen(HDR.FileName,'rt','ieee-le');
                HDR.XML = char(fread(fid,[1,inf],'char'));
                fclose(fid);
                try,
                        %HDR.XML = xml_parser(HDR.XML);
                        HDR.XML = xmltree(HDR.XML);
                        HDR.XML = convert(HDR.XML);
                        HDR.TYPE= 'XML';
                catch
                end;
        end;
        fprintf(HDR.FILE.stderr,'Warning SOPEN (XML): Implementation of XML-Files not completed.\n',HDR.NS);
        return;
        
elseif strcmp(HDR.TYPE,'unknown'),
        fprintf(HDR.FILE.stderr,'ERROR SOPEN: File %s could not be opened - unknown type.\n',HDR.FileName);
        HDR.FILE.FID = -1;
        return;
        
else
        %fprintf(2,'SOPEN does not support your data format yet. Contact <a.schloegl@ieee.org> if you are interested in this feature.\n');
        HDR.FILE.FID = -1;	% this indicates that file could not be opened. 
        return;
end;

if any(PERMISSION=='r');
        HDR.Calib = full(HDR.Calib);	% Octave can not index sparse matrices
        if exist('ReRefMx')==1,
                % HDR.SIE.ChanSelect = 1:size(ReRefMx,2);         
                [i,j,v] = find(ReRefMx);
                if any(i) > HDR.NS,
                        fprintf(HDR.FILE.stderr,'ERROR: size of ReRefMx [%i,%i] exceeds Number of Channels (%i)\n',size(ReRefMx),HDR.NS);
                        fclose(HDR.FILE.FID); 
                        HDR.FILE.FID = -1;	
                        return;
                end;
                HDR.Calib = HDR.Calib * sparse(i,j,v,HDR.NS,size(ReRefMx,2));
                HDR.InChanSelect = find(any(HDR.Calib(2:end,:),2));
        else 
                if CHAN==0,
                        CHAN = 1:HDR.NS;
                end;
                HDR.InChanSelect = CHAN(:);
                HDR.Calib = HDR.Calib(:,CHAN);
        end;
        %HDR.Calib = sparse(HDR.Calib);
        
        if any(HDR.InChanSelect > HDR.NS)
                fprintf(HDR.FILE.stderr,'ERROR: selected channels exceed Number of Channels %i\n',HDR.NS);
                fclose(HDR.FILE.FID); 
                HDR.FILE.FID = -1;	
                return;
        end;
end;
