function [signal,H] = loadeeg(FILENAME,CHAN,TYPE)
% LOADEEG loads EEG data of various data formats
% 
% Currently are the following data formats supported: 
%    EDF, CNT, EEG, BDF, GDF, BKR, MAT(*), 
%    PhysioNet (MIT-ECG), Poly5/TMS32,  
%
% [signal,header] = loadeeg(FILENAME [,CHANNEL[,TYPE]])
%
% FILENAME      name of file
% channel       list of selected channels
%               default=0: loads all channels
% TYPE (optional) forces a certain dataformat
%
% see also: EEGOPEN, EEGREAD, EEGCLOSE
%

%	$Revision: 1.10 $
%	$Id: loadeeg.m,v 1.10 2003-04-26 10:22:35 schloegl Exp $
%	Copyright (C) 1997-2003 by Alois Schloegl 
%	a.schloegl@ieee.org	

% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% Version 2 of the License, or (at your option) any later version.
%
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
%
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the
% Free Software Foundation, Inc., 59 Temple Place - Suite 330,
% Boston, MA  02111-1307, USA.


if nargin<2; CHAN=0; end;

if CHAN<1 | ~isfinite(CHAN)
        CHAN=0;
end;

if isstruct(FILENAME),
        HDR=FILENAME;
        if isfield(HDR,'FileName'),
                FILENAME=HDR.FileName;
        else
                fprintf(2,'Error LOADEEG: missing FileName.\n');	
                return; 
        end;
end;

[p,f,FileExt] = fileparts(FILENAME);
FileExt = FileExt(2:length(FileExt));
if nargin<3, 
        TYPE = FileExt; 
        %%% EDF format
        if     upper(FileExt)=='REC' TYPE='EDF';
        elseif upper(FileExt)=='EDF' TYPE='EDF';
                
        %%% Neuroscan Format        
        elseif upper(FileExt)=='AVG' TYPE='CNT';
        elseif upper(FileExt)=='COH' TYPE='CNT';
        elseif upper(FileExt)=='CSA' TYPE='CNT';
        elseif upper(FileExt)=='EEG' TYPE='CNT';
        elseif upper(FileExt)=='CNT' TYPE='CNT';
        elseif upper(FileExt)=='SET' TYPE='CNT';
        elseif upper(FileExt)=='AST' TYPE='CNT';
                
        %%% BKR Format        
	elseif upper(FileExt)=='BKR' TYPE='BKR';
	elseif upper(FileExt)=='SPB' TYPE='BKR';
	elseif upper(FileExt)=='SAB' TYPE='BKR';
	elseif upper(FileExt)=='SRB' TYPE='BKR';
	elseif upper(FileExt)=='MNB' TYPE='BKR';
	elseif upper(FileExt)=='STB' TYPE='BKR';
                
        % PhysioNet MIT-ECG database        
        elseif upper(FileExt)=='HEA' TYPE='MIT';
        elseif upper(FileExt)=='ATR' TYPE='MIT';
                
        % other formates        
        elseif upper(FileExt)=='DAT' 
		fn = dir(fullfile(p,[f,'.hea']));
		if isempty(fn),
			TYPE='DAT';
		else
			TYPE='MIT';
                end;

        elseif upper(FileExt)=='SIG' TYPE='SIG';
                
        elseif upper(FileExt(1:2))=='DA' TYPE='DA_';
                
        elseif upper(FileExt([1 3]))=='RF' TYPE='RG64';
                
        end; 
end;
TYPE = upper(TYPE);

%if strcmp(TYPE,'BKR') | strcmp(TYPE,'CNT') | strcmp(TYPE,'EDF') | strcmp(TYPE,'BDF') | strcmp(TYPE,'GDF') | strcmp(TYPE,'EEG'), 
H.FileName = FILENAME;
H.TYPE = TYPE;
H = eegopen(H,'r',CHAN);
if H.FILE.FID>0,
	[signal,H] = eegread(H);
	H = eegclose(H);
	
elseif strcmp(TYPE,'MIT')
	[pfad,file,ext]=fileparts(FILENAME);

	fid = fopen(fullfile(pfad,[file,'.hea']),'r');
	z = fgetl(fid);
	if ~strcmp(file,strtok(z,' /')),
		fprintf(2,'Warning: RecordName %s does not fit filename %s\n',strtok(z,' /'),file);
	end;	

	% sscanf(z, '%*s %d %d %d',[1,3]);
	[tmp,z] = strtok(z); 
	[tmp,z] = strtok(z); 
	H.NS = str2num(tmp);   % number of signals
	[tmp,z] = strtok(z); 
	H.SampleRate = str2num(tmp);   % sample rate of data
	[tmp,z] = strtok(z,' ()'); 
	H.SPR   = str2num(tmp);   % sample rate of data
	H.NRec  = 1;

	for k=1:H.NS,
	    	z = fgetl(fid);
	        A = sscanf(z, '%*s %d %d %d %d %d',[1,5]);
		dformat(k,1) = A(1);         % format; 
		gain(k,1) = A(2);              % number of integers per mV
		bitres(k,1) = A(3);            % bitresolution
		zerovalue(k,1)  = A(4);         % integer value of ECG zero point
		H.firstvalue(1,k) = A(5);        % first integer value of signal (to test for errors)
	end;
	fclose(fid);

	if all(dformat==dformat(1)),
		H.VERSION = dformat(1);
	else
		fprintf(2,'different DFORMATs not supported.\n');
		H.FILE.FID = -1;
		return;
	end;
	
	%------ LOAD BINARY DATA --------------------------------------------------
	fid = fopen(fullfile(pfad,[file,'.dat']),'r','ieee-le');
	if H.VERSION == 212, 
		A = fread(fid, [ceil(H.NS*3/2), inf], 'uint8')';  % matrix with 3 rows, each 8 bits long, = 2*12bit
		for k = 1:ceil(H.NS/2),
			signal(:,2*k-1) = bitand(A(:,3*k+[-2:-1])*(2.^[0;8]),2^13-1);
			signal(:,2*k)   = bitshift(bitand(A(:,3*k-1),15*16),4)+A(:,3*k);
			signal = signal(:,1:H.NS);
			signal = signal - 2^12*(signal>=2^11);	% 2-th complement
		end;

	elseif H.VERSION == 310, 
		A = fread(fid, [ceil(H.NS*2/3), inf], 'uint16')';  % matrix with 3 rows, each 8 bits long, = 2*12bit
		for k = 1:ceil(H.NS/3),
			k1=3*k-2; k2=3*k-1; k3=3*k;
			signal(:,3*k-2) = bitand(A(:,k*2-1),2^12-2)/2;	
			signal(:,3*k-1) = bitand(A(:,k*2),2^12-2)/2;	
			signal(:,3*k  ) = bitshift(A(:,k*2-1),-11) + bitshift(bitshift(A(:,k*2),-11) ,5); 
			signal = signal(:,1:H.NS);
			signal = signal - 2^10*(signal>=2^9);	% 2-th complement
		end;

	elseif H.VERSION == 311, 
		A = fread(fid, [ceil(H.NS/3), inf], 'uint32')';  % matrix with 3 rows, each 8 bits long, = 2*12bit
		for k = 1:ceil(H.NS/3),
			signal(:,3*k-2) = bitand(A(:,k),2^11-1);	
			signal(:,3*k-1) = bitand(bitshift(A(:,k),-11),2^11-1);	
			signal(:,3*k)   = bitand(bitshift(A(:,k),-22),2^11-1);	
			signal = signal(:,1:H.NS);
			signal = signal - 2^10*(signal>=2^9);	% 2-th complement
		end;

	elseif H.VERSION == 8, 
		signal = fread(fid, [H.NS,inf], 'int8')';  
		signal = cumsum(signal');

	elseif H.VERSION == 80, 
		signal = fread(fid, [H.NS,inf], 'uint8')';  
		signal = signal'-128;

	elseif H.VERSION == 160, 
		signal = fread(fid, [H.NS,inf], 'uint16')';  
		signal = signal'-2^15;

	elseif H.VERSION == 16, 
		signal = fread(fid, [H.NS,inf], 'int16')'; 
		signal = signal';

	elseif H.VERSION == 61, 
		fclose(fid);
		fid = fopen([pfad,file,'.dat'],'r','ieee-be');
		signal = fread(fid, [H.NS,inf], 'int16')'; 
		signal = signal';

	else
		fprintf(2, 'ERROR MIT-ECG: format %i not supported.\n',H.VERSION); 
	
	end;
	fclose(fid);

	if any(signal(1,:) ~= H.firstvalue), 
		fprintf(2,'ERROR MIT-ECG: inconsistency in the first bit values'); 
	end;
	for k = 1:H.NS,
		signal(:,k) = (signal(:,k) - zerovalue(k))/gain(k);
	end;
	if all(CHAN>0),
		signal = signal(:,CHAN);
	end;

	%------ LOAD ATTRIBUTES DATA ----------------------------------------------
	fid = fopen(fullfile(pfad,[file,'.atr']),'r','ieee-le');
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
	ATRTIME  = (cumsum(ATRTIME))/H.SampleRate;
	ind = find(ATRTIME <= size(signal,1)/H.SampleRate);
	H.ATRTIMED = ATRTIME(ind);
	ANNOT    = round(ANNOT);
	H.ANNOTD = ANNOT(ind);
	
	
elseif strcmp(TYPE,'MAT')
        tmp = load(FILENAME);
        if isfield(tmp,'y')
                H.NS = size(tmp.y,2);
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                warning(['Sensitivity not known in ',FILENAME]);
                if any(CHAN),
                        signal = tmp.y(:,CHAN);
                else
        	        signal = tmp.y;
                end;
                
        elseif isfield(tmp,'eeg');
                warning(['Sensitivity not known in ',FILENAME]);
                H.NS=size(tmp.eeg,2);
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                if any(CHAN),
                        signal = tmp.eeg(:,CHAN);
                else
        	        signal = tmp.eeg;
                end;
                
        elseif isfield(tmp,'P_C_S');	% G.Tec Ver 1.02, 1.50 data format
                if isstruct(tmp.P_C_S),	% without BS.analyze	
                        if (tmp.P_C_S.version==1.02) | (tmp.P_C_S.version==1.5),
                                H.Filter.LowPass  = tmp.P_C_S.lowpass;
                                H.Filter.HighPass = tmp.P_C_S.highpass;
                                H.Filter.Notch    = tmp.P_C_S.notch;
                                H.SampleRate   = tmp.P_C_S.samplingfrequency;
                                H.AS.Attribute = tmp.P_C_S.attribute;
                                
                                data = double(tmp.P_C_S.data);
                                
                        else
                                fprintf(2,'Warning: PCS-Version is %4.2f.\n',tmp.P_C_S.version);
                        end;        
                        
                elseif 1,	% with BS.analyze software, ML6.5
                        if (tmp.P_C_S.Version==1.02) | (tmp.P_C_S.Version==1.5),
                                H.Filter.LowPass  = tmp.P_C_S.LowPass;
                                H.Filter.HighPass = tmp.P_C_S.HighPass;
                                H.Filter.Notch    = tmp.P_C_S.Notch;
                                H.SampleRate   = tmp.P_C_S.SamplingFrequency;
                                H.AS.Attribute = tmp.P_C_S.Attribute;
                                
                                data = double(tmp.P_C_S.Data);
                                
                        else
                                fprintf(2,'Warning: PCS-Version is %4.2f.\n',tmp.P_C_S.Version);
                        end;
                end;
                
                sz   = size(data);
                H.NRec = sz(1);
                H.Dur  = sz(2)/H.SampleRate;
                H.NS   = sz(3);
                H.FLAG.TRIGGERED = H.NRec>1;
                
                if any(CHAN),
                        %signal=signal(:,CHAN);
                        sz(3)= length(CHAN);
                else
                        CHAN = 1:H.NS;
                end;
                
                tmp.P_C_S = []; % clear memory
                signal = reshape(permute(data(:,:,CHAN),[2,1,3]),[sz(1)*sz(2),sz(3)]);
                
        elseif isfield(tmp,'P_C_DAQ_S');
                signal = double(tmp.P_C_DAQ_S.data{1});
                H.NS = size(signal,2);
                %H.PhysDim=tmp.P_C_DAQ_S.unit;     %propriatory information
                %scale=tmp.P_C_DAQ_S.sens;         %propriatory information
                H.SampleRate = tmp.P_C_DAQ_S.samplingfrequency;
                sz     = size(signal);
                H.NRec = sz(1);
                H.Dur  = sz(2)/H.SampleRate;
                H.NS   = sz(3);
                H.FLAG.TRIGGERED = H.NRec>1;
                
        elseif isfield(tmp,'data');
                H.NS = size(tmp.data,2);
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                if any(CHAN),
                        signal = tmp.data(:,CHAN);
                else
        	        signal = tmp.data;
                end;
                
        elseif isfield(tmp,'EEGdata');  % Telemonitoring Daten (Reinhold Scherer)
                H.NS = size(tmp.EEGdata,2);
                H.Classlabel = tmp.classlabel;
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                H.PhysDim = 'µV';
                warning(['Sensitivity not known in ',FILENAME,'. 50µV is chosen']);
                if any(CHAN),
                        signal = tmp.EEGdata(:,CHAN)*50;
                else
                        signal = tmp.EEGdata*50;
                end;
                
        elseif isfield(tmp,'daten');	% EP Daten von Michael Woertz
                H.NS=size(tmp.daten.raw,2)-1;
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 2000Hz is chosen']);
                        H.SampleRate=2000;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                H.PhysDim='µV';
                warning(['Sensitivity not known in ',FILENAME,'. 100µV is chosen']);
                %signal=tmp.daten.raw(:,1:H.NS)*100;
                if any(CHAN),
                        signal = tmp.daten.raw(:,CHAN)*100;
                else
                        signal = tmp.daten.raw*100;
                end;
                
        elseif isfield(tmp,'neun') & isfield(tmp,'zehn') & isfield(tmp,'trig');
                H.NS=3;
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                warning(['Sensitivity not known in ',FILENAME]);
                signal=[tmp.neun;tmp.zehn;tmp.trig];
                if any(CHAN),
                        signal=signal(:,CHAN);
                end;        
        end;        
        
elseif strcmp(TYPE,'DAT')
        loaddat;     
        signal = Voltage(:,CHAN);
elseif strcmp(TYPE,'EBS') 
        loadebs;
elseif strcmp(TYPE,'RAW')
        loadraw;
elseif strcmp(TYPE,'RDT')
        [signal] = loadrdt(FILENAME,CHAN);
        Fs = 128;
elseif strcmp(TYPE,'XLS')
        loadxls;
elseif strcmp(TYPE,'DA_')
        fprintf('Warning LOADEEG: Format DA# in testing state and is not supported\n');
        loadda_;
elseif strcmp(TYPE,'RG64')
        [signal,H.SampleRate,H.Label,H.PhysDim,H.NS]=loadrg64(FILENAME,CHAN);
        %loadrg64;
elseif strcmp(TYPE,'SIG')
        if exist('loadsig')~=2,
                error('LOADSIG not installed.');
        end;
        [H,signal] = loadsig(FILENAME);
else
        fprintf('Error LOADEEG: Unknown Data Format\n');
end;
