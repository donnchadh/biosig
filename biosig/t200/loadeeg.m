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

%	$Revision: 1.12 $
%	$Id: loadeeg.m,v 1.12 2003-05-09 17:54:18 schloegl Exp $
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
                if isfield(tmp,'classlabel'),
                	H.Classlabel = tmp.classlabel;
                end;        
                        	
        elseif isfield(tmp,'P_C_S');	% G.Tec Ver 1.02, 1.50 data format
                if isstruct(tmp.P_C_S),	% without BS.analyze	
                        if (tmp.P_C_S.version==1.02) | (tmp.P_C_S.version==1.5),
                                H.Filter.LowPass  = tmp.P_C_S.lowpass;
                                H.Filter.HighPass = tmp.P_C_S.highpass;
                                H.Filter.Notch    = tmp.P_C_S.notch;
                                H.SampleRate   = tmp.P_C_S.samplingfrequency;
                                H.AS.Attribute = tmp.P_C_S.attribute;
                                H.AS.AttributeName = tmp.P_C_S.attributename;
                                H.Label = tmp.P_C_S.channelname;
                                H.AS.EpochingSelect = tmp.P_C_S.epochingselect;
                                H.AS.EpochingName = tmp.P_C_S.epochingname;
                                
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
                                H.AS.AttributeName = tmp.P_C_S.AttributeName;
                                H.Label = tmp.P_C_S.ChannelName;
                                H.AS.EpochingSelect = tmp.P_C_S.EpochingSelect;
                                H.AS.EpochingName = tmp.P_C_S.EpochingName;
                                
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
                %scale  = tmp.P_C_DAQ_S.sens;      
                H.Cal = tmp.P_C_DAQ_S.sens*(2.^(1-tmp.P_C_DAQ_S.daqboard{1}.HwInfo.Bits));
                
                if all(tmp.P_C_DAQ_S.unit==1)
                        H.PhysDim='uV';
                else
                        H.PhysDim='[?]';
                end;
                
                H.SampleRate = tmp.P_C_DAQ_S.samplingfrequency;
                sz     = size(signal);
                if length(sz)==2, sz=[1,sz]; end;
                H.NRec = sz(1);
                H.Dur  = sz(2)/H.SampleRate;
                H.NS   = sz(3);
                H.FLAG.TRIGGERED = H.NRec>1;
                H.Filter.LowPass = tmp.P_C_DAQ_S.lowpass;
                H.Filter.HighPass = tmp.P_C_DAQ_S.highpass;
                H.Filter.Notch = tmp.P_C_DAQ_S.notch;
                if any(CHAN),
                        signal=signal(:,CHAN);
                else
                        CHAN=1:H.NS;
                end; 
                if ~H.FLAG.UCAL,
			signal=signal*diag(H.Cal(CHAN));                	        
                end;
                
                
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
                H.NS = size(tmp.daten.raw,2)-1;
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 2000Hz is chosen']);
                        H.SampleRate=2000;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                H.PhysDim = 'µV';
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
                signal  = [tmp.neun;tmp.zehn;tmp.trig];
                H.Label = {'Neun','Zehn','TRIG'};
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
