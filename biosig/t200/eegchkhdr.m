function HDR=eegchkhdr(HDR);
% EEGCHKHDR checks header information an 
% provides interactive dialog for missing Headerinformation
%
% HDR=eegchkhdr(HDR)
%
% see also: EEGOPEN, EEGREAD, EEGSEEK, EEGTELL, EEGCLOSE, EEGWRITE

%	$Revision: 1.1 $
%	$Id: eegchkhdr.m,v 1.1 2003-02-01 15:03:45 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


if nargin<1, HDR=[]; end;

if ~isfield(HDR,'FileName'),
        tmp = input('Whats the Filename? :');
        HDR.FileName = tmp;
end;
if ~isfield(HDR,'TYPE'),
	[pfad,file,FileExt] = fileparts(HDR.FileName);
        HDR.TYPE = upper(FileExt(2:length(FileExt)));
end;

Answer='N';
if strcmp(HDR.TYPE,'BKR'),
        while ~strcmp(upper(Answer),'Y'),
                if ~isfield(HDR,'FileName'),
                        tmp = input('Whats the Filename? :');
                        HDR.FileName = tmp;
                end;
                
                if ~isfield(HDR,'VERSION'),
                        HDR.VERSION=207;
                end;
                
                if ~isfield(HDR,'NS'),
                        tmp=input('Whats the number of channels? :');
                        HDR.NS=tmp;
                end;
                if ~isfield(HDR,'SampleRate'),
                        tmp=input('Whats the sampling rate[Hz] :');
                        HDR.SampleRate=tmp;
                end;
                if ~isfield(HDR,'NRec'),
                        tmp=input('How many trials do you want to save (type 1 for a continous data):');
                        HDR.NRec=tmp;
                end;
                if isempty(HDR.NRec), HDR.NRec=1; end;
                
                if ~isfield(HDR,'PhysMax'),
                        tmp=input('Whats the maximum physical value [µV]:');
                        HDR.PhysMax=tmp;
                end;
                if isempty(HDR.PhysMax), HDR.PhysMax=nan; end;
                
                if ~isfield(HDR,'DigMax'),
                        HDR.DigMax=2^15-1;
                end;
                if ~isfield(HDR,'Filter'),
                        HDR.Filter=[];
                end;
                if ~isfield(HDR.Filter,'LowPass'),
                        tmp=input('Whats the upper cutoff frequency?[Hz]:');
                        HDR.Filter.LowPass=tmp;
                end;
                if ~isfield(HDR.Filter,'HighPass'),
                        tmp=input('Whats the lower cutoff frequency?[Hz]:');
                        HDR.Filter.HighPass=tmp;
                end;
                
                fprintf(1,'\n%% This demonstrates how the Header should be defined. \n');
                fprintf(1,'HDR.FileName\t= ''%s'';  %%(FileName)\n',HDR.FileName);
                fprintf(1,'HDR.NS\t\t=%5i;  %%(number of channels)\n',HDR.NS);
                fprintf(1,'HDR.SampleRate\t=%5i;  %%(Sampling rate [Hz])\n',HDR.SampleRate);
                fprintf(1,'HDR.NRec\t=%5i;  %%(number of trials)\n',HDR.NRec);
                fprintf(1,'HDR.PhysMax\t= %.3f; %%(Physical Maximum)\n',HDR.PhysMax);
                fprintf(1,'HDR.DigMax\t= %5i;  %%(Digital Maximum)\n',HDR.DigMax);
                fprintf(1,'HDR.Filter.LowPass\t=%5f;  %%(upper cutoff frequency)\n',HDR.Filter.LowPass);
                fprintf(1,'HDR.Filter.HighPass\t=%5f;  %%(lower cutoff frequency [Hz])\n',HDR.Filter.HighPass);
                
                if ~isfield(HDR,'FLAG'), 
                        tmp_flag = 0; 
                        ref_flag = 0;
                else 
                        tmp_flag = isfield(HDR.FLAG,'TRIGGERED');
                        ref_flag = isfield(HDR.FLAG,'REFERENCE');
                end; 
                if strcmp(HDR.TYPE,'BKR'),
                        if HDR.VERSION ~= 207;
                                fprintf(HDR.FILE.stderr,'Error: HDR.VERSION is not 207\n',HDR.VERSION);
                                HDR.Version=207;
                        end;
                        if ~tmp_flag, 
                                HDR.FLAG.TRIGGERED   = HDR.NRec>1;	% Trigger Flag
                        end;
                        if ~ref_flag, 
                                HDR.FLAG.REFERENCE   = '';	% Trigger Flag
                        end;
                        %fprintf(1,'HDR.FLAG.TRIGGERED\t=%2i;  %%(FLAG TRIGGERED 0=OFF, 1=ON)\n',HDR.FLAG.TRIGGERED);
                        fprintf(1,'HDR.FLAG.REFERENCE\t=''%3s'';  %% (COM, CAR: common average reference; LOC,LAR local average ref; LAP Laplacian derivation, WGT weighted average)\n',HDR.FLAG.REFERENCE);
                end;
                
                %A = input('Are all values correct [Y]/n?');
                Answer = 'Y';
        end;
end;
