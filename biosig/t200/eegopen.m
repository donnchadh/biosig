function [HDR,H1,h2]=eegopen(arg1,PERMISSION,CHAN,MODE,TYPE,arg5,arg6)
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

%	$Revision: 1.3 $
%	$Id: eegopen.m,v 1.3 2003-04-25 13:59:28 schloegl Exp $
%	(C) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


if nargin<2, PERMISSION = 'r'; end; 
if nargin<3, CHAN = 0; end; 
if nargin<4, MODE=''; end;
if nargin<5, TYPE='unknown'; end;

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

fid = fopen('HDR.FileName','r');
if fid>0,
	[s,c]=fread(fid,8,'uint8');
	if s=='0       '; 
		TYPE='EDF';
	elseif all(s==[255,abs('BIOSEMI')]); 
		TYPE='EDF';
	elseif s(1)==207; 
		TYPE='BKR';
	elseif s(1:min(3,c))=='GDF'; 
		TYPE='GDF';
	elseif s(1:min(4,c))=='RG64'; 
		TYPE='RG64';
	elseif s(1:min(6,c))=='IAvSFo'; 
		TYPE='SIG';
	elseif s=='Version '; 
		TYPE='CNT';
	elseif any(s(4)==(2:7)) & all(s(1:3)==0); % [int32] 2...7
		TYPE='EGI';
	elseif s=='ISHNE1.0';	% ISHNE Holter standard output file.
		TYPE='ISHNE';
	elseif s(1:min(4,c))=='rhdE';	% Holter Excel 2 file, not supported yet. 
		TYPE='rhdE';          
	else
		TYPE='unknown';
	end;
	fclose(fid);
end;

if strcmp(TYPE,'unknown')	, 
        TYPE = upper(FileExt(2:length(FileExt)));
        
        %%% EDF format
        if     strcmp(TYPE,'REC'), TYPE='EDF';
        elseif strcmp(TYPE,'EDF'), TYPE='EDF';

        elseif strcmp(TYPE,'BDF'), TYPE='BDF';

        elseif strcmp(TYPE,'GDF'), TYPE='GDF';
                
        %%% Neuroscan Format        
        elseif strcmp(TYPE,'AVG'), TYPE='AVG';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
        elseif strcmp(TYPE,'COH'), TYPE='COH';
		error(sprintf('EEGOPEN: filetype %s not implemented, yet.',TYPE));
        elseif strcmp(TYPE,'CSA'), TYPE='COH';
		error(sprintf('EEGOPEN: filetype %s not implemented, yet.',TYPE));
        elseif strcmp(TYPE,'EEG'), TYPE='EEG';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
        elseif strcmp(TYPE,'CNT'), TYPE='CNT';
        elseif strcmp(TYPE,'SET'), TYPE='SET';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
        elseif strcmp(TYPE,'AST'), TYPE='AST';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
                
        %%% BKR Format        
	elseif strcmp(TYPE,'BKR'), TYPE='BKR';
	elseif strcmp(TYPE,'SPB'), TYPE='BKR';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
	elseif strcmp(TYPE,'SAB'), TYPE='BKR';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
	elseif strcmp(TYPE,'SRB'), TYPE='BKR';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
	elseif strcmp(TYPE,'MNB'), TYPE='BKR';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
	elseif strcmp(TYPE,'STB'), TYPE='BKR';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
                
        % MIT-ECG / Physiobank format
        elseif strcmp(TYPE,'hea'), TYPE='MIT';
        elseif strcmp(TYPE,'atr'), TYPE='MIT';
                
        % other formates        
        elseif strcmp(TYPE,'LDR'), TYPE='LDR';

        elseif strcmp(TYPE,'DAT'), TYPE='DAT';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
                
        elseif strcmp(TYPE,'SIG'), TYPE='SIG';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
                
        elseif strcmp(TYPE(1:2),'DA'), TYPE='DA_';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
                
        elseif strcmp(TYPE([1,3]),'RF'), TYPE='RG64';
		warning(sprintf('EEGOPEN: filetype %s not tested, yet.',TYPE));
                
	else
			
        end; 
end;

HDR.TYPE = TYPE;

if strcmp(TYPE,'EDF'),
        HDR = sdfopen(HDR,PERMISSION,CHAN);
        HDR.TYPE = TYPE;	% restore type information
	HDR.FLAG.TRIGGERED = 0;	% Trigger Flag
        
elseif strcmp(TYPE,'BDF'),
        HDR = sdfopen(HDR,PERMISSION,CHAN);
        HDR.TYPE = TYPE;	% restore type information
	HDR.FLAG.TRIGGERED = 0;	% Trigger Flag
        
elseif strcmp(TYPE,'GDF'),
        HDR = sdfopen(HDR.FileName,PERMISSION,CHAN);
        HDR.TYPE = TYPE;	% restore type information
        
elseif strcmp(TYPE,'BKR'),
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
        
elseif strcmp(TYPE,'CNT'),
	if strcmp(PERMISSION,'r'),
	        HDR = cntopen(HDR,'r',CHAN);
	else
		fprintf(HDR.FILE.stderr,'PERMISSION %s not supported\n',PERMISSION);	
        end;
        
elseif strcmp(TYPE,'EEG'),
	if strcmp(PERMISSION,'r'),
	        HDR = cntopen(HDR,'r',CHAN);
	else
		fprintf(HDR.FILE.stderr,'PERMISSION %s not supported\n',PERMISSION);	
        end;
        
elseif strcmp(TYPE,'EGI'),
	if strcmp(PERMISSION,'r'),
	        HDR = openegi(HDR,'r',CHAN);
	else
		fprintf(HDR.FILE.stderr,'PERMISSION %s not supported\n',PERMISSION);	
        end;
        
elseif strcmp(TYPE,'LDR'),
        HDR = openldr(HDR,PERMISSION);      
        
elseif strcmp(TYPE,'ISHNE'),
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
		end;

		HDR.PacemakerCode = fread(HDR.FILE.FID,1,'int16');		
		HDR.TypeOfRecorder = fread(HDR.FILE.FID,40,'char');		
		HDR.SampleRate = fread(HDR.FILE.FID,1,'int16');		
		HDR.Proprietary_of_ECG = fread(HDR.FILE.FID,80,'char');		
		HDR.Copyright = fread(HDR.FILE.FID,80,'char');		
		HDR.reserved1 = fread(HDR.FILE.FID,80,'char');		
		if ftell(HDR.FILE.FID)~=HDR.offset_variable_legnth_block,
			fprintf(HDR.FILE.stderr,'ERROR: length of fixed header does not fit %i %i \n',ftell(HDR.FILE.FID),HDR.offset_variable_length_block);
			return;
		end;
		HDR.VariableHeader=fread(HDR.FILE.FID,HDR.variable_length_block,'char);	
		if ftell(HDR.FILE.FID)~=HDR.HeadLen,
			fprintf(HDR.FILE.stderr,'ERROR: length of variable header does not fit %i %i \n',ftell(HDR.FILE.FID),HDR.HeadLen);
			return;
		end;

		if CHAN==0,		
			HDR.SIE.InChanSelect = 1:HDR.NS;
		elseif all(CHAN>0 & CHAN<=HDR.NS),
			HDR.SIE.InChanSelect = CHAN;
		else
			fprintf(HDR.FILE.stderr,'ERROR: selected channels are not positive or exceed Number of Channels %i\n',HDR.NS);
			return;
		end;
		
		HDR.Cal = eye(AmplitudeResolution(HDR.InChanSelect))/1000;
		HDR.PhysDim = 'uV';
		HDR.AS.bpb = 2*HDR.NS;
		HDR.AS.endpos = 8+2+512+HDR.variable_length_block+HDR.NS*2*HDR.SPR;
		
	else
		fprintf(HDR.FILE.stderr,'PERMISSION %s not supported\n',PERMISSION);	
	end;			

else
	fprintf(HDR.FILE.stderr,'Format not supported yet. \nFor more information contact <a.schloegl@ieee.org> Subject: Biosig/Dataformats \n',PERMISSION);	

end;

