function [BKR,s]=bkropen(arg1,PERMISSION,CHAN,arg4,arg5,arg6)
% BKROPEN opens BKR file
% [BKR,signal] = bkropen(Filename, PERMISSION, ChanList);
% [s,BKR] = eegread(BKR [,NoS [,Startpos]])
% 
% FILENAME 
% PERMISSION is one of the following strings 
%	'r'	read 
% ChanList	(List of) Channel(s) default 0 loads all channels
%
% see also: SOPEN, SREAD, SSEEK, STELL, SCLOSE, SWRITE, SEOF

%	$Revision: 1.28 $
%	$Id: bkropen.m,v 1.28 2005-01-20 10:18:48 schloegl Exp $
%	Copyright (c) 1997-2005 by Alois Schloegl
%	a.schloegl@ieee.org	

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

if nargin<2, 
        PERMISSION='rb'; 
elseif ~any(PERMISSION=='b');
        PERMISSION = [PERMISSION,'b']; % force binary open. 
end;
if nargin<3, CHAN=0; end;

if isstruct(arg1),
	BKR=arg1;
	FILENAME=BKR.FileName;
	if BKR.FILE.OPEN,
		fseek(BKR.FILE.FID,0,'bof');	
	else
		BKR.FILE.FID = fopen(FILENAME,PERMISSION,'ieee-le');          
	end;
else
	FILENAME=arg1;
	BKR.FILE.FID = fopen(FILENAME,PERMISSION,'ieee-le');          
        BKR.FileName  = FILENAME;
        [pfad,file,FileExt] = fileparts(BKR.FileName);
        BKR.FILE.Name = file;
        BKR.FILE.Path = pfad;
        BKR.FILE.Ext  = FileExt(2:length(FileExt));
end;
if BKR.FILE.FID<0,
        fprintf(2,'Error BKROPEN: file %s not found.\n',FILENAME); 
        return;
end;
if ftell(BKR.FILE.FID)~=0,	% 
        fprintf(2,'Error: Fileposition is not 0\n');        	        
end;

BOOL='int16';
ULONG='uint32'; 
FLOAT='float32';

if any(PERMISSION=='r'),
	BKR.FILE.OPEN = 1;
	fid = BKR.FILE.FID;
%%%%% READ HEADER

%VARIABLE	TYPE			#bytes	OFFSET	COMMENT
	VER=fread(fid,1,'ushort');	%	2 Byte	0	Versionsnummer 
	if ((VER<=200) | (VER>207)) fprintf(2,'LOADBKR: WARNING  Version BKR Format %i',VER); end;
	nch=fread(fid,1,'ushort');	%	2 Byte	2	Anzahl der Kanäle
	nhz=fread(fid,1,'ushort');	%	2 Byte	4	Abtastfrequenz	
	ntr=fread(fid,1, 'uint32');	%	4 Byte	6	Anzahl der Trials
	nsp=fread(fid,1, 'uint32');	%	4 Byte	10	Anzahl Samples per Trial
	cvlt=fread(fid,1,'ushort');	%	2 Byte	14	Kalibrierspannung	
	cval=fread(fid,1,'ushort');	%	2 Byte	16	Kalibrierwert
	code=fread(fid,4,'char');	%	4 Byte	18	Elektrodencode
	code=setstr(code)';
	lcf=fread(fid,1, FLOAT);	%	4 Byte	22	untere Eckfrequenz
	ucf=fread(fid,1, FLOAT);	%	4 Byte	26	obere Eckfrequenz
	BKR.BKR.sref=fread(fid,1, ULONG);	%	4 Byte	30	Startzeitpunkt Referenz in Samples
	BKR.BKR.eref=fread(fid,1, ULONG);	%	4 Byte	34	Länge Referenz in Samples	
	BKR.BKR.sact=fread(fid,1, ULONG);	%	4 Byte	38	Startzeitpunkt Aktion in Samples
	BKR.BKR.eact=fread(fid,1, ULONG);	%	4 Byte	42	Länge Aktion in Samples
	trg=fread(fid,1,BOOL);		%	2 Byte	46	flag für Trigger
	BKR.BKR.pre=fread(fid,1, ULONG);	%	4 Byte	48	Anzahl der Sampels vor dem Trigger
	BKR.BKR.pst=fread(fid,1, ULONG);	%	4 Byte	52	Anzahl der Sampels nach dem Trigger
	BKR.BKR.hav=fread(fid,1,BOOL);		%	2 Byte	56	flag für "horizontale" Mittelung
	BKR.BKR.nah=fread(fid,1, ULONG);	%	4 Byte	58	Anzahl der gemittelten Trials
	BKR.BKR.vav=fread(fid,1,BOOL);		%	2 Byte	62	flag für "vertikale" Mittelung	
	BKR.BKR.nav=fread(fid,1,'ushort');	%	2 Byte	64	Anzahl der gemittelten Kanäle
	BKR.BKR.cav=fread(fid,1,BOOL);		%	2 Byte	66	flag für Datenkomprimierung
	BKR.BKR.nac=fread(fid,1, ULONG);	%	4 Byte	68	Anzahl der gemittelten Samples
	com=fread(fid,1,BOOL);		%	2 Byte	72	flag: Common Average Reference
	loc=fread(fid,1,BOOL);		%	2 Byte	74	flag: Local Average Reference
	lap=fread(fid,1,BOOL);		%	2 Byte	76	flag: Laplace Berechnung
	wgt=fread(fid,1,BOOL);		%	2 Byte	78	flag: Weighted Average Reference
	BKR.BKR.pwr=fread(fid,1,BOOL);		%	2 Byte	80	flag: Leistung
	BKR.BKR.avr=fread(fid,1,BOOL);		%	2 Byte	82	flag: Mittelwert
	BKR.BKR.std=fread(fid,1,BOOL);		%	2 Byte	84	flag: Streuung
	BKR.BKR.bps=fread(fid,1,BOOL);		%	2 Byte	86	flag: Bandpaß
	BKR.BKR.erd=fread(fid,1,BOOL);		%	2 Byte	88	flag: ERD
	BKR.BKR.sig=fread(fid,1,BOOL);		%	2 Byte	90	flag: Signifikanz 
	BKR.BKR.coh=fread(fid,1,BOOL);		%	2 Byte	92	flag: Kohärenz
	BKR.BKR.spc=fread(fid,1,BOOL);		%	2 Byte	94	flag: Spectrum
	BKR.BKR.conf=fread(fid,1, FLOAT);	%	4 Byte	96	Konfidenz
	BKR.BKR.csp=fread(fid,1,BOOL);		%	2 Byte	100	flag: Kohärenz Leistungsspektrum
	BKR.BKR.erc=fread(fid,1,BOOL);		%	2 Byte	102	flag: ERC
	BKR.BKR.ham=fread(fid,1,BOOL);		%	2 Byte	104	flag: Hanning smoothed
	BKR.BKR.ann=fread(fid,1,BOOL);		%	2 Byte	106	flag: art. Neuronal. NW. Filter (ANN)
	niu=fread(fid,1,'ushort');	%	2 Byte 	108	ANN: Anzahl input units 
	nhu=fread(fid,1,'ushort');	%	2 Byte	110	ANN: Anzahl hidden units
	nlc=fread(fid,1, ULONG);	%	4 Byte	112	ANN: Anzahl Lernzyklen
	reg=fread(fid,1, FLOAT);	%	4 Byte	116	ANN: regression
	lco=fread(fid,1, FLOAT);	%	4 Byte 	120	ANN: Lernkoeffizient
	epo=fread(fid,1,'ushort');	%	2 Byte	124	ANN: Epoche
	BKR.BKR.rel=fread(fid,1,BOOL);		%	2 Byte	126	flag: ERC in Relativwerten
	wnd=fread(fid,1,'ushort');	%	2 Byte	128	Fenstertyp
	BKR.BKR.kal=fread(fid,1,BOOL);		%	2 Byte	130	flag: Kalman gefilterte Daten
	BKR.BKR.cwt=fread(fid,1,BOOL);		%	2 Byte	132	flag: kont. Wavelet transformtiert
	cwt_fmin=fread(fid,1, FLOAT);	%	4 Byte	134	unterste Frequenz, kont. Wavelettransform.
	cwt_fmax=fread(fid,1, FLOAT);	%	4 Byte	138	oberste Frequenz, kont. Wavelettransform.
	scales=fread(fid,1,'ushort');	%	2 Byte	142	Anzahl Frequenzbänder für kont. WT
	cwt_fe=fread(fid,1, FLOAT);	%	4 Byte	144	frequ. für Dt = Df = 1/2Öp
	cwt_start=fread(fid,1, ULONG);	%	4 Byte	148	Startsample für kont. WT Berechnung
        %-- NULL --	-------------	--------	152	-- Offset bis 512 Byte --
        if 1, 
                fread(fid,1024-152,'char');
        else
                fread(fid,512-152,'char');
                for i=1:nch,
                        eletyp(i)=fread(fid,1,'uchar');	%	1 Byte	512	Elektrode 1: Signalart (z.B: EEG)
                        elenum(i)=fread(fid,1,'uchar');	%	1 Byte	513	Elektrode 1: Kanalnr. für gleiche Signalart
                        ref(i)=fread(fid,1, FLOAT);	%	4 Byte	514	Referenzwert für Kanal 1
                end;
                if nch>85,
                        fprintf(2,'Warning BKRLOAD: Number of channels larger than 85; Header does not support more\n');
                end;
                fseek(fid,512-(nch*6),'cof');
        end;
        HeaderEnd = ftell(fid);
	if HeaderEnd~=1024,
	        fprintf(2,'Warning BKRLOAD: Length of Header does not confirm BKR-specification\n');
	end;
	%HeaderEnd = 1024;

	%%%%% Generate BKR-Struct; similar to EDF-struct
	BKR.VERSION=VER;
	BKR.NS=nch;
	BKR.NRec=ntr;
	BKR.Dur=1/nhz;

	BKR.SampleRate=nhz;%(ones(BKR.NS,1));
	BKR.SPR=nsp;%(ones(BKR.NS,1));
	BKR.DigMax=cval;
	BKR.PhysMax=cvlt;
	BKR.Cal=BKR.PhysMax/BKR.DigMax; %*ones(BKR.NS,1);
	BKR.Off=zeros(BKR.NS,1);
        BKR.Calib = sparse(2:BKR.NS+1,1:BKR.NS,BKR.Cal,BKR.NS+1,BKR.NS);
        BKR.PhysDim = repmat('µV',BKR.NS,1);
	BKR.Label=code;
	tmp=sprintf('LowPass %4.1f Hz; HighPass %4.1f Hz; Notch ?',lcf,ucf);
	BKR.PreFilt=tmp;%ones(BKR.NS,1)*[tmp 32+zeros(1,80-length(tmp))];
	BKR.HeadLen=HeaderEnd;

	BKR.AS.startrec = 0;
	BKR.AS.numrec = 0;
	BKR.AS.bpb = BKR.NS*2;	% Bytes per Block
	BKR.AS.spb = BKR.NS;	% Samples per Block
	BKR.FILE.POS = 0;

	BKR.Filter.LowPass  = lcf;
	BKR.Filter.HighPass = ucf;
	BKR.Filter.Notch    = nan; %h.notchfilter;
        
        BKR.FLAG.TRIGGERED = trg;
	if ~BKR.FLAG.TRIGGERED & (BKR.NRec>1);
		fprintf(BKR.FILE.stderr,'Warning: TriggerFlag in file %s was not set.\n',BKR.FileName);
		BKR.FLAG.TRIGGERED = 1;
	end;	
        BKR.FLAG.REFERENCE = 'unknown';
        if com, BKR.FLAG.REFERENCE = 'COM'; end;
        if loc, BKR.FLAG.REFERENCE = 'LOC'; end;
        if lap, BKR.FLAG.REFERENCE = 'LAP'; end;
        if wgt, BKR.FLAG.REFERENCE = 'WGT'; end;
                
        if (CHAN==0), 
                CHAN = 1:BKR.NS; 
        end;

        % THRESHOLD for Overflow detection
        BKR.SIE.THRESHOLD = -(2^15);
        BKR.THRESHOLD = repmat([-1,1]*BKR.DigMax,BKR.NS,1);

	%status = fseek(fid,0,'eof');
        BKR.data = fread(fid,[BKR.NS,inf],'int16=>int16')';
	EndPos = ftell(fid);
        %status = fseek(fid,BKR.HeadLen,'bof');
	BKR.AS.endpos = (EndPos-BKR.HeadLen)/BKR.AS.bpb;
	fclose(fid);
	BKR.FILE.OPEN = 1;

        % check whether Headerinfo fits to file length.
	if (EndPos-BKR.HeadLen)~=BKR.SPR*BKR.NRec*BKR.NS*2,
		[EndPos,BKR.HeadLen,BKR.SPR,BKR.NRec,BKR.NS],
		[EndPos-BKR.HeadLen-BKR.SPR*BKR.NRec*BKR.NS*2],
		fprintf(2,'Header information in %s corrupted; Data could be reconstructed.\n',BKR.FileName);
		if BKR.NRec==1,
			BKR.SPR=(EndPos-HeaderEnd)/(BKR.NRec*BKR.NS*2);
        	end;
        end;

        % look for Classlabel information
        if ~isfield(BKR,'Classlabel'),
                BKR.Classlabel = [];
        end;
        if ~isfield(BKR,'TRIG'),
                BKR.TRIG = [];
        end;
        tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.mat']);
        if ~exist(tmp,'file'),
                tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.MAT']);
        end
        x = [];
        if exist(tmp,'file'),
                x = load('-mat',tmp);
        end;
        if isfield(x,'header'),
                BKR.MAT = x.header;
                if isfield(x.header,'Result') & isfield(x.header.Result,'Classlabel'),
                        BKR.Classlabel = x.header.Result.Classlabel;
                end;
                if isfield(x.header,'Paradigm')
                        if isempty(BKR.Classlabel) & isfield(x.header.Paradigm,'Classlabel')
                                BKR.Classlabel = x.header.Paradigm.Classlabel;
                        end;
                        BKR.BCI.Paradigm = x.header.Paradigm;
                        if isfield(BKR.BCI.Paradigm,'TriggerOnset');
                                BKR.TriggerOffset = BKR.BCI.Paradigm.TriggerOnset;
                        elseif isfield(BKR.BCI.Paradigm,'TriggerTiming');
                            %    BKR.BCI.Paradigm.TriggerTiming,
                                BKR.TriggerOffset = BKR.BCI.Paradigm.TriggerTiming;
                                fprintf(2,'Warning BKROPEN: Paradigm.TriggerOnset is unknown. Paradigm.TriggerTiming= %f ms is used instead\n',BKR.TriggerOffset);
                        end;
                end;

                if isfield(x.header,'PhysioRec'), % R. Leeb's data 
                        BKR.Label = x.header.PhysioRec;
                end;
                if isfield(x.header,'BKRHeader'), % R. Scherer Data 
                        if isfield(x.header.BKRHeader,'TO'),
                                BKR.T0 = x.header.BKRHeader.TO;
                        end;
                        if isfield(x.header.BKRHeader,'Label'),
                                BKR.Label = x.header.BKRHeader.Label;
                                ns = BKR.NS-size(BKR.Label,1);
                                if ns == 1;
                                        BKR.Label = strvcat(BKR.Label,'TRIGGER');
                                elseif ns > 1;
                                        BKR.Label = strvcat(BKR.Label,char(repmat('n.a.',ns,1)));
                                end;
                        end;
                end;
                if isfield(x.header,'Model'), % More 
                        if isfield(x.header.Model,'AnalogInput'), 
                                for k = 1:length(x.header.Model.AnalogInput),
                                        BKR.Filter.HighPass(k) = x.header.Model.AnalogInput{k}{5};
                                        BKR.Filter.LowPass(k)  = x.header.Model.AnalogInput{k}{6};
                                        BKR.Filter.Notch(k)    = strcmpi(x.header.Model.AnalogInput{k}{7},'on');

                                        BKR.MAT.Cal(k) = x.header.Model.AnalogInput{k}{3};
                                end
                        end;
                end;
                if ~isempty(strmatch('TRIGGER',BKR.Label))
                        BKR.AS.TRIGCHAN = BKR.NS; %strmatch('TRIGGER',H.Label); 
                end;
        end;
        if 1; %~isfield(BKR,'Classlabel'),
                tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.par']);
                if ~exist(tmp,'file'),
                	tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.PAR']);
                end
                if exist(tmp,'file'),
                        BKR.Classlabel = load(tmp);
                end;
        end;

        %%% Artifact Selection files 
        tmp1=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.sel']);
        if ~exist(tmp1,'file'),
                tmp1=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.SEL']);
        end
        tmp2 = fullfile(BKR.FILE.Path,[BKR.FILE.Name,'_artifact.mat']);
        SW   = (exist(tmp1,'file')>0) + 2*(exist(tmp2,'file')>0);
        if SW == 0, 
        elseif SW == 1,
                if exist('OCTAVE_VERSION')>5
                        BKR.ArtifactSelection = load('-ascii',tmp1);
                else
                        BKR.ArtifactSelection = load(tmp1);
                end;
        elseif SW == 2,
                if exist('OCTAVE_VERSION')>5
                        tmp = load('-mat',tmp2);
                else
                        tmp = load(tmp2);
                end;
                BKR.ArtifactSelection = tmp.artifact(:);
        elseif SW == 3,
                fprintf(HDR.FILE.stderr,'Warning BKROPEN: more than one ArtifactSelection files. File %s is used.\n',tmp1);
                if exist('OCTAVE_VERSION')>5
                        BKR.ArtifactSelection = load('-ascii',tmp1);
                else
                        BKR.ArtifactSelection = load(tmp1);
                end;
        end;
        if isfield(BKR,'ArtifactSelection'),
                if any(BKR.ArtifactSelection>1) | (length(BKR.ArtifactSelection)<length(BKR.Classlabel))
                        sel = zeros(size(BKR.Classlabel));
                        sel(BKR.ArtifactSelection) = 1;
                        BKR.ArtifactSelection = sel(:);
                end;
                BKR.ArtifactSelection = BKR.ArtifactSelection(:);
        end;
        
        
        if isfield(BKR.AS,'TRIGCHAN') % & isempty(BKR.EVENT.POS)
                if BKR.AS.TRIGCHAN<=size(BKR.data,2),
                        BKR.THRESHOLD(BKR.AS.TRIGCHAN,:)=NaN; % do not apply overflow detection for Trigger channel 
                        BKR.TRIG = gettrigger(double(BKR.data(:,BKR.AS.TRIGCHAN)));
                        if isfield(BKR,'TriggerOffset')
                                BKR.TRIG = BKR.TRIG - round(BKR.TriggerOffset/1000*BKR.SampleRate);
                        end;
                end;
        end;
	BKR.TYPE = 'native'; 
        if length(BKR.TRIG)~=length(BKR.Classlabel),
                % hack to deal with BCI22 data
                fprintf(2,'Warning BKROPEN: Number of triggers (%i) and number of Classlabels (%i) do not fit\n',length(BKR.TRIG),length(BKR.Classlabel));
                BKR.TRIG = [];
                BKR.Classlabel = [];
                BKR.ArtifactSelection = [];
        end;

        
elseif any(PERMISSION=='w'),
        
        BKR.FILE.OPEN = 2;		
        BKR.VERSION   = 207;
        BKR.TYPE      = 'BKR';
        if ~strcmpi(BKR.FILE.Ext,'BKR'),
                fprintf(2,'Warning BKROPEN-WRITE: file extionsion is not BKR.\n');
        end;
        if ~isfield(BKR,'SampleRate'),
                fprintf(2,'Error BKROPEN-WRITE: HDR.SampleRate not defined.\n');
                return;
        end;
        if ~isfield(BKR,'NS'),
                BKR.NS = 0; 	% unknown channel number ...
        end;
        if ~isfield(BKR,'SPR'),
                BKR.SPR = 0; 	% Unknown - Value will be fixed when file is closed. 
        end;
        if isfield(BKR,'NRec'),
                BKR.FLAG.TRIGGERED = BKR.NRec>1;
        else
                BKR.NRec = -1; 	% Unknown - Value will be fixed when file is closed. 
        end;
        if ~isfield(BKR,'PhysMax'), BKR.PhysMax = NaN; end;
        if isempty(BKR.PhysMax),    BKR.PhysMax = NaN; end;
        if ~isfield(BKR,'DigMax'),  BKR.DigMax  = NaN; end;
        if isnan(BKR.DigMax) | isempty(BKR.DigMax),
                BKR.DigMax = 2^15-1;   
        end;
        
        if any([BKR.NS==0,BKR.SPR==0,BKR.NRec<0,isnan([BKR.NRec,BKR.NS,BKR.SPR,BKR.DigMax,BKR.PhysMax,BKR.SampleRate])]), 	% if any unknown, ...	
                BKR.FILE.OPEN = 3;			%	... fix header when file is closed. 
        end;
        if ~isfield(BKR,'FLAG'),
                BKR.FLAG.UCAL = 0; 
        elseif ~isfield(BKR.FLAG,'UCAL'),
                BKR.FLAG.UCAL = 0; 
        end;

        tmp = round(BKR.PhysMax);
	fprintf(1,'Scaling error in file %s due to rounding of PhysMax is in the range of %f%%.\n',BKR.FileName, abs((BKR.PhysMax-tmp)/tmp)*100);
	BKR.PhysMax = tmp;

        % THRESHOLD for Overflow detection
        BKR.SIE.THRESHOLD = -(2^15);
        
	count=fwrite(BKR.FILE.FID,BKR.VERSION,'short');	        % version number of header
	count=fwrite(BKR.FILE.FID,BKR.NS,'short');	        % number of channels
	count=fwrite(BKR.FILE.FID,BKR.SampleRate,'short');      % sampling rate
	count=fwrite(BKR.FILE.FID,BKR.NRec,'int32');            % number of trials: 1 for untriggered data
	count=fwrite(BKR.FILE.FID,BKR.SPR,'uint32');            % samples/trial/channel
	count=fwrite(BKR.FILE.FID,BKR.PhysMax,'short');		% Kalibrierspannung
	count=fwrite(BKR.FILE.FID,BKR.DigMax, 'short');		% Kalibrierwert
        
	count=fwrite(BKR.FILE.FID,zeros(4,1),'char');        
	if isfield(BKR,'Filter'),
		if ~isfield(BKR.Filter,'LowPass'),
			BKR.Filter.LowPass = NaN; 
		end;        
		if ~isfield(BKR.Filter,'HighPass'),
			BKR.Filter.HighPass= NaN; 
		end;
	else
		BKR.Filter.LowPass =NaN; 
		BKR.Filter.HighPass=NaN; 
        end;
	if all(BKR.Filter.LowPass(1)==BKR.Filter.LowPass)
		BKR.Filter.LowPass = BKR.Filter.LowPass(1);
	else
		BKR.Filter.LowPass = NaN;
	end;
	if all(BKR.Filter.HighPass(1)==BKR.Filter.HighPass)
		BKR.Filter.HighPass = BKR.Filter.HighPass(1);
	else
		BKR.Filter.HighPass = NaN;
	end;
	count=fwrite(BKR.FILE.FID,[BKR.Filter.LowPass,BKR.Filter.HighPass],'float'); 

	count=fwrite(BKR.FILE.FID,zeros(16,1),'char');         	% offset 30
	count=fwrite(BKR.FILE.FID,BKR.FLAG.TRIGGERED,'int16');	% offset 32
	count=fwrite(BKR.FILE.FID,zeros(24,1),'char');         	% offset 46
        
        if ~isfield(BKR,'FLAG')
                BKR.FLAG.REFERENCE='';
        end;
        if ~isfield(BKR.FLAG,'REFERENCE')
                BKR.FLAG.REFERENCE='';
        end;
        
        tmp  = [strcmp(BKR.FLAG.REFERENCE,'COM')|strcmp(BKR.FLAG.REFERENCE,'CAR'), strcmp(BKR.FLAG.REFERENCE,'LOC')|strcmp(BKR.FLAG.REFERENCE,'LAR'), strcmp(BKR.FLAG.REFERENCE,'LAP'), strcmp(BKR.FLAG.REFERENCE,'WGT')];
        
        fwrite(BKR.FILE.FID,tmp,BOOL); 		% offset 72 + 4*BOOL
                
	%speichert den rest des BKR-headers
	count = fwrite(BKR.FILE.FID,zeros(1024-80,1),'char');
	BKR.HeadLen = ftell(BKR.FILE.FID);
	if BKR.HeadLen~=1024,
		fprintf(2,'Error BKROPEN WRITE: HeaderLength is not 1024 but %i\n',BKR.HeadLen);
	end;
	BKR.FILE.POS  = 0;
	BKR.AS.endpos = 0;
	BKR.AS.bpb = BKR.NS*2;	% Bytes per Block
        BKR.AS.spb = BKR.NS;	% Samples per Block
        
        if isfield(BKR,'Classlabel');
                fid = fopen(fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.par']),'w+b');  % force binary mode
                fprintf(fid,'%i\r\n',BKR.Classlabel);  % explicit 0x0d and 0x0a
                fclose(fid);
        end;
        if isfield(BKR,'ArtifactSelection');
                fid = fopen(fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.sel']),'w+b');  % force binary mode
                fprintf(fid,'%i\r\n',BKR.ArtifactSelection);  % explicit 0x0d and 0x0a
                fclose(fid);
        end;
end;


