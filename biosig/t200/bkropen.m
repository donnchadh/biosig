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

%	$Revision: 1.15 $
%	$Id: bkropen.m,v 1.15 2004-02-26 15:03:43 schloegl Exp $
%	Copyright (c) 1997-2003 by  Alois Schloegl
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
	if BKR.FILE.FID<0,
		fprintf(2,'Error BKROPEN: file %s not found.\n',FILENAME); 
		return;
	end;
        BKR.FileName  = FILENAME;
        [pfad,file,FileExt] = fileparts(BKR.FileName);
        BKR.FILE.Name = file;
        BKR.FILE.Path = pfad;
        BKR.FILE.Ext  = FileExt(2:length(FileExt));
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
	HeaderEnd=ftell(fid);
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
	BKR.Label=code;
	tmp=sprintf('LowPass %4.1fHz, HighPass %4.1fHz, Notch ?',lcf,ucf);
	BKR.PreFilt=tmp;%ones(BKR.NS,1)*[tmp 32+zeros(1,80-length(tmp))];
	BKR.HeadLen=HeaderEnd;

	BKR.AS.startrec = 0;
	BKR.AS.numrec = 0;
	BKR.AS.bpb = BKR.NS*2;	% Bytes per Block
	BKR.AS.spb = BKR.NS;	% Samples per Block
	BKR.FILE.POS = 0;

	%BKR.BKR.ham=ham;
	%BKR.BKR.spc=spc;
	%BKR.AS.sref=sref;
	%BKR.AS.eref=eref;
	%BKR.AS.eletyp=eletyp;
	%BKR.AS.elenum=elenum;
	%BKR.AS.ref=ref;

	BKR.Filter.LowPass  = lcf;
	BKR.Filter.HighPass = ucf;
	BKR.Filter.Notch    = nan; %h.notchfilter;
	%BKR.Filter.NotchOn = h.filterflag;
	%BKR.Filter.ON   = [e(:).filtered];
        
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
                
	if CHAN<1, CHAN=1:BKR.NS; end;
	BKR.SIE.ChanSelect = CHAN;
	BKR.SIE.InChanSelect = CHAN;

	% check whether Headerinfo fits to file length.
	status = fseek(fid,0,'eof');
	EndPos = ftell(fid);
	BKR.AS.endpos = (EndPos-BKR.HeadLen)/BKR.AS.bpb;
	status = fseek(fid,BKR.HeadLen,'bof');
	if (EndPos-BKR.HeadLen)~=BKR.SPR*BKR.NRec*BKR.NS*2,
		[EndPos,BKR.HeadLen,BKR.SPR,BKR.NRec,BKR.NS],
		[EndPos-BKR.HeadLen-BKR.SPR*BKR.NRec*BKR.NS*2],
		warning(['Header information in ',BKR.FileName,' corrupted; Data could be reconstructed.']);
		if BKR.NRec==1,
			BKR.SPR=(EndPos-HeaderEnd)/(BKR.NRec*BKR.NS*2);
        	end;
        end;
        
        % look for Classlabel information
        if 1; %~isfield(BKR,'Classlabel'),
                tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.par']);
                if exist(tmp)~=2,
                	tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.PAR']);
                end
                if exist(tmp)==2,
                        BKR.Classlabel = load(tmp);
                end;
        end;
        if 1; %~isfield(BKR,'ArtifactSelection'),
                tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.sel']);
                if exist(tmp)~=2,
                	tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.SEL']);
                end
                if exist(tmp)==2,
                        BKR.ArtifactSelection = load(tmp);
                end;
        end;
        if ~isfield(BKR,'Classlabel'),
                tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.mat']);
                if exist(tmp)~=2,
                	tmp=fullfile(BKR.FILE.Path,[BKR.FILE.Name,'.MAT']);
                end
                if exist(tmp)==2,
                        try,
                                tmp = load(tmp);
                        catch
                                tmp = [];
                        end
                        if isfield(tmp,'header'),
                                BKR.Classlabel = tmp.header.Result.Classlabel;
                        end;
                end;
        end;


elseif any(PERMISSION=='w'),
        
        BKR.FILE.OPEN = 2;		
        BKR.VERSION   = 207;
        if ~isfield(BKR,'NS'),
                BKR.NS = 0; 	% unknown channel number ...
        end;
        if ~isfield(BKR,'SPR'),
                BKR.SPR = 0; 	% Unknown - Value will be fixed when file is closed. 
        end;
        if ~isfield(BKR,'NRec'),
                BKR.NRec = -1; 	% Unknown - Value will be fixed when file is closed. 
		if isfield(BKR,'FLAG');
                if isfield(BKR.FLAG,'TRIGGERED');
                        BKR.NRec = sign(BKR.FLAG.TRIGGERED-.5);
        	end;
    		end;
        end;
        if any([BKR.NS==0,BKR.SPR==0,BKR.NRec<0]), 	% if any unknown, ...	
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
        
	count=fwrite(BKR.FILE.FID,BKR.VERSION,'short');	        % version number of header
	count=fwrite(BKR.FILE.FID,BKR.NS,'short');	        % number of channels
	count=fwrite(BKR.FILE.FID,BKR.SampleRate,'short');      % sampling rate
	count=fwrite(BKR.FILE.FID,BKR.NRec,'uint32');           % number of trials: 1 for untriggered data
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

	count=fwrite(BKR.FILE.FID,[BKR.Filter.LowPass,BKR.Filter.HighPass],'float'); 

	count=fwrite(BKR.FILE.FID,zeros(16,1),'char');         	% offset 30
	count=fwrite(BKR.FILE.FID,BKR.FLAG.TRIGGERED,'int16');	% offset 32
	count=fwrite(BKR.FILE.FID,zeros(24,1),'char');         	% offset 46
        
        tmp  = [strcmp(BKR.FLAG.REFERENCE,'COM')|strcmp(BKR.FLAG.REFERENCE,'CAR'), strcmp(BKR.FLAG.REFERENCE,'LOC')|strcmp(BKR.FLAG.REFERENCE,'LAR'), strcmp(BKR.FLAG.REFERENCE,'LAP'), strcmp(BKR.FLAG.REFERENCE,'WGT')];
        
        fwrite(BKR.FILE.FID,tmp,BOOL); 		% offset 72 + 4*BOOL
                
	%speichert den rest des BKR-headers
	count = fwrite(BKR.FILE.FID,zeros(1024-80,1),'char');
	BKR.HeadLen   = ftell(BKR.FILE.FID);
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


