function [signal,H] = loadeeg(FILENAME,CHAN,TYPE)
% LOADEEG loads EEG data of various data formats
%
% --!! LOADEEG is obsolete. Use EEGOPEN, EEGREAD, EEGCLOSE instead !!--
%       HDR = eegopen(FILENAME,'r',CHANNEL);
%	[signal,HDR] = eegread(HDR);
%       HDR = eegclose(HDR);
%
% [signal,header] = loadeeg(FILENAME [,CHANNEL[,TYPE]])
%
% FILENAME      name of file
% channel       list of selected channels
%               default=0: loads all channels
% TYPE (optional) forces a certain dataformat
%
% see also: EEGOPEN, EEGREAD, EEGCLOSE

%	$Revision: 1.5 $
%	$Id: loadeeg.m,v 1.5 2003-03-13 17:48:45 schloegl Exp $
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


% fprintf(2,'Warning: LOADEEG is obsolete. Use EEGOPEN, EEGREAD, EEGCLOSE instead. see HELP LOADEEG\n');

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

[p,f,FileExt]=fileparts(FILENAME);
FileExt=FileExt(2:length(FileExt));
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
                
        % other formates        
        elseif upper(FileExt)=='DAT' TYPE='DAT';
                
        elseif upper(FileExt)=='SIG' TYPE='SIG';
                
        elseif upper(FileExt(1:2))=='DA' TYPE='DA_';
                
        elseif upper(FileExt([1 3]))=='RF' TYPE='RG64';
                
        end; 
end;
TYPE = upper(TYPE);

if strcmp(TYPE,'BKR') | strcmp(TYPE,'CNT') | strcmp(TYPE,'EDF')| strcmp(TYPE,'BDF')|strcmp(TYPE,'GDF')| strcmp(TYPE,'EEG'), 
        H = eegopen(FILENAME,'r',CHAN);
	if H.FILE.FID<0, return; end;
        [signal,H] = eegread(H);
	H = eegclose(H);
	
elseif strcmp(TYPE,'DAT')
        loaddat;     
        signal = Voltage(:,CHAN);
elseif strcmp(TYPE,'EBS') 
        loadebs;

elseif strcmp(TYPE,'MAT')
        tmp = load(FILENAME);
        if isfield(tmp,'y')
                warning(['Sensitivity not known in ',FILENAME]);
                signal = tmp.y;
                H.NS = size(tmp.y,2);
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
        elseif isfield(tmp,'eeg');
                warning(['Sensitivity not known in ',FILENAME]);
                signal=tmp.eeg;
                H.NS=size(tmp.eeg,2);
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                
        elseif isfield(tmp,'P_C_S');	% G.Tec Ver 1.02 Data format
                if isstruct(tmp.P_C_S),	% without gb-software	
                        if (tmp.P_C_S.version==1.02) | (tmp.P_C_S.version==1.5),
                                
                                H.Filter.LowPass  = tmp.P_C_S.lowpass;
                                H.Filter.HighPass = tmp.P_C_S.highpass;
                                H.Filter.Notch    = tmp.P_C_S.notch;
                                H.SampleRate = tmp.P_C_S.samplingfrequency;
                                H.AS.Attribute = tmp.P_C_S.attribute;
                                
                                sz   = size(tmp.P_C_S.data);
                                data = double(tmp.P_C_S.data);
                                
                        else
                                fprintf(2,'Warning: PCS-Version is %4.2f.\n',tmp.P_C_S.version);
                        end;        
                        
                elseif 1,	% with GB-analyze software, ML6.5
                        if (tmp.P_C_S.Version==1.02) | (tmp.P_C_S.Version==1.5),
                                
                                H.Filter.LowPass = tmp.P_C_S.LowPass;
                                H.Filter.HighPass = tmp.P_C_S.HighPass;
                                H.Filter.Notch = tmp.P_C_S.Notch;
                                H.SampleRate = tmp.P_C_S.SamplingFrequency;
                                H.AS.Attribute = tmp.P_C_S.Attribute;
                                
                                sz   = size(tmp.P_C_S.Data);
                                data = double(tmp.P_C_S.Data);
                                
                        else
                                fprintf(2,'Warning: PCS-Version is %4.2f.\n',tmp.P_C_S.Version);
                        end;
                end;
                
                H.NRec = sz(1);
                H.Dur  = sz(2)/H.SampleRate;
                H.NS   = sz(3);
                H.FLAG.TRIGGERED  = H.NRec>1;
                
                signal  = repmat(NaN,sz(1)*sz(2),sz(3)); 
                for k1 = 1:sz(1),
                        signal((k1-1)*sz(2)+(1:sz(2)),:) = squeeze(data(k1,:,:));
                end;
                cali = 1;
                
        elseif isfield(tmp,'P_C_DAQ_S');
                signal=double(tmp.P_C_DAQ_S.data{1});
                H.NS=size(signal,2);
                %H.PhysDim=tmp.P_C_DAQ_S.unit;     %propriatory information
                %scale=tmp.P_C_DAQ_S.sens;         %propriatory information
                H.SampleRate=tmp.P_C_DAQ_S.samplingfrequency;
                
        elseif isfield(tmp,'data');
                signal=tmp.data;
                H.NS=size(tmp.data,2);
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                
        elseif isfield(tmp,'EEGdata');  % Telemonitoring Daten (Reinhold Scherer)
                warning(['Sensitivity not known in ',FILENAME,'. 50µV is chosen']);
                signal=tmp.EEGdata*50;
                H.PhysDim='µV';
                H.NS=size(tmp.EEGdata,2);
		cl = tmp.classlabel;
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                
        elseif isfield(tmp,'daten');	% EP Daten von Michael Woertz
                warning(['Sensitivity not known in ',FILENAME,'. 100µV is chosen']);
                H.NS=size(tmp.daten.raw,2)-1;
                signal=tmp.daten.raw(:,1:H.NS)*100;
                H.PhysDim='µV';
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 2000Hz is chosen']);
                        H.SampleRate=2000;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
                
        elseif isfield(tmp,'neun') & isfield(tmp,'zehn') & isfield(tmp,'trig');
                warning(['Sensitivity not known in ',FILENAME]);
                signal=[tmp.neun;tmp.zehn;tmp.trig];
                H.NS=3;
                if ~isfield(tmp,'SampleRate')
                        warning(['Samplerate not known in ',FILENAME,'. 128Hz is chosen']);
                        H.SampleRate=128;
                else
                        H.SampleRate=tmp.SampleRate;
                end;
        end;        
        
	if any(CHAN),
                signal=signal(:,CHAN);
        end;        
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
