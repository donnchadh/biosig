function [EDF,H1,h2]=sdfopen(arg1,arg2,arg3,arg4,arg5,arg6)
% Opens EDF/GDF/SDF files for reading and writing. 
% The EDF format is specified in [1], GDF in [2].
% SDF is the SIESTA convention about polygraphic recordings (see [3], pp.8-9). 
% SDF used the EDF format.
%
% EDF=sdfopen(Filename,'r' [,CHAN [,Mode [,TSR]]]);
% [S,EDF]=sdfread(EDF, NoR, StartPos)
%
% CHAN defines the selected Channels or a re-referencing matrix
%
% Mode=='SIESTA' indicates that #1 - #6 is rereferenced to (M1+M2)/2           
% Mode=='AFIR' indicates that Adaptive FIR filtering is used for ECG removing
%			Implements Adaptive FIR filtering for ECG removal in EDF/GDF-tb.
% 			based on the Algorithm of Mikko Koivuluoma <k7320@cs.tut.fi>
%                 A delay of EDF.AFIR.delay number of samples has to be considered. 
% Mode=='SIESTA+AFIR' for both
% Mode=='UCAL' [default]
%                 indicates that no calibration (re-scaling) to Physical Dim. is used. 
%                 The output are 16bit interger numbers. 'UCAL' overrides 'SIESTA'
% Mode=='RAW' One column represents one EDF-block
% Mode=='Notch50Hz' Implements a simple FIR-notch filter for 50Hz
% Mode=='RECG' Implements ECG minimization with regression analysis
% Mode=='TECG' Implements ECG minimization with template removal (Test status)
% Mode=='HPF0.0Hz'  Implements a high-pass filter (with the zero at z=+1, i.e. a differentiator 
%                     In this case a Notch-filter and/or sub-sampling is recommended. 
% Mode=='TAUx.yS' compensates time-constant of x.y seconds
% Mode=='EOG[hvr]' produces HEOG, VEOG and/or REOG output (CHAN not considered)
% 
% Mode=='OVERFLOW' overflow detection
% Mode=='FailingElectrodeDetector' using optimized threshold based AUC for FED & OFC
% Mode=='Units_Blocks' requests the arguments in SDFREAD in Blocks [default is seconds]
%
% TSR [optional] is the target sampling rate
%         Currently only downsampling from 256 and 200 to 100Hz is supported.  
%         The details are described in the appendix of [4].
%
% EDF.ErrNo~=0  indicates that an error occured.
% For compatibility to former versions, 
%    EDF.FILE.FID = -1 indicates that file has not been opened.
%
% Opening of an EDF/SDF File for writing
% [EDF]=sdfopen(EDF,'w') is equal to  
% [EDF]=sdfopen(EDF.FileName,'w',EDF.Dur,EDF.SampleRate);
% At least EDF.FileName, EDF.NS, EDF.Dur and EDF.EDF.SampleRate must be defined
% 
% 
% See also: fopen, SDFREAD, SWRITE, SCLOSE, SSEEK, SREWIND, STELL, SEOF


% References: 
% [1] Bob Kemp, Alpo Värri, Agostinho C. Rosa, Kim D. Nielsen and John Gade
%     A simple format for exchange of digitized polygraphic recordings.
%     Electroencephalography and Clinical Neurophysiology, 82 (1992) 391-393.
% see also: http://www.medfac.leidenuniv.nl/neurology/knf/kemp/edf/edf_spec.htm
%
% [2] Alois Schlögl, Oliver Filz, Herbert Ramoser, Gert Pfurtscheller
%     GDF - A GENERAL DATAFORMAT FOR BIOSIGNALS
%     Technical Report, Department for Medical Informatics, Universtity of Technology, Graz (2004)
% see also: http://www.dpmi.tu-graz.ac.at/~schloegl/matlab/eeg/gdf4/TR_GDF.PDF
%
% [3] The SIESTA recording protocol. 
% see http://www.ai.univie.ac.at/siesta/protocol.html
% and http://www.ai.univie.ac.at/siesta/protocol.rtf 
%
% [4] Alois Schlögl
%     The electroencephalogram and the adaptive autoregressive model: theory and applications. 
%     (ISBN 3-8265-7640-3) Shaker Verlag, Aachen, Germany.
% see also: "http://www.shaker.de/Online-Gesamtkatalog/Details.idc?ISBN=3-8265-7640-3"


% Testing state
%
% (1) reads header information and writes the header; can be used to check SDFOPEN or for correcting headerinformation
% EDF=sdfopen(EDF,'r+b'); EDF=sdfclose(EDF); 
% 
% (2a) Minimal requirements for opening an EDF-File
% EDF.FileName='file.edf'; % define filename
% EDF.NS = 5; % fix number of channels
% EDF=sdfopen(EDF,'wb');
%     write file
%     define header somewhen before 
% EDF=sdfclose(EDF); % writes the corrected header information
% 
% (2b) Minimal requirements for opening an EDF-File
% EDF=sdfopen('file.edf','wb',N); % N number of channels
%      .. do anything, e.g define header
% EDF=sdfopen(EDF,'w+b'); % writes Header information
%
%
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

% EDF.WarnNo   1: ascii(0) in 1st header
%              2: ascii(0) in 2nd header
%              4: invalid SPR
%              4: invalid samples_per_record-values
%              8: date not in EDF-format (tries to guess correct date, see also E2)
%             16: invalid number_of-channels-value
%             32: invalid value of the EDF header length
%             64: invalid value of block_duration
%            128: Polarity of #7 probably inverted  
%
% EDF.ErrNo    1: first 8 bytes different to '0       ', this violates the EDF spec.
%              2: Invalid date (unable to guess correct date)
%              4: Incorrect date information (later than actual date) 
%             16: incorrect filesize, Header information does not match actual size

%	$Revision: 1.34 $
%	$Id: sdfopen.m,v 1.34 2004-12-09 16:51:31 schloegl Exp $
%	(C) 1997-2002, 2004 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

if nargin<2, 
        arg2='rb'; 
elseif ~any(arg2=='b');
        arg2= [arg2,'b']; % force binary open. 
end;

if isstruct(arg1) 
        EDF=arg1; 
        FILENAME=EDF.FileName;
else
        FILENAME=arg1;
        fprintf(2,'Warning SDFOPEN: the use of SDFOPEN is discouraged (SDFOPEN might disappear); please use SOPEN instead.\n');
end;

H1idx = [8 80 80 8 8 8 44 8 8 4];
H2idx = [16 80 8 8 8 8 8 80 8 32];

%%%%% Define Valid Data types %%%%%%
%GDFTYPES=[0 1 2 3 4 5 6 7 16 17 255+(1:64) 511+(1:64)];
GDFTYPES=[0 1 2 3 4 5 6 7 16 17 255+[1 12 22 24] 511+[1 12 22 24]];

%%%%% Define Size for each data type %%%%%
GDFTYP_BYTE=zeros(1,512+64);
GDFTYP_BYTE(256+(1:64))=(1:64)/8;
GDFTYP_BYTE(512+(1:64))=(1:64)/8;
GDFTYP_BYTE(1:18)=[1 1 1 2 2 4 4 8 8 4 8 0 0 0 0 0 4 8]';

%EDF.GDFTYP.TEXT={'char','int8','uint8','int16','uint16','int32','uint32','int64','uint64','float32','float64'};
%GDFTYP_BYTE=[1 1 1 2 2 4 4 8 8 4 8 0 0 0 0 0 4 8]';
%GDFTYPES=[0 1 2 3 4 5 6 7 16 17];

EDF.ErrNo = 0;

if strcmp(EDF.TYPE,'EDF')
        EDF.VERSION='0       ';
elseif strcmp(EDF.TYPE,'GDF') 
        EDF.VERSION='GDF     ';
elseif strcmp(EDF.TYPE,'BDF'),
        EDF.VERSION=[char(255),'BIOSEMI'];
end;

%%%%%%% ============= READ ===========%%%%%%%%%%%%
if any(arg2=='r'), 
        
[EDF.FILE.FID,MESSAGE]=fopen(FILENAME,arg2,'ieee-le');          
%EDF.FILE.FID=fid;

if EDF.FILE.FID<0 
        %fprintf(EDF.FILE.stderr,'Error SDFOPEN: %s %s\n',MESSAGE,FILENAME);  
        H1=MESSAGE; H2=FILENAME;
        EDF.ErrNo = [32,EDF.ErrNo];
	return;
end;
EDF.FILE.OPEN = 1 + any(arg2=='+'); 
EDF.FileName = FILENAME;

PPos=min([max(find(FILENAME=='.')) length(FILENAME)+1]);
SPos=max([0 find(FILENAME==filesep)]);
EDF.FILE.Ext = FILENAME(PPos+1:length(FILENAME));
EDF.FILE.Name = FILENAME(SPos+1:PPos-1);
if SPos==0
	EDF.FILE.Path = pwd;
else
	EDF.FILE.Path = FILENAME(1:SPos-1);
end;
EDF.FileName = [EDF.FILE.Path filesep EDF.FILE.Name '.' EDF.FILE.Ext];
%EDF.datatype = 'EDF';

%%% Read Fixed Header %%%
[tmp,count]=fread(EDF.FILE.FID,184,'uchar');     %
if count<184,
        EDF.ErrNo = [64,EDF.ErrNo];
        return;
end;
H1=setstr(tmp');     %


EDF.VERSION=H1(1:8);                     % 8 Byte  Versionsnummer 
if ~(strcmp(EDF.VERSION,'0       ') | all(abs(EDF.VERSION)==[255,abs('BIOSEMI')]) | strcmp(EDF.VERSION(1:3),'GDF'))
        EDF.ErrNo = [1,EDF.ErrNo];
	if ~strcmp(EDF.VERSION(1:3),'   '); % if not a scoring file, 
%	    return; 
	end;
end;
EDF.PID = deblank(H1(9:88));                  % 80 Byte local patient identification
EDF.RID = deblank(H1(89:168));                % 80 Byte local recording identification

IsGDF=strcmp(EDF.VERSION(1:3),'GDF');

if strcmp(EDF.VERSION(1:3),'GDF'),
        if 1, % if strcmp(EDF.VERSION(4:8),' 0.12'); % in future versions the date format might change. 
      		EDF.T0(1,1) = str2double( H1(168 + [ 1:4]));
		EDF.T0(1,2) = str2double( H1(168 + [ 5 6]));
        	EDF.T0(1,3) = str2double( H1(168 + [ 7 8]));
        	EDF.T0(1,4) = str2double( H1(168 + [ 9 10]));
        	EDF.T0(1,5) = str2double( H1(168 + [11 12]));
        	EDF.T0(1,6) = str2double( H1(168 + [13:16]))/100;
     	end; 
     
	if str2double(EDF.VERSION(4:8))<0.12
                tmp = setstr(fread(EDF.FILE.FID,8,'uchar')');    % 8 Byte  Length of Header
                EDF.HeadLen = str2double(tmp);    % 8 Byte  Length of Header
        else
		%EDF.HeadLen = fread(EDF.FILE.FID,1,'int64');    % 8 Byte  Length of Header
		EDF.HeadLen = fread(EDF.FILE.FID,1,'int32');    % 8 Byte  Length of Header
		tmp         = fread(EDF.FILE.FID,1,'int32');    % 8 Byte  Length of Header
        end;
        EDF.reserved1 = fread(EDF.FILE.FID,8+8+8+20,'uchar');     % 44 Byte reserved
        
        %EDF.NRec = fread(EDF.FILE.FID,1,'int64');     % 8 Byte  # of data records
        EDF.NRec = fread(EDF.FILE.FID,1,'int32');     % 8 Byte  # of data records
                   fread(EDF.FILE.FID,1,'int32');     % 8 Byte  # of data records
        if strcmp(EDF.VERSION(4:8),' 0.10')
                EDF.Dur =  fread(EDF.FILE.FID,1,'float64');    % 8 Byte  # duration of data record in sec
        else
                tmp =  fread(EDF.FILE.FID,2,'uint32');    % 8 Byte  # duration of data record in sec
                EDF.Dur =  tmp(1)./tmp(2);
        end;
        EDF.NS =   fread(EDF.FILE.FID,1,'uint32');     % 4 Byte  # of signals
else 
        tmp=(find((H1<32) | (H1>126))); 		%%% syntax for Matlab
        if ~isempty(tmp) %%%%% not EDF because filled out with ASCII(0) - should be spaces
                %H1(tmp)=32; 
                EDF.ErrNo=[1025,EDF.ErrNo];
        end;
        
        EDF.T0 = zeros(1,6);
        ErrT0=0;
        tmp = str2double( H1(168 + [ 7  8]));
        if ~isnan(tmp), EDF.T0(1) = tmp; else ErrT0 = 1; end;
        tmp = str2double( H1(168 + [ 4  5]));
        if ~isnan(tmp), EDF.T0(2) = tmp; else ErrT0 = 1; end;
        tmp = str2double( H1(168 + [ 1  2]));
        if ~isnan(tmp), EDF.T0(3) = tmp; else ErrT0 = 1; end;
        tmp = str2double( H1(168 + [ 9 10]));
        if ~isnan(tmp), EDF.T0(4) = tmp; else ErrT0 = 1; end;
        tmp = str2double( H1(168 + [12 13]));
        if ~isnan(tmp), EDF.T0(5) = tmp; else ErrT0 = 1; end;
        tmp = str2double( H1(168 + [15 16]));
        if ~isnan(tmp), EDF.T0(6) = tmp; else ErrT0 = 1; end;
        
	if any(EDF.T0~=fix(EDF.T0)); ErrT0=1; end;

        if ErrT0,
                ErrT0=0;
                EDF.ErrNo = [1032,EDF.ErrNo];
                
                tmp = H1(168 + [1:8]);
                for k = [3 2 1],
                        %fprintf(1,'\n zz%szz \n',tmp);
                        [tmp1,tmp] = strtok(tmp,' :./-');
			tmp1 = str2double([tmp1,' ']);
			
                        if isempty(tmp1)
                                ErrT0 = ErrT0 | 1;
                        else
                                EDF.T0(k)  = tmp1;
                        end;
                end;
                tmp = H1(168 + [9:16]);
                for k = [4 5 6],
                        [tmp1,tmp] = strtok(tmp,' :./-');
                        tmp1=str2double([tmp1,' ']);
                        if isempty(tmp1)
                                ErrT0 = ErrT0 | 1;
                        else
                                EDF.T0(k)  = tmp1;
                        end;
                end;
                if ErrT0
                        EDF.ErrNo = [2,EDF.ErrNo];
                end;
        else
                % Y2K compatibility until year 2084
                if EDF.T0(1) < 85    % for biomedical data recorded in the 1950's and converted to EDF
                        EDF.T0(1) = 2000+EDF.T0(1);
                elseif EDF.T0(1) < 100
                        EDF.T0(1) = 1900+EDF.T0(1);
                %else % already corrected, do not change
                end;
        end;     
        H1(185:256)=setstr(fread(EDF.FILE.FID,256-184,'uchar')');     %
        EDF.HeadLen = str2double(H1(185:192));           % 8 Bytes  Length of Header
        EDF.reserved1=H1(193:236);              % 44 Bytes reserved   
        EDF.NRec    = str2double(H1(237:244));     % 8 Bytes  # of data records
        EDF.Dur     = str2double(H1(245:252));     % 8 Bytes  # duration of data record in sec
        EDF.NS      = str2double(H1(253:256));     % 4 Bytes  # of signals
	EDF.AS.H1   = H1;	                     % for debugging the EDF Header
end;

if strcmp(EDF.reserved1(1:4),'EDF+'),	% EDF+ specific header information 
	[EDF.Patient.Id,   tmp] = strtok(EDF.PID,' ');
	[EDF.Patient.Sex,  tmp] = strtok(tmp,' ');
	[EDF.Patient.Birthday, tmp] = strtok(tmp,' ');
	[EDF.Patient.Name, tmp] = strtok(tmp,' ');

	[chk, tmp] = strtok(EDF.RID,' ');
	if ~strcmp(chk,'Startdate')
		fprintf(EDF.FILE.stderr,'Warning SDFOPEN: EDF+ header is corrupted.\n');
	end;
	[EDF.Date2, tmp] = strtok(tmp,' ');
	[EDF.ID.Investigation, tmp] = strtok(tmp,' ');
	[EDF.ID.Investigator,  tmp] = strtok(tmp,' ');
	[EDF.ID.Equiment, tmp] = strtok(tmp,' ');
end;

if any(size(EDF.NS)~=1) %%%%% not EDF because filled out with ASCII(0) - should be spaces
        fprintf(EDF.FILE.stderr, 'Warning SDFOPEN: invalid NS-value in header of %s\n',EDF.FileName);
        EDF.ErrNo=[1040,EDF.ErrNo];
        EDF.NS=1;
end;
% Octave assumes EDF.NS is a matrix instead of a scalare. Therefore, we need
% Otherwise, eye(EDF.NS) will be executed as eye(size(EDF.NS)).
EDF.NS = EDF.NS(1);     

if isempty(EDF.HeadLen) %%%%% not EDF because filled out with ASCII(0) - should be spaces
        EDF.ErrNo=[1056,EDF.ErrNo];
        EDF.HeadLen=256*(1+EDF.NS);
end;

if isempty(EDF.NRec) %%%%% not EDF because filled out with ASCII(0) - should be spaces
        EDF.ErrNo=[1027,EDF.ErrNo];
        EDF.NRec = -1;
end;

if isempty(EDF.Dur) %%%%% not EDF because filled out with ASCII(0) - should be spaces
        EDF.ErrNo=[1088,EDF.ErrNo];
        EDF.Dur=30;
end;

if  any(EDF.T0>[2084 12 31 24 59 59]) | any(EDF.T0<[1985 1 1 0 0 0])
        EDF.ErrNo = [4, EDF.ErrNo];
end;

%%% Read variable Header %%%
if ~strcmp(EDF.VERSION(1:3),'GDF'),
        idx1=cumsum([0 H2idx]);
        idx2=EDF.NS*idx1;

        h2=zeros(EDF.NS,256);
        [H2,count]=fread(EDF.FILE.FID,EDF.NS*256,'uchar');
        if count < EDF.NS*256 
	        EDF.ErrNo=[8,EDF.ErrNo];
                return; 
        end;
                
        %tmp=find((H2<32) | (H2>126)); % would confirm 
        tmp = find((H2<32) | ((H2>126) & (H2~=255) & (H2~=181)& (H2~=230))); 
        if ~isempty(tmp) %%%%% not EDF because filled out with ASCII(0) - should be spaces
                H2(tmp) = 32; 
	        EDF.ErrNo = [1026,EDF.ErrNo];
        end;
        
        for k=1:length(H2idx);
                %disp([k size(H2) idx2(k) idx2(k+1) H2idx(k)]);
                h2(:,idx1(k)+1:idx1(k+1))=reshape(H2(idx2(k)+1:idx2(k+1)),H2idx(k),EDF.NS)';
        end;
        %size(h2),
        h2=setstr(h2);
        %(h2(:,idx1(9)+1:idx1(10))),
        %abs(h2(:,idx1(9)+1:idx1(10))),
        
        EDF.Label      =            h2(:,idx1(1)+1:idx1(2));
        EDF.Transducer =            h2(:,idx1(2)+1:idx1(3));
        EDF.PhysDim    =            h2(:,idx1(3)+1:idx1(4));
        EDF.PhysMin    = str2double(h2(:,idx1(4)+1:idx1(5)));
        EDF.PhysMax    = str2double(h2(:,idx1(5)+1:idx1(6)));
        EDF.DigMin     = str2double(h2(:,idx1(6)+1:idx1(7)));
        EDF.DigMax     = str2double(h2(:,idx1(7)+1:idx1(8)));
        EDF.PreFilt    =            h2(:,idx1(8)+1:idx1(9));
        EDF.SPR        = str2double(h2(:,idx1(9)+1:idx1(10)));
        %EDF.reserved  =       h2(:,idx1(10)+1:idx1(11));
	if ~all(abs(EDF.VERSION)==[255,abs('BIOSEMI')]),
		EDF.GDFTYP     = 3*ones(1,EDF.NS);	%	datatype
	else
		EDF.GDFTYP     = (255+24)*ones(1,EDF.NS);	%	datatype
	end;
	
        if isempty(EDF.SPR), 
                fprintf(EDF.FILE.stderr, 'Warning SDFOPEN: invalid SPR-value in header of %s\n',EDF.FileName);
                EDF.SPR=ones(EDF.NS,1);
	        EDF.ErrNo=[1028,EDF.ErrNo];
        end;
else
        if (ftell(EDF.FILE.FID)~=256),
		error('position error');
	end;	 
%%%        status = fseek(EDF.FILE.FID,256,'bof');
        EDF.Label      =  setstr(fread(EDF.FILE.FID,[16,EDF.NS],'uchar')');		
        EDF.Transducer =  setstr(fread(EDF.FILE.FID,[80,EDF.NS],'uchar')');	
        EDF.PhysDim    =  setstr(fread(EDF.FILE.FID,[ 8,EDF.NS],'uchar')');
%       EDF.AS.GDF.TEXT = EDF.GDFTYP.TEXT;
        EDF.PhysMin    =         fread(EDF.FILE.FID,[EDF.NS,1],'float64');	
        EDF.PhysMax    =         fread(EDF.FILE.FID,[EDF.NS,1],'float64');	

        %EDF.DigMin     =         fread(EDF.FILE.FID,[EDF.NS,1],'int64');	
        %EDF.DigMax     =         fread(EDF.FILE.FID,[EDF.NS,1],'int64');	
	tmp            =         fread(EDF.FILE.FID,[2*EDF.NS,1],'int32');	
        EDF.DigMin     = tmp((1:EDF.NS)*2-1);
        tmp            =         fread(EDF.FILE.FID,[2*EDF.NS,1],'int32');	
        EDF.DigMax     = tmp((1:EDF.NS)*2-1);
        
        EDF.PreFilt    =  setstr(fread(EDF.FILE.FID,[80,EDF.NS],'uchar')');	%	
        EDF.SPR        =         fread(EDF.FILE.FID,[ 1,EDF.NS],'uint32')';	%	samples per data record
        EDF.GDFTYP     =         fread(EDF.FILE.FID,[ 1,EDF.NS],'uint32');	%	datatype
        %                        fread(EDF.FILE.FID,[32,EDF.NS],'uchar')';	%	datatype
end;

EDF.Filter.LowPass = repmat(nan,1,EDF.NS);
EDF.Filter.HighPass = repmat(nan,1,EDF.NS);
for k=1:EDF.NS,
	tmp = EDF.PreFilt(k,:);

	ixh=strfind(tmp,'HP');
	ixl=strfind(tmp,'LP');
	ixn=strfind(tmp,'Notch');
	ix =strfind(lower(tmp),'hz');
	%tmp(tmp==':')=' ';
try;
	if any(tmp==';')
		[tok1,tmp] = strtok(tmp,';');
		[tok2,tmp] = strtok(tmp,';');
		[tok3,tmp] = strtok(tmp,';');
	else
		[tok1,tmp] = strtok(tmp,' ');
		[tok2,tmp] = strtok(tmp,' ');
		[tok3,tmp] = strtok(tmp,' ');
	end;
	[T1, F1 ] = strtok(tok1,': ');
	[T2, F2 ] = strtok(tok2,': ');
	[T3, F3 ] = strtok(tok3,': ');

	[F1 ] = strtok(F1,': ');
	[F2 ] = strtok(F2,': ');
	[F3 ] = strtok(F3,': ');
	
	if strcmp(F1,'DC'), F1='0'; end;
	if strcmp(F2,'DC'), F2='0'; end;
	if strcmp(F3,'DC'), F3='0'; end;
	
	tmp = strfind(lower(F1),'hz');
	if ~isempty(tmp), F1=F1(1:tmp-1); end;
	tmp = strfind(lower(F2),'hz');
	if ~isempty(tmp), F2=F2(1:tmp-1); end;
	tmp = strfind(lower(F3),'hz');
	if ~isempty(tmp), F3=F3(1:tmp-1); end;

	if strcmp(T1,'LP'), 
		EDF.Filter.LowPass(k) =str2double(F1);
	elseif strcmp(T1,'HP'), 
		EDF.Filter.HighPass(k)=str2double(F1);
	elseif strcmp(T1,'Notch'), 
		EDF.Filter.Notch(k)   =str2double(F1);
	end;
	if strcmp(T2,'LP'), 
		EDF.Filter.LowPass(k) =str2double(F2);
	elseif strcmp(T2,'HP'), 
		EDF.Filter.HighPass(k)=str2double(F2);
	elseif strcmp(T2,'Notch'), 
		EDF.Filter.Notch(k)   =str2double(F2);
	end;
	if strcmp(T3,'LP'), 
		EDF.Filter.LowPass(k) =str2double(F3);
	elseif strcmp(T3,'HP'), 
		EDF.Filter.HighPass(k)=str2double(F3);
	elseif strcmp(T3,'Notch'), 
		EDF.Filter.Notch(k)   =str2double(F3);
	end;
catch
	fprintf(1,'Cannot interpret: %s\n',EDF.PreFilt(k,:));
end;
end;

%		EDF=gdfcheck(EDF,1);
if any(EDF.PhysMax==EDF.PhysMin), EDF.ErrNo=[1029,EDF.ErrNo]; end;	
if any(EDF.DigMax ==EDF.DigMin ), EDF.ErrNo=[1030,EDF.ErrNo]; end;	
EDF.Cal = (EDF.PhysMax-EDF.PhysMin)./(EDF.DigMax-EDF.DigMin);
EDF.Off = EDF.PhysMin - EDF.Cal .* EDF.DigMin;
EDF.EDF.SampleRate = EDF.SPR / EDF.Dur;
EDF.AS.MAXSPR=1;
for k=1:EDF.NS,
        EDF.AS.MAXSPR = lcm(EDF.AS.MAXSPR,EDF.SPR(k));
end;
EDF.SampleRate = EDF.AS.MAXSPR/EDF.Dur;

EDF.AS.spb = sum(EDF.SPR);	% Samples per Block
EDF.AS.bi = [0;cumsum(EDF.SPR)]; 
EDF.AS.BPR  = ceil(EDF.SPR.*GDFTYP_BYTE(EDF.GDFTYP+1)'); 
EDF.AS.SAMECHANTYP = all(EDF.AS.BPR == (EDF.SPR.*GDFTYP_BYTE(EDF.GDFTYP+1)')) & ~any(diff(EDF.GDFTYP)); 
EDF.AS.GDFbi = [0;cumsum(ceil(EDF.SPR.*GDFTYP_BYTE(EDF.GDFTYP+1)'))]; 
EDF.AS.bpb = sum(ceil(EDF.SPR.*GDFTYP_BYTE(EDF.GDFTYP+1)'));	% Bytes per Block
EDF.AS.startrec = 0;
EDF.AS.numrec = 0;
EDF.AS.EVENTTABLEPOS = -1;

EDF.Calib = [EDF.Off'; diag(EDF.Cal)];
EDF.AS.endpos = EDF.FILE.size;

%[status, EDF.AS.endpos, EDF.HeadLen, EDF.AS.bpb EDF.NRec, EDF.HeadLen+EDF.AS.bpb*EDF.NRec]
if EDF.NRec == -1   % unknown record size, determine correct NRec
        EDF.NRec = floor((EDF.AS.endpos - EDF.HeadLen) / EDF.AS.bpb);
elseif  EDF.NRec ~= ((EDF.AS.endpos - EDF.HeadLen) / EDF.AS.bpb);
        if ~strcmp(EDF.VERSION(1:3),'GDF'),
                EDF.ErrNo=[16,EDF.ErrNo];
                fprintf(2,'\nWarning SDFOPEN: size (%i) of file %s does not fit headerinformation\n',EDF.AS.endpos,EDF.FileName);
                EDF.NRec = floor((EDF.AS.endpos - EDF.HeadLen) / EDF.AS.bpb);
        else
                EDF.AS.EVENTTABLEPOS = EDF.HeadLen + EDF.AS.bpb*EDF.NRec;
        end;
end; 


if 0, 
        
elseif strcmp(EDF.TYPE,'GDF') & (EDF.AS.EVENTTABLEPOS > 0),  
        status = fseek(EDF.FILE.FID, EDF.AS.EVENTTABLEPOS, 'bof');
        [EVENT.Version,c] = fread(EDF.FILE.FID,1,'char');
        EDF.EVENT.SampleRate = [1,256,65536]*fread(EDF.FILE.FID,3,'uint8');
	if ~EDF.EVENT.SampleRate, % ... is not defined in GDF 1.24 or earlier
		EDF.EVENT.SampleRate = EDF.SampleRate; 
	end;
        EVENT.N = fread(EDF.FILE.FID,1,'uint32');
        if EVENT.Version==1,
                [EDF.EVENT.POS,c1] = fread(EDF.FILE.FID,[EVENT.N,1],'uint32');
                [EDF.EVENT.TYP,c2] = fread(EDF.FILE.FID,[EVENT.N,1],'uint16');
                if any([c1,c2]~=EVENT.N) | (EDF.AS.endpos~=EDF.AS.EVENTTABLEPOS+8+EVENT.N*6),
                        fprintf(2,'\nERROR SDFOPEN: Eventtable corrupted in file %s\n',EDF.FileName);
                end
                
                % convert EVENT.Version 1 to 3
                EDF.EVENT.CHN = zeros(EVENT.N,1);    
                EDF.EVENT.DUR = zeros(EVENT.N,1);    
                flag_remove = zeros(size(EDF.EVENT.TYP));        
                types  = unique(EDF.EVENT.TYP);
                for k1 = find(bitand(types(:)',hex2dec('8000')));
                        TYP0 = bitand(types(k1),hex2dec('7fff'));
                        TYP1 = types(k1);
                        ix0 = (EDF.EVENT.TYP==TYP0);
                        ix1 = (EDF.EVENT.TYP==TYP1);
                        if sum(ix0)==sum(ix1), 
                                EDF.EVENT.DUR(ix0) = EDF.EVENT.POS(ix1) - EDF.EVENT.POS(ix0);
                                flag_remove = flag_remove | (EDF.EVENT.TYP==TYP1);
                        else 
                                fprintf(2,'Warning ELOAD: number of event onset (TYP=%s) and event offset (TYP=%s) differ\n',dec2hex(TYP0),dec2hex(TYP1));
                        end;
                end
                if any(EDF.EVENT.DUR<0)
                        fprintf(2,'Warning ELOAD: EVENT ONSET later than EVENT OFFSET\n',dec2hex(TYP0),dec2hex(TYP1));
                        EDF.EVENT.DUR(:) = 0
                end;
                EDF.EVENT.TYP = EDF.EVENT.TYP(~flag_remove);
                EDF.EVENT.POS = EDF.EVENT.POS(~flag_remove);
                EDF.EVENT.CHN = EDF.EVENT.CHN(~flag_remove);
                EDF.EVENT.DUR = EDF.EVENT.DUR(~flag_remove);
                EVENT.Version = 3; 
                
        elseif EVENT.Version==3,
                [EDF.EVENT.POS,c1] = fread(EDF.FILE.FID,[EVENT.N,1],'uint32');
                [EDF.EVENT.TYP,c2] = fread(EDF.FILE.FID,[EVENT.N,1],'uint16');
                [EDF.EVENT.CHN,c3] = fread(EDF.FILE.FID,[EVENT.N,1],'uint16');
                [EDF.EVENT.DUR,c4] = fread(EDF.FILE.FID,[EVENT.N,1],'uint32');
                if any([c1,c2,c3,c4]~=EVENT.N) | (EDF.AS.endpos~=EDF.AS.EVENTTABLEPOS+8+EVENT.N*12),
                        fprintf(2,'\nERROR SDFOPEN: Eventtable corrupted in file %s\n',EDF.FileName);
                end
                
        else
                fprintf(2,'\nWarning SDFOPEN: Eventtable version %i not supported\n',EVENT.Version);
        end;
        EDF.AS.endpos = EDF.AS.EVENTTABLEPOS;   % set end of data block, might be important for SSEEK

        % Classlabels according to 
        % http://cvs.sourceforge.net/viewcvs.py/*checkout*/biosig/biosig/t200/eventcodes.txt
        if (length(EDF.EVENT.TYP)>0)
                ix = (EDF.EVENT.TYP>hex2dec('0300')) & (EDF.EVENT.TYP<hex2dec('030d'));
                ix = ix | (EDF.EVENT.TYP==hex2dec('030f')); % unknown/undefined cue
                EDF.Classlabel = mod(EDF.EVENT.TYP(ix),256);
                EDF.Classlabel(EDF.Classlabel==15) = NaN; % unknown/undefined cue
        end;

        % Trigger information and Artifact Selection 
        ix = find(EDF.EVENT.TYP==hex2dec('0300')); 
        EDF.TRIG = EDF.EVENT.POS(ix);
        EDF.ArtifactSelection = repmat(logical(0),length(ix),1);
        for k = 1:length(ix),
                ix2 = find(EDF.EVENT.POS(ix(k))==EDF.EVENT.POS);
                if any(EDF.EVENT.TYP(ix2)==hex2dec('03ff'))
                        EDF.ArtifactSelection(k) = logical(1);                
                end;
        end;
        
elseif strcmp(EDF.TYPE,'EDF') & (length(strmatch('EDF Annotations',EDF.Label))==1),
        % EDF+: 
        tmp = strmatch('EDF Annotations',EDF.Label);
        EDF.EDF.Annotations = tmp;
        EDF.Cal(EDF.EDF.Annotations) = 1;
        EDF.Off(EDF.EDF.Annotations) = 0;
        
        status = fseek(EDF.FILE.FID,EDF.HeadLen+EDF.AS.bi(EDF.EDF.Annotations)*2,'bof');
        t = fread(EDF.FILE.FID,EDF.SPR(EDF.EDF.Annotations),'uchar',EDF.AS.bpb-EDF.SPR(EDF.EDF.Annotations)*2);
        t = char(t)';
        lt = length(t);
        EDF.EDF.ANNONS = t;
        N = 0; 
        ix = 1; 
	t = [t,' '];
        while ix < length(t),
                while (ix<=lt) & (t(ix)==0), ix = ix+1; end;
                ix1 = ix; 
                while (ix<=lt) & (t(ix)~=0), ix = ix+1; end;
                ix2 = ix; 
                if (ix < lt),
                        v = t(ix1:ix2-1);
                        [s1,v]=strtok(v,20);
                        s1(s1==21) = 0;
                        tmp=str2double(char(s1));
                        
                        [s2,v]=strtok(v,20);
                        [s3,v]=strtok(v,20);
                        
                        N = N+1;
                        EDF.EVENT.POS(N,1) = tmp(1);
                        if length(tmp)>1,
                                EDF.EVENT.DUR(N,1) = tmp(2);
                        end;
                        EDF.EVENT.TeegType{N,1} = char(s2);
                        EDF.EVENT.TeegDesc{N,1} = char(s3);
                end;
        end;
        EDF.EVENT.TYP(1:N,1) = 0;

elseif strcmp(EDF.TYPE,'EDF') & (length(EDF.FILE.Name)==8) & any(lower(EDF.FILE.Name(1))=='bchmnpsu') 
        if strcmp(lower(EDF.FILE.Name([3,6:8])),'001a'),
                % load scoring of ADB database if available 

                fid2 = fopen(fullfile(EDF.FILE.Path, [EDF.FILE.Name(1:7),'.txt']),'r');
                if fid2<0,
                        fid2 = fopen(fullfile(EDF.FILE.Path,[lower(EDF.FILE.Name(1:7)),'.txt']),'r');
                end
                if fid2<0,
                        fid2 = fopen(fullfile(EDF.FILE.Path,[EDF.FILE.Name(1:7),'.TXT']),'r');
                end
                if fid2>0,
                        tmp = fread(fid2,inf,'char');
                        fclose(fid2);
                        [ma,status] = str2double(char(tmp'));
                        if ~any(isnan(status(:))),
                                %%% TODO: include ADB2EVENT here

				% code from MAK2BIN.M (C) 1998-2004 A. Schlögl 
				ERG = zeros(size(ma));

				%%%% one artifact %%%%
				for k=0:9,
				        if exist('OCTAVE_VERSION')==5
				                ERG = ERG+(ma==k)*2^k;
				        else
				                ERG(ma==k) = 2^k;
				        end;
				end;

				%%%% more than one artifact %%%%
				[i,j] = find(ma>9);
				L='123456789';
				for k=1:length(i),
				        b=int2str(ma(i(k),j(k)));
				        erg=0;
				        for l=1:9,
				                if any(b==L(l)), erg=erg+2^l; end;
				        end;        
				        ERG(i(k),j(k)) = erg;
				end;
				
				N   = 0;
				POS = [];
				TYP = [];
				DUR = [];
				CHN = [];
				cc  = zeros(1,10);
				for k = 1:9,
				        for c = 1:7;%size(ERG,2),
				                tmp = [0;~~(bitand(ERG(:,c),2^k));0];
				 
				                cc(k+1) = cc(k+1) + sum(tmp);
				                pos = find(diff(tmp)>0);
				                pos2 = find(diff(tmp)<0);
				                n   = length(pos);
				                
				                POS = [POS; pos(:)];
				                TYP = [TYP; repmat(k,n,1)];
				                CHN = [CHN; repmat(c,n,1)];
				                DUR = [DUR; pos2(:)-pos(:)];
				                N   = N + n;
				        end;
				end;
				
				EDF.EVENT.Fs = 1;
				if nargin>1,
				        EVENT.Fs = EDF.SampleRate;
				end;
				
				[tmp,ix] = sort(POS);
				EDF.EVENT.POS = (POS(ix)-1)*EVENT.Fs+1;
				EDF.EVENT.TYP = TYP(ix) + hex2dec('0100');
				EDF.EVENT.CHN = CHN(ix);
				EDF.EVENT.DUR = DUR(ix)*EVENT.Fs;
				
				%EDF.EVENT.ERG = ERG;
			end;
                end
        end;
else
	% search for WSCORE scoring file in path and in file directory. 
	tmp = [upper(EDF.FILE.Name),'.006'];
        fid2 = fopen(fullfile(EDF.FILE.Path,tmp),'r');
        if fid2 > 0,
                tmp = fread(fid2,inf,'char');
                fclose(fid2);
                [x,status] = str2double(char(tmp'));
                if ~any(isnan(status(:))),
                        EDF.EVENT.POS = x(:,1);
                        EDF.EVENT.TYP = x(:,2);
                end;
        end;
end;

status = fseek(EDF.FILE.FID, EDF.HeadLen, 'bof');
EDF.FILE.POS = 0;

% if Channelselect, ReReferenzing and Resampling
% Overflowcheck, Adaptive FIR
% Layer 4 

%if nargin<3 %%%%%          Channel Selection 
if nargin <3 %else
        arg3=0;
end;

EDF.SIE.ChanSelect = 1:EDF.NS;
EDF.InChanSelect = 1:EDF.NS;

%EDF.SIE.RR=1; %is replaced by ~EDF.FLAG.UCAL
if ~isfield(EDF,'FLAG')
        EDF.FLAG.UCAL=0;
end;
if ~isfield(EDF.FLAG,'UCAL');
        EDF.FLAG.UCAL=0;
end;
EDF.SIE.RS=0; %exist('arg5')==1; if EDF.SIE.RS, EDF.SIE.RS==(arg5>0); end;
EDF.SIE.TH=0; %(exist('arg6')==1);
EDF.SIE.RAW=0;
EDF.SIE.REGC=0;
EDF.SIE.TECG=0;
EDF.SIE.AFIR=0;
EDF.SIE.FILT=0;
EDF.SIE.TimeUnits_Seconds=1;
%EDF.SIE.ReRefMx=eye(EDF.NS);
EDF.SIE.REG=eye(EDF.NS);
%EDF.SIE.ReRefMx = EDF.SIE.ReRefMx(:,EDF.SIE.ChanSelect);
%EDF.Calib=EDF.Calib*EDF.SIE.REG*EDF.SIE.ReRefMx; 

%if nargin>2 %%%%%          Channel Selection 
        EDF.SIE.REG=eye(EDF.NS);
        if arg3==0
                EDF.SIE.ChanSelect = 1:EDF.NS;
                EDF.InChanSelect = 1:EDF.NS;
                EDF.SIE.ReRefMx = eye(EDF.NS);
        else
                [nr,nc]=size(arg3);
                if all([nr,nc]>1), % Re-referencing
                        EDF.SIE.ReRefMx = [[arg3 zeros(size(arg3,1),EDF.NS-size(arg3,2))]; zeros(EDF.NS-size(arg3,1),EDF.NS)];
                        EDF.InChanSelect = find(any(EDF.SIE.ReRefMx'));
                        EDF.SIE.ChanSelect = find(any(EDF.SIE.ReRefMx));
                        if nargin>3
                        %        fprintf(EDF.FILE.stderr,'Error SDFOPEN: Rereferenzing does not work correctly with %s (more than 3 input arguments)\n',arg4);
                        end;
                else
                        EDF.SIE.ChanSelect = arg3; %full(sparse(1,arg3(:),1,1,EDF.NS));
                        EDF.InChanSelect = arg3;
                        EDF.SIE.ReRefMx = eye(EDF.NS);
                end;
        end;
        
        if (exist('sedfchk')==2),  
                EDF=sedfchk(EDF); % corrects incorrect Header information. 
        end;

	%EDF.SIE.CS=1;
        EDF.SIE.RS=exist('arg5')==1; if EDF.SIE.RS, EDF.SIE.RS==(arg5>0); end;
        EDF.SIE.TH=0; %(exist('arg6')==1);
        EDF.SIE.RAW=0;
        EDF.SIE.REGC=0;
        EDF.SIE.TECG=0;
        EDF.SIE.AFIR=0;
        EDF.SIE.FILT=0;
        %EDF.AS.MAXSPR=max(EDF.SPR(EDF.SIE.ChanSelect)); % Layer 3 defines EDF.AS.MAXSPR in GDFREAD
        if 0,
		EDF.AS.MAXSPR=EDF.SPR(EDF.SIE.ChanSelect(1));
	        for k=2:length(EDF.SIE.ChanSelect),
    		        EDF.AS.MAXSPR = lcm(EDF.AS.MAXSPR,EDF.SPR(EDF.SIE.ChanSelect(k)));
	        end;
	end;
	EDF.SampleRate = EDF.AS.MAXSPR/EDF.Dur;
        
        EDF.SIE.REG=eye(EDF.NS);

        %elseif nargin>3   %%%%% RE-REFERENCING
        if nargin>3   
                if ~isempty(strfind(upper(arg4),'SIESTA'))
                        EDF.SIE.ReRefMx = eye(EDF.NS);% speye(EDF.NS);
                        EDF.SIE.ReRefMx(7,1:6)=[1 1 1 -1 -1 -1]/2;
                        EDF.SIE.TH=1;
	                % calculate (In)ChanSelect based on ReRefMatrix
    		        EDF.InChanSelect = find(any(EDF.SIE.ReRefMx'));
            		EDF.SIE.ChanSelect = find(any(EDF.SIE.ReRefMx));
                end;
                if ~isempty(strfind(upper(arg4),'EOG'))
                        tmp=strfind(upper(arg4),'EOG');
                        tmp=lower(strtok(arg4(tmp:length(arg4)),' +'));
                        EDF.SIE.ReRefMx = sparse(EDF.NS,EDF.NS);% speye(EDF.NS);
                        if any(tmp=='h'); EDF.SIE.ReRefMx(8:9,1)=[ 1 -1 ]';  end;
                        if any(tmp=='v'); EDF.SIE.ReRefMx(1:9,2)=[ 1 0 0 1 0 0 1 -1 -1]';end;
                        if any(tmp=='r'); EDF.SIE.ReRefMx(3:9,3)=[ 1 0 0 1 2 -2 -2]'/2;  end;
                        %EDF.SIE.TH = 1;
	                % calculate (In)ChanSelect based on ReRefMatrix
    		        EDF.InChanSelect = find(any(EDF.SIE.ReRefMx'));
            		EDF.SIE.ChanSelect = find(any(EDF.SIE.ReRefMx));
                end;
                
                if ~isempty(strfind(upper(arg4),'UCAL'))
                        %EDF.SIE.ReRefMx = speye(EDF.NS); % OVERRIDES 'SIESTA' and 'REGRESS_ECG'
                        %EDF.SIE.RR  = 0;
                        EDF.FLAG.UCAL = 1;
                end;
                if ~isempty(strfind(upper(arg4),'RAW'))
                        EDF.SIE.RAW = 1;
                end;
                if ~isempty(strfind(upper(arg4),'OVERFLOW'))
                        EDF.SIE.TH  = 1;
                end;
                if ~isempty(strfind(arg4,'FailingElectrodeDetector'))
                        EDF.SIE.FED = 1;
			EDF.SIE.TH  = 2;
                end;
		
                if ~isempty(strfind(upper(arg4),'ECG')), % identify ECG channel for some ECG artifact processing method   
                        if ~isempty(strfind(upper(arg4),'SIESTA'))
                                channel1=12;
                                %channel2=1:9;
                                M=zeros(EDF.NS,1);M(channel1)=1;
                        end
                        if isfield(EDF,'ChanTyp')
                                M=upper(char(EDF.ChanTyp(channel1)))=='C';
                                channel1=find(M);
                                M=(upper(char(EDF.ChanTyp))=='E' | upper(char(EDF.ChanTyp))=='O' );
                                %channel2=find(M);
                        else
                                channel1 = 12;
                                %channel2=1:9;
                                M=zeros(EDF.NS,1);M(channel1)=1;
                        end;
                        
                end;
                if ~isempty(strfind(upper(arg4),'RECG'))
                        EDF.SIE.REGC = 1;
                        if all(EDF.InChanSelect~=channel1)
                                EDF.InChanSelect=[EDF.InChanSelect channel1];        
                        end;
                end;
                
                if ~isempty(strfind(upper(arg4),'TECG'))
                        fprintf(EDF.FILE.stderr,'SDFOPEN: status of TECG Mode: alpha test passed\n');    
                        %%%% TECG - ToDo
                        % - optimize window
                        if exist([lower(EDF.FILE.Name) 'ECG.mat']);
                                if exist('OCTAVE_VERSION')==5
                                        load(file_in_loadpath([lower(EDF.FILE.Name) 'ECG.mat']));
                                else
                                        load([lower(EDF.FILE.Name) 'ECG.mat']);
                                end;
                                if isstruct(QRS)
                                        EDF.SIE.TECG = 1;
					%%%  EDF.AS.MAXSPR=size(QRS.Templates,1)/3;
                                        EDF.AS.MAXSPR=lcm(EDF.AS.MAXSPR,EDF.SPR(channel1));
				else
                                        fprintf(EDF.FILE.stderr,'WARNING SDFOPEN: %s invalid for TECG\n',[ lower(EDF.FILE.Name) 'ECG.mat']);
                                end;
                        else
                                fprintf(EDF.FILE.stderr,'WARNING SDFOPEN: %s not found\t (needed for Mode=TECG)\n',[lower(EDF.FILE.Name) 'ECG.mat']);
                        end;
                end;
                
                if ~isempty(strfind(upper(arg4),'AFIR')) 
                        % Implements Adaptive FIR filtering for ECG removal in EDF/GDF-tb.
                        % based on the Algorithm of Mikko Koivuluoma <k7320@cs.tut.fi>
                                
                        % channel2 determines which channels should be corrected with AFIR 
                        
                        %EDF = sdf_afir(EDF,12,channel2);
                        channel2=EDF.SIE.ChanSelect;	
                        fprintf(EDF.FILE.stderr,'Warning SDFOPEN: option AFIR still buggy\n');    
                        if isempty(find(channel2))
                                EDF.SIE.AFIR=0;
                        else
                                EDF.SIE.AFIR=1;
                                EDF.AFIR.channel2=channel2;
                                
                                EDF.AFIR.alfa=0.01;
                                EDF.AFIR.gamma=1e-32;
                                
                                EDF.AFIR.delay  = ceil(0.05*EDF.AS.MAXSPR/EDF.Dur); 
                                EDF.AFIR.nord = EDF.AFIR.delay+EDF.AS.MAXSPR/EDF.Dur; 
                                
                                EDF.AFIR.nC = length(EDF.AFIR.channel2);
                                EDF.AFIR.w = zeros(EDF.AFIR.nC, max(EDF.AFIR.nord));
                                EDF.AFIR.x = zeros(1, EDF.AFIR.nord);
                                EDF.AFIR.d = zeros(EDF.AFIR.delay, EDF.AFIR.nC);
                                
                                channel1=12;
                                
                                if isfield(EDF,'ChanTyp')
                                        if upper(char(EDF.ChanTyp(channel1)))=='C';
                                                EDF.AFIR.channel1 = channel1;
                                        else
                                                EDF.AFIR.channel1 = find(EDF.ChanTyp=='C');
                                                fprintf(EDF.FILE.stderr,'Warning %s: #%i is not an ECG channel, %i used instead\n' ,filename,channel1,EDF.AFIR.channel1);
                                        end;
                                else
                                        EDF.AFIR.channel1 = channel1;
                                end;
                                
                                if all(EDF.InChanSelect~=channel1)
                                        EDF.InChanSelect=[EDF.InChanSelect channel1];        
                                end;
                        end;
                end;


                if isempty(strfind(upper(arg4),'NOTCH50')) 
                        EDF.Filter.A = 1;
                        EDF.Filter.B = 1;
                else
                        EDF.SIE.FILT = 1;
                        EDF.Filter.A = 1;
                        EDF.Filter.B = 1;
                        %if all(EDF.SampleRate(EDF.SIE.ChanSelect)==100)
                        if EDF.AS.MAXSPR/EDF.Dur==100
                                EDF.Filter.B=[1 1]/2;
                                %elseif all(EDF.SampleRate(EDF.SIE.ChanSelect)==200)
                        elseif EDF.AS.MAXSPR/EDF.Dur==200
                                EDF.Filter.B=[1 1 1 1]/4;
                                %elseif all(EDF.SampleRate(EDF.SIE.ChanSelect)==256)
                        elseif EDF.AS.MAXSPR/EDF.Dur==400
                                EDF.Filter.B=ones(1,8)/8;
                                %elseif all(EDF.SampleRate(EDF.SIE.ChanSelect)==256)
                        elseif EDF.AS.MAXSPR/EDF.Dur==256
                                EDF.Filter.B=poly([exp([-1 1 -2 2]*2*pi*i*50/256)]); %max(EDF.SampleRate(EDF.SIE.ChanSelect)))]);
                                EDF.Filter.B=EDF.Filter.B/sum(EDF.Filter.B);
                        else
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN: 50Hz Notch does not fit\n');
                        end;
                end;



                if ~isempty(strfind(upper(arg4),'NOTCH60')) 
                        fprintf(EDF.FILE.stderr,'Warning SDFOPEN: option NOTCH60 not implemented yet.\n');    
                end;


                if ~isempty(strfind(upper(arg4),'HPF')),  % high pass filtering
                        if EDF.SIE.FILT==0; EDF.Filter.B=1; end;
                        EDF.SIE.FILT=1;
                        EDF.Filter.A=1;
		end;
                if ~isempty(strfind(upper(arg4),'HPF0.0Hz')),  % high pass filtering
                        EDF.Filter.B=conv([1 -1], EDF.Filter.B);
                elseif ~isempty(strfind(upper(arg4),'TAU')),  % high pass filtering / compensate time constant
                        tmp=strfind(upper(arg4),'TAU');
                        TAU=strtok(upper(arg4(tmp:length(arg4))),'S');
                        tau=str2double(TAU);
                        if isempty(tau)
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN: invalid tau-value.\n');
                        else
                                EDF.Filter.B=conv([1 (EDF.Dur/EDF.AS.MAXSPR/tau-1)], EDF.Filter.B);
                        end;
			
                %%%% example 'HPF_1.0Hz_Hamming',  % high pass filtering
                elseif ~isempty(strfind(upper(arg4),'HPF')),  % high pass filtering
			    tmp=strfind(upper(arg4),'HPF');
			    FilterArg0=arg4(tmp+4:length(arg4));
			    %[tmp,FilterArg0]=strtok(arg4,'_');
			    [FilterArg1,FilterArg2]=strtok(FilterArg0,'_');
			    [FilterArg2,FilterArg3]=strtok(FilterArg2,'_');
			    tmp=strfind(FilterArg1,'Hz');
			    F0=str2double(FilterArg1(1:tmp-1));				    
			    B=feval(FilterArg2,F0*EDF.AS.MAXSPR/EDF.Dur);
			    B=B/sum(B);
			    B(ceil(length(B)/2))=(B(ceil(length(B)/2)))-1;
			    
                        EDF.Filter.B=conv(-B, EDF.Filter.B);
                end;


                if ~isempty(strfind(upper(arg4),'UNITS_BLOCK'))
			EDF.SIE.TimeUnits_Seconds=0; 
                end;

        end; % end nargin >3
        
        if EDF.SIE.FILT==1;
		EDF.Filter.Z=[];
                for k=1:length(EDF.SIE.ChanSelect),
                        [tmp,EDF.Filter.Z(:,k)]=filter(EDF.Filter.B,EDF.Filter.A,zeros(length(EDF.Filter.B+1),1));
                end;
    		EDF.FilterOVG.Z=EDF.Filter.Z;
        end;
        
        if EDF.SIE.REGC
                FN=[lower(EDF.FILE.Name) 'cov.mat'];
                if exist(FN)~=2
                        fprintf(EDF.FILE.stderr,'Warning SDFOPEN: Covariance-file %s not found.\n',FN);
                        EDF.SIE.REGC=0;   
                else
                        if exist('OCTAVE_VERSION')==5
                                load(file_in_loadpath(FN));
                        else
                                load(FN);
                        end;
                        if exist('XC') == 1
                                %EDF.SIE.COV = tmp.XC;
                                %[N,MU,COV,Corr]=decovm(XC);
                                N=size(XC,2);
                                COV=(XC(2:N,2:N)/XC(1,1)-XC(2:N,1)*XC(1,2:N)/XC(1,1)^2);
                                
                                %clear tmp;
                                %cov = diag(EDF.Cal)*COV*diag(EDF.Cal);
                                mcov = M'*diag(EDF.Cal)*COV*diag(EDF.Cal);
                                %mcov(~())=0;
                                EDF.SIE.REG = eye(EDF.NS) - M*((mcov*M)\(mcov));
                                EDF.SIE.REG(channel1,channel1) = 1; % do not remove the regressed channels
                                %mcov, EDF.SIE.REG, 
                        else
                                fprintf(EDF.FILE.stderr,'Error SDFOPEN: Regression Coefficients for ECG minimization not available.\n');
                        end;
                end;
                
        end;
        
        if EDF.SIE.TECG == 1; 
                % define channels that should be corrected
                if isfield(QRS,'Version')
                        OutChanSelect=[1:11 13:EDF.NS];
                        if EDF.SIE.REGC % correct templates
                                QRS.Templates=QRS.Templates*EDF.SIE.REG;
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN: Mode TECG+RECG not tested\n');
                        end;
                        if QRS.Version~=2
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN Mode TECG: undefined QRS-version\n');
                        end;
                else
                        %OutChanSelect=find(EDF.ChanTyp=='E' | EDF.ChanTyp=='O');
                        OutChanSelect=[1:9 ];
			if any(EDF.SIE.ChanSelect>10)
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN: Mode TECG: Only #1-#9 are corrected\n');
                        end;
                        if EDF.SIE.REGC, % correct the templates
                                QRS.Templates=QRS.Templates*EDF.SIE.REG([1:9 12],[1:9 12]);
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN: Mode TECG+RECG not tested\n');
                        end;
                        
                end;
                fs = EDF.SPR(12)/EDF.Dur; 
                QRS.Templates=detrend(QRS.Templates,0); %remove mean
                EDF.TECG.idx = [(QRS.Index-fs/2-1) (EDF.NRec+1)*EDF.SPR(12)]; %include terminating element
                EDF.TECG.idxidx = 1; %pointer to next index

                % initialize if any spike is detected before first window    
	        pulse = zeros(length(QRS.Templates),1);
    		Index=[];
	        while EDF.TECG.idx(EDF.TECG.idxidx) < 1,
    		        Index=[Index EDF.TECG.idx(EDF.TECG.idxidx)-EDF.AS.startrec*EDF.SPR(12)];
            		EDF.TECG.idxidx=EDF.TECG.idxidx+1;
	        end;
	        if ~isempty(Index)
    		        pulse(Index+length(QRS.Templates)) = 1;  
	        end;
        
                for k=1:length(EDF.InChanSelect),
                        k=find(OutChanSelect==EDF.InChanSelect(k));
                        if isempty(k)
                                EDF.TECG.QRStemp(:,k) = zeros(fs,1);
                        else
                                EDF.TECG.QRStemp(:,k) = QRS.Templates(0.5*fs:1.5*fs-1,k).*hanning(fs);
                        end;
                        [tmp,EDF.TECG.Z(:,k)] = filter(EDF.TECG.QRStemp(:,k),1,pulse);
                end;
        end; % if EDF.SIE.TECG==1
        
        %syms Fp1 Fp2 M1 M2 O2 O1 A1 A2 C3 C4
        if 0, % ??? not sure, whether it has any advantage
        for k=EDF.SIE.ChanSelect,
                %fprintf(1,'#%i: ',k);
                tmp=find(EDF.SIE.ReRefMx(:,k))';
                
                if EDF.SIE.ReRefMx(tmp(1),k)==1,
                        x=EDF.Label(tmp(1),:);
                else
                        x=sprintf('%3.1f*%s',EDF.SIE.ReRefMx(tmp(1),k),deblank(EDF.Label(tmp(1),:)));
                end;
                for l=2:length(tmp), L=tmp(l);
                        if (EDF.SPR(tmp(l-1),:)~=EDF.SPR(tmp(l),:))  
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN: SampleRate Mismatch in "%s", channel #%i and #%i\n',EDF.FILE.Name,tmp(l-1),tmp(l));
                        end;
                        if ~strcmp(EDF.PhysDim(tmp(l-1),:),EDF.PhysDim(tmp(l),:))  
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN: Dimension Mismatch in "%s", channel #%i and #%i\n',EDF.FILE.Name,tmp(l-1),tmp(l));
                        end;
                        if ~strcmp(EDF.Transducer(tmp(l-1),:),EDF.Transducer(tmp(l),:))  
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN: Transducer Mismatch in "%s", channel #%i and #%i\n',EDF.FILE.Name,tmp(l-1),tmp(l));
                        end;
                        if ~strcmp(EDF.PreFilt(tmp(l-1),:),EDF.PreFilt(tmp(l),:))  
                                fprintf(EDF.FILE.stderr,'Warning SDFOPEN: PreFiltering Mismatch in "%s", channel #%i and #%i\n',EDF.FILE.Name,tmp(l-1),tmp(l));
                        end;
                        x=[x sprintf('+(%3.1f)*(%s)',EDF.SIE.ReRefMx(tmp(l),k),deblank(EDF.Label(tmp(l),:)))];
                end;
                EDF.Label(k,1:length(x))=x; %char(sym(x))
        end;
        %Label,
        EDF.SIE.ReRefMx = EDF.SIE.ReRefMx(:,EDF.SIE.ChanSelect);
	        
        EDF.PhysDim = EDF.PhysDim(EDF.SIE.ChanSelect,:);
        EDF.PreFilt = EDF.PreFilt(EDF.SIE.ChanSelect,:);
        EDF.Transducer = EDF.Transducer(EDF.SIE.ChanSelect,:);
        end;
        
        if EDF.SIE.RS,
		tmp = EDF.AS.MAXSPR/EDF.Dur;
                if arg5==0
                        %arg5 = max(EDF.SPR(EDF.SIE.ChanSelect))/EDF.Dur;
                        
                elseif ~rem(tmp,arg5); % The target sampling rate must divide the source sampling rate   
			EDF.SIE.RS = 1;
			tmp=tmp/arg5;
			EDF.SIE.T = ones(tmp,1)/tmp;
                elseif arg5==100; % Currently, only the target sampling rate of 100Hz are supported. 
                        EDF.SIE.RS=1;
                        tmp=EDF.AS.MAXSPR/EDF.Dur;
                        if exist('OCTAVE_VERSION')
                                load resample_matrix4octave.mat T256100 T200100;
                        else
                                load('resample_matrix');
                        end;
                        if 1,
                                if tmp==400,
                                        EDF.SIE.T=ones(4,1)/4;
                                elseif tmp==256,
                                        EDF.SIE.T=T256100;
                                elseif tmp==200,
                                        EDF.SIE.T=T200100; 
                                elseif tmp==100,
                                        EDF.SIE.T=1;
                                else
                                        fprintf('Warning SDFOPEN-READ: sampling rates should be equal\n');     
                                end;	
                        else
                                tmp=EDF.SPR(EDF.SIE.ChanSelect)/EDF.Dur;
                                if all((tmp==256) | (tmp<100)) 
                                        EDF.SIE.RS = 1;
                                        %tmp=load(RSMN,'T256100');	
                                        EDF.SIE.T = T256100;	
                                elseif all((tmp==400) | (tmp<100)) 
                                        EDF.SIE.RS = 1;
                                        EDF.SIE.T = ones(4,1)/4;
                                elseif all((tmp==200) | (tmp<100)) 
                                        EDF.SIE.RS = 1;
                                        %tmp=load(RSMN,'T200100');	
                                        EDF.SIE.T = T200100;	
                                elseif all(tmp==100) 
                                        %EDF.SIE.T=load('resample_matrix','T100100');	
                                        EDF.SIE.RS=0;
                                else
                                        EDF.SIE.RS=0;
                                        fprintf('Warning SDFOPEN-READ: sampling rates should be equal\n');     
                                end;
                        end;
                else
                        fprintf(EDF.FILE.stderr,'Error SDFOPEN-READ: invalid target sampling rate of %i Hz\n',arg5);
                        EDF.SIE.RS=0;
			EDF.ErrNo=[EDF.ErrNo,];
			%EDF=sdfclose(EDF);
			%return;
                end;
        end;
        
        FN=[lower(EDF.FILE.Name), 'th.mat'];
        if exist(FN)~=2,
	        if EDF.SIE.TH, % && ~exist('OCTAVE_VERSION'),
                        fprintf(EDF.FILE.stderr,'Warning SDFOPEN: THRESHOLD-file %s not found.\n',FN);
                        EDF.SIE.TH=0;   
                end;
        else
                if exist('OCTAVE_VERSION')==5
                        tmp=load(file_in_loadpath(FN));
                else
                        tmp=load(FN);
                end;
                if isfield(tmp,'TRESHOLD'), 
                        EDF.SIE.THRESHOLD = tmp.TRESHOLD;
            	        %fprintf(EDF.FILE.stderr,'Error SDFOPEN: TRESHOLD''s not found.\n');
                end;
                

        end;
    	if EDF.SIE.TH>1, % Failing electrode detector 
	        fprintf(2,'Warning SDFOPEN (%s): FED not implemented yet\n',EDF.FileName);
                for k=1:length(EDF.InChanSelect),K=EDF.InChanSelect(k);
	        %for k=1:EDF.NS,
    		        [y1,EDF.Block.z1{k}] = filter([1 -1], 1, zeros(EDF.SPR(K)/EDF.Dur,1));
            		[y2,EDF.Block.z2{k}] = filter(ones(1,EDF.SPR(K)/EDF.Dur)/(EDF.SPR(K)/EDF.Dur),1,zeros(EDF.SPR(K)/EDF.Dur,1));
                
           		[y3,EDF.Block.z3{k}] = filter(ones(1,EDF.SPR(K)/EDF.Dur)/(EDF.SPR(K)/EDF.Dur),1,zeros(EDF.SPR(K)/EDF.Dur,1));
    		end;
        end;

% Initialization of Bufferblock for random access (without EDF-blocklimits) of data 
	if ~EDF.SIE.RAW & EDF.SIE.TimeUnits_Seconds
                EDF.Block.number=[0 0 0 0]; %Actual Blocknumber, start and end time of loaded block, diff(EDF.Block.number(1:2))==0 denotes no block is loaded;
                                            % EDF.Blcok.number(3:4) indicate start and end of the returned data, [units]=samples.
		EDF.Block.data=[];
		EDF.Block.dataOFCHK=[];
	end;
        
%end; % end of SDFOPEN-READ



%%%%%%% ============= WRITE ===========%%%%%%%%%%%%        

elseif any(arg2=='w') %  | (arg2=='w+')
%        fprintf(EDF.FILE.stderr,'error EDFOPEN: write mode not possible.\n'); 
        H1=[]; H2=[];
%        return;
        
EDF.SIE.RAW = 0;
if ~isstruct(arg1)  % if arg1 is the filename 
        EDF.FileName=arg1;
        if nargin<3
                tmp=input('SDFOPEN: list of samplerates for each channel? '); 
                EDF.EDF.SampleRate = tmp(:);
        else
                EDF.EDF.SampleRate = arg3;
        end;
        EDF.NS=length(EDF.EDF.SampleRate);
        if nargin<4
                tmp=input('SDFOPEN: Duration of one block in seconds: '); 
                EDF.Dur = tmp;
                EDF.SPR=EDF.Dur*EDF.SampleRate;
        else
                if ~isempty(strfind(upper(arg4),'RAW'))
                        EDF.SIE.RAW = 1;
                else
                        EDF.Dur = arg4;
                        EDF.SPR=EDF.Dur*EDF.SampleRate;
                end;
        end;
        EDF.SampleRate = EDF.AS.MAXSPR/EDF.Dur;
end;

FILENAME=EDF.FileName;
PPos=min([max(find(FILENAME=='.')) length(FILENAME)+1]);
SPos=max([0 find(FILENAME==filesep)]);
EDF.FILE.Ext = FILENAME(PPos+1:length(FILENAME));
EDF.FILE.Name = FILENAME(SPos+1:PPos-1);
if SPos==0
	EDF.FILE.Path = pwd;
else
	EDF.FILE.Path = FILENAME(1:SPos-1);
end;
EDF.FileName = [EDF.FILE.Path filesep EDF.FILE.Name '.' EDF.FILE.Ext];

% Check all fields of Header1
if ~isfield(EDF,'VERSION')
        fprintf('Warning SDFOPEN-W: EDF.VERSION not defined; default=EDF assumed\n');
        EDF.VERSION='0       '; % default EDF-format
end;

if ~strcmp(EDF.VERSION(1:3),'GDF');
        %EDF.VERSION = '0       ';
        fprintf(EDF.FILE.stderr,'\nData are stored with integer16.\nMeasures for minimizing round-off errors have been taken.\nDespite, overflow and round off errors may occur.\n');  
        
        if sum(EDF.SPR)>61440/2;
                fprintf(EDF.FILE.stderr,'\nWarning SDFOPEN: One block exceeds 61440 bytes.\n')
        end;
else
        EDF.VERSION = 'GDF 1.25';       % April 15th, 2004, support of eventtable position included
end;

if ~isfield(EDF,'PID')
        fprintf(EDF.FILE.stderr,'Warning SDFOPEN-W: EDF.PID not defined\n');
        EDF.PID=setstr(32+zeros(1,80));
end;
if ~isfield(EDF,'RID')
        fprintf(EDF.FILE.stderr,'Warning SDFOPEN-W: EDF.RID not defined\n');
        EDF.RID=setstr(32+zeros(1,80));
end;
if ~isfield(EDF,'T0')
        EDF.T0=zeros(1,6);
        fprintf(EDF.FILE.stderr,'Warning SDFOPEN-W: EDF.T0 not defined\n');
end;
if ~isfield(EDF,'reserved1')
        EDF.reserved1=char(ones(1,44)*32);
else
        tmp=min(8,size(EDF.reserved1,2));
        EDF.reserved1=[EDF.reserved1(1,1:tmp), char(32+zeros(1,44-tmp))];
end;
if ~isfield(EDF,'NRec')
        EDF.NRec=-1;
end;
if ~isfield(EDF,'Dur')
        if EDF.NS>0,
                fprintf('Warning SDFOPEN-W: EDF.Dur not defined\n');
        end;
        EDF.Dur=NaN;
end;
if ~isfield(EDF,'NS')
        EDF.ERROR = sprintf('Error SDFOPEN-W: EDF.NS not defined\n');
        EDF.ErrNo = EDF.ErrNo + 128;
	return;
end;
if ~isfield(EDF,'SampleRate')
        EDF.SampleRate = NaN;
end;
if ~isfield(EDF,'SPR')
        EDF.SPR = NaN;
end;
if ~isfield(EDF,'EDF')
        EDF.EDF.SampleRate = repmat(EDF.SampleRate,EDF.NS,1);
elseif ~isfield(EDF.EDF,'SampleRate')
        EDF.EDF.SampleRate = repmat(EDF.SampleRate,EDF.NS,1);
end;

if ~EDF.NS,
elseif ~isnan(EDF.Dur) & any(isnan(EDF.SPR)) & ~any(isnan(EDF.EDF.SampleRate))
	EDF.SPR = EDF.EDF.SampleRate * EDF.Dur;
elseif ~isnan(EDF.Dur) & ~any(isnan(EDF.SPR)) & any(isnan(EDF.EDF.SampleRate))
	EDF.SampleRate = EDF.Dur * EDF.SPR;
elseif isnan(EDF.Dur) & ~any(isnan(EDF.SPR)) & ~any(isnan(EDF.EDF.SampleRate))
	EDF.Dur = EDF.SPR ./ EDF.SampleRate;
	if all(EDF.Dur(1)==EDF.Dur)
		EDF.Dur = EDF.Dur(1);
	else
		fprintf(EDF.FILE.stderr,'Warning SDFOPEN: SPR and SampleRate do not fit\n');
		[EDF.SPR,EDF.SampleRate,EDF.Dur]
	end;
elseif ~isnan(EDF.Dur) & ~any(isnan(EDF.SPR)) & ~any(isnan(EDF.EDF.SampleRate))
        %% thats ok, 
else
        EDF.ErrNo = EDF.ErrNo + 128;
	fprintf(EDF.FILE.stderr,'ERROR SDFOPEN: more than 1 of EDF.Dur, EDF.SampleRate, EDF.SPR undefined.\n');
	return; 
end;


% Check all fields of Header2
if ~isfield(EDF,'Label')
        EDF.Label=setstr(32+zeros(EDF.NS,16));
        if EDF.NS>0,
                fprintf(EDF.FILE.stderr,'Warning SDFOPEN-W: EDF.Label not defined\n');
        end;
else
        tmp=min(16,size(EDF.Label,2));
        EDF.Label = [EDF.Label(1:EDF.NS,1:tmp), char(32+zeros(EDF.NS,16-tmp))];
end;
if ~isfield(EDF,'Transducer')
        EDF.Transducer=setstr(32+zeros(EDF.NS,80));
else
        tmp=min(80,size(EDF.Transducer,2));
        EDF.Transducer=[EDF.Transducer(1:EDF.NS,1:tmp), setstr(32+zeros(EDF.NS,80-tmp))];
end;
if ~isfield(EDF,'PreFilt')
        EDF.PreFilt = setstr(32+zeros(EDF.NS,80));
        if isfield(EDF,'Filter'),
        if isfield(EDF.Filter,'LowPass') & isfield(EDF.Filter,'HighPass') & isfield(EDF.Filter,'Notch'),
                if any(length(EDF.Filter.LowPass) == [1,EDF.NS]) & any(length(EDF.Filter.HighPass) == [1,EDF.NS]) & any(length(EDF.Filter.Notch) == [1,EDF.NS])
                        for k = 1:EDF.NS,
                                k1 = min(k,length(EDF.Filter.LowPass));
                                k2 = min(k,length(EDF.Filter.HighPass));
                                k3 = min(k,length(EDF.Filter.Notch));
                                PreFilt{k,1} = sprintf('LP: %5.f Hz; HP: %5.2f Hz; Notch: %i',EDF.Filter.LowPass(k1),EDF.Filter.HighPass(k2),EDF.Filter.Notch(k3));
                        end;
                        EDF.PreFilt = strvcat(PreFilt);
                end;
        end
        end
end;
tmp = min(80,size(EDF.PreFilt,2));
EDF.PreFilt = [EDF.PreFilt(1:EDF.NS,1:tmp), setstr(32+zeros(EDF.NS,80-tmp))];

if ~isfield(EDF,'PhysDim')
        EDF.PhysDim=setstr(32+zeros(EDF.NS,8));
        if EDF.NS>0,
                fprintf(EDF.FILE.stderr,'Warning SDFOPEN-W: EDF.PhysDim not defined\n');
        end;
else
        tmp=min(8,size(EDF.PhysDim,2));
        EDF.PhysDim=[EDF.PhysDim(1:EDF.NS,1:tmp), setstr(32+zeros(EDF.NS,8-tmp))];
end;

if ~isfield(EDF,'PhysMin')
        if EDF.NS>0,
                fprintf(EDF.FILE.stderr,'Warning SDFOPEN-W: EDF.PhysMin not defined\n');
        end
        EDF.PhysMin=repmat(nan,EDF.NS,1);
else
        EDF.PhysMin=EDF.PhysMin(1:EDF.NS);
end;
if ~isfield(EDF,'PhysMax')
        if EDF.NS>0,
                fprintf('Warning SDFOPEN-W: EDF.PhysMax not defined\n');
        end;
        EDF.PhysMax=repmat(nan,EDF.NS,1);
else
        EDF.PhysMax=EDF.PhysMax(1:EDF.NS);
end;
if ~isfield(EDF,'DigMin')
        if EDF.NS>0,
                fprintf(EDF.FILE.stderr,'Warning SDFOPEN-W: EDF.DigMin not defined\n');
        end
        EDF.DigMin=repmat(nan,EDF.NS,1);
else
        EDF.DigMin=EDF.DigMin(1:EDF.NS);
end;
if ~isfield(EDF,'DigMax')
        if EDF.NS>0,
                fprintf('Warning SDFOPEN-W: EDF.DigMax not defined\n');
        end;
        EDF.DigMax=repmat(nan,EDF.NS,1);
else
        EDF.DigMax=EDF.DigMax(1:EDF.NS);
end;
if ~isfield(EDF,'SPR')
        if EDF.NS>0,
                fprintf('Warning SDFOPEN-W: EDF.SPR not defined\n');
        end;
        EDF.SPR = repmat(nan,EDF.NS,1);
        EDF.ERROR = sprintf('Error SDFOPEN-W: EDF.SPR not defined\n');
        EDF.ErrNo = EDF.ErrNo + 128;
        %fclose(EDF.FILE.FID); return;
else
        EDF.SPR=reshape(EDF.SPR(1:EDF.NS),EDF.NS,1);
end;
EDF.AS.MAXSPR = 1;
for k=1:EDF.NS,
        EDF.AS.MAXSPR = lcm(EDF.AS.MAXSPR,EDF.SPR(k));
end;

if (abs(EDF.VERSION(1))==255)  & strcmp(EDF.VERSION(2:8),'BIOSEMI'),
        EDF.GDFTYP=255+24+zeros(1,EDF.NS);

elseif strcmp(EDF.VERSION,'0       '),
        EDF.GDFTYP=3+zeros(1,EDF.NS);

elseif strcmp(EDF.VERSION(1:3),'GDF'),
	if EDF.NS == 0;
		EDF.GDFTYP = [];
	elseif ~isfield(EDF,'GDFTYP'),
	        EDF.ERROR = sprintf('Warning SDFOPEN-W: EDF.GDFTYP not defined\n');
                EDF.ErrNo = EDF.ErrNo + 128;
                % fclose(EDF.FILE.FID); return;
        else
                EDF.GDFTYP= EDF.GDFTYP(1:EDF.NS);
        end;
else
	fprintf(EDF.FILE.stderr,'Error SDFOPEN: invalid VERSION %s\n ',EDF.VERSION);
	return;
end;

%%%%%% generate Header 1, first 256 bytes 
EDF.HeadLen=(EDF.NS+1)*256;
H1=setstr(32*ones(1,256));
H1(1:8)=EDF.VERSION; %sprintf('%08i',EDF.VERSION);     % 8 Byte  Versionsnummer 
H1( 8+(1:length(EDF.PID)))=EDF.PID;       
H1(88+(1:length(EDF.RID)))=EDF.RID;
%H1(185:192)=sprintf('%-8i',EDF.HeadLen);

%%%%% Open File 
if ~any(arg2=='+') 
        [fid,MESSAGE]=fopen(FILENAME,'w+b','ieee-le');          
else  % (arg2=='w+')  % may be called only by SDFCLOSE
        if EDF.FILE.OPEN==2 
                [fid,MESSAGE]=fopen(FILENAME,'r+b','ieee-le');          
        else
                fprintf(EDF.FILE.stderr,'Error SDFOPEN-W+: Cannot open %s for write access\n',FILENAME);
                return;
        end;
end;
if fid<0 
        %fprintf(EDF.FILE.stderr,'Error EDFOPEN: %s\n',MESSAGE);  
        H1=MESSAGE;H2=[];
        EDF.ErrNo = EDF.ErrNo + 32;
        fprintf(EDF.FILE.stderr,'Error SDFOPEN-W: Could not open %s \n',FILENAME);
        return;
end;
EDF.FILE.FID = fid;
EDF.FILE.OPEN = 2;

if strcmp(EDF.VERSION(1:3),'GDF'),
        H1(168+(1:16))=sprintf('%04i%02i%02i%02i%02i%02i%02i',floor(EDF.T0),floor(100*rem(EDF.T0(6),1)));
        c=fwrite(fid,abs(H1(1:184)),'uchar');
        %c=fwrite(fid,EDF.HeadLen,'int64');
        c=fwrite(fid,[EDF.HeadLen,0],'int32');
        c=fwrite(fid,ones(8,1)*32,'uint8'); % EP_ID=ones(8,1)*32;
        c=fwrite(fid,ones(8,1)*32,'uint8'); % Lab_ID=ones(8,1)*32;
        c=fwrite(fid,ones(8,1)*32,'uint8'); % T_ID=ones(8,1)*32;
        c=fwrite(fid,ones(20,1)*32,'uint8'); % 
        %c=fwrite(fid,EDF.NRec,'int64');
        c=fwrite(fid,[EDF.NRec,0],'int32');
        %fwrite(fid,EDF.Dur,'float64');
        [n,d]=rat(EDF.Dur); fwrite(fid,[n d], 'uint32');
	c=fwrite(fid,EDF.NS,'uint32');
else
        H1(168+(1:16))=sprintf('%02i.%02i.%02i%02i:%02i:%02i',floor(rem(EDF.T0([3 2 1 4 5 6]),100)));
        H1(185:192)=sprintf('%-8i',EDF.HeadLen);
        H1(193:236)=EDF.reserved1;
        H1(237:244)=sprintf('%-8i',EDF.NRec);
        H1(245:252)=sprintf('%-8i',EDF.Dur);
        H1(253:256)=sprintf('%-4i',EDF.NS);
        H1(abs(H1)==0)=char(32); 
        c=fwrite(fid,abs(H1),'uchar');
end;

%%%%%% generate Header 2,  NS*256 bytes 
if ~strcmp(EDF.VERSION(1:3),'GDF');
        sPhysMax=setstr(32+zeros(EDF.NS,8));
        for k=1:EDF.NS,
                tmp=sprintf('%-8g',EDF.PhysMin(k));
                lt=length(tmp);
                if lt<9
                        sPhysMin(k,1:lt)=tmp;
                else
                        if any(upper(tmp)=='E') | find(tmp=='.')>8,
                                fprintf(EDF.FILE.stderr,'Error SDFOPEN-W: PhysMin(%i) does not fit into header\n', k);
                        else
                                sPhysMin(k,:)=tmp(1:8);
                        end;
                end;
                tmp=sprintf('%-8g',EDF.PhysMax(k));
                lt=length(tmp);
                if lt<9
                        sPhysMax(k,1:lt)=tmp;
                else
                        if any(upper(tmp)=='E') | find(tmp=='.')>8,
                                fprintf(EDF.FILE.stderr,'Error SDFOPEN-W: PhysMin(%i) does not fit into header\n', k);
                        else
                                sPhysMax(k,:)=tmp(1:8);
                        end;
                end;
        end;
        sPhysMin=reshape(sprintf('%-8.1f',EDF.PhysMin)',8,EDF.NS)';
        sPhysMax=reshape(sprintf('%-8.1f',EDF.PhysMax)',8,EDF.NS)';
        
        idx1=cumsum([0 H2idx]);
        idx2=EDF.NS*idx1;
        h2=setstr(32*ones(EDF.NS,256));
        size(h2);
        h2(:,idx1(1)+1:idx1(2))=EDF.Label;
        h2(:,idx1(2)+1:idx1(3))=EDF.Transducer;
        h2(:,idx1(3)+1:idx1(4))=EDF.PhysDim;
        %h2(:,idx1(4)+1:idx1(5))=sPhysMin;
        %h2(:,idx1(5)+1:idx1(6))=sPhysMax;
        h2(:,idx1(4)+1:idx1(5))=sPhysMin;
        h2(:,idx1(5)+1:idx1(6))=sPhysMax;
        h2(:,idx1(6)+1:idx1(7))=reshape(sprintf('%-8i',EDF.DigMin)',8,EDF.NS)';
        h2(:,idx1(7)+1:idx1(8))=reshape(sprintf('%-8i',EDF.DigMax)',8,EDF.NS)';
        h2(:,idx1(8)+1:idx1(9))=EDF.PreFilt;
        h2(:,idx1(9)+1:idx1(10))=reshape(sprintf('%-8i',EDF.SPR)',8,EDF.NS)';
        h2(abs(h2)==0)=char(32);
        for k=1:length(H2idx);
                fwrite(fid,abs(h2(:,idx1(k)+1:idx1(k+1)))','uchar');
        end;
else
        fwrite(fid, abs(EDF.Label)','uchar');
        fwrite(fid, abs(EDF.Transducer)','uchar');
        fwrite(fid, abs(EDF.PhysDim)','uchar');
        fwrite(fid, EDF.PhysMin,'float64');
        fwrite(fid, EDF.PhysMax,'float64');
	if exist('OCTAVE_VERSION')>4,  % Octave does not support INT64 yet. 
        	fwrite(fid, [EDF.DigMin,-(EDF.DigMin<0)]','int32');
	        fwrite(fid, [EDF.DigMax,-(EDF.DigMax<0)]','int32');
        else
		fwrite(fid, EDF.DigMin, 'int64');
	        fwrite(fid, EDF.DigMax, 'int64');
	end;

        fwrite(fid, abs(EDF.PreFilt)','uchar');
        fwrite(fid, EDF.SPR,'uint32');
        fwrite(fid, EDF.GDFTYP,'uint32');
        fprintf(fid,'%c',32*ones(32,EDF.NS));
end;

tmp = ftell(EDF.FILE.FID);
if tmp ~= (256 * (EDF.NS+1)) 
        fprintf(1,'Warning SDFOPEN-WRITE: incorrect header length %i bytes\n',tmp);
%else   fprintf(1,'sdfopen in write mode: header info stored correctly\n');
end;        

EDF.AS.spb = sum(EDF.SPR);	% Samples per Block
EDF.AS.bi  = [0;cumsum(EDF.SPR)];
EDF.AS.BPR = ceil(EDF.SPR(:).*GDFTYP_BYTE(EDF.GDFTYP(:)+1)');
EDF.AS.SAMECHANTYP = all(EDF.AS.BPR == (EDF.SPR(:).*GDFTYP_BYTE(EDF.GDFTYP(:)+1)')) & ~any(diff(EDF.GDFTYP));
%EDF.AS.GDFbi= [0;cumsum(EDF.AS.BPR)];
EDF.AS.GDFbi = [0;cumsum(ceil(EDF.SPR(:).*GDFTYP_BYTE(EDF.GDFTYP(:)+1)'))];
EDF.AS.bpb   = sum(ceil(EDF.SPR(:).*GDFTYP_BYTE(EDF.GDFTYP(:)+1)'));	% Bytes per Block
EDF.AS.startrec = 0;
EDF.AS.numrec = 0;
EDF.FILE.POS  = 0;

else % if arg2 is not 'r' or 'w'
        fprintf(EDF.FILE.stderr,'Warning SDFOPEN: Incorrect 2nd argument. \n');
end;        

if EDF.ErrNo>0
        fprintf(EDF.FILE.stderr,'ERROR %i SDFOPEN\n',EDF.ErrNo);
end;

