function [HDR] = save2bkr(arg1,arg2,arg3);
% SAVE2BKR loads EEG data and saves it in BKR format
% The following data formats are supported:
%	CNT, EDF, BKR, MAT, etc. format
%
%       HDR = save2bkr(sourcefile [, destfile [, option]]);  
%
%       HDR = eegchkhdr();
%	HDR = save2bkr(HDR,data);
%
%   sourcefile	sourcefile wildcards are allowed
%   destfile	destination file in BKR format 
%	if destfile is empty or a directory, sourcefile but with extension .bkr is used.
%   options
%       gain            Gain factor for unscaled EEG data (e.g. old Matlab files) 
%       'removeDC'      removes mean
%       'autoscale k:l'	uses only channels from k to l for scaling
%       'detrend k:l'	channels from k to l are detrended with an FIR-highpass filter.
%       'PhysMax=XXX'	uses a fixed scaling factor; might be important for concanating BKR files 
%			+XXX and -XXX correspond to the maximum and minimum physical value, resp. 
% 		You can concanate several options by separating with space, komma or semicolon 
%
%   HDR		Header, HDR.FileName must contain target filename
%   data	data samples
%
% Examples: 
%   save2bkr('/tmp/*.cnt',[],'autoscale 5:30');
%	converts all CNT-files from subdir /tmp/ into BKR files 
%       and saves them in the current directory 
%   save2bkr('/tmp/*.cnt','/tmp2/','autoscale 5:30, PhysMax=200');
%	converts all CNT-files from subdir /tmp/ into BKR files 
%       and saves them in the directory /tmp2/
%	
%
%
% see also: EEGCHKHDR

%	$Revision: 1.8 $
% 	$Id: save2bkr.m,v 1.8 2003-07-21 16:19:27 schloegl Exp $
%	Copyright (C) 2002-2003 by Alois Schloegl <a.schloegl@ieee.org>		

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


FLAG_REMOVE_DC = 0;
FLAG_AUTOSCALE = 0;
FLAG_DETREND = 0;
FLAG_PHYSMAX = 0;

chansel = 0; 

if nargin<2, arg2=[]; end;

if nargin<3,
        cali = NaN;
elseif isnumeric(arg3)
        cali = arg3;        
else
        FLAG_REMOVE_DC = findstr(lower(arg3),'removedc');        
        tmp = findstr(arg3,'autoscale');
        if ~isempty(tmp);
                [chansel,tmp] = strtok(arg3(tmp+9:length(arg3)),' ;,+');
                tmp = str2num(chansel);
                if isempty(tmp),
                        fprintf(2,'invalid autoscale argument %s',chansel);
                        return;
                else
		        FLAG_AUTOSCALE = 1;
	                chansel = tmp;
                end;
        end;
        tmp = findstr(lower(arg3),'detrend');
        if ~isempty(tmp);
                [chansel_dt,tmp] = strtok(arg3(tmp+7:length(arg3)),' ;,+');
                tmp = str2num(chansel_dt);
                if isempty(tmp),
                        fprintf(2,'invalid detrend argument %s',chansel);
                        return;
                else
                        FLAG_DETREND = 1;
	                chansel_dt = tmp;
                end;
        end;
        tmp = findstr(lower(arg3),'physmax=');
        if ~isempty(tmp);
                [tmp,tmp1] = strtok(arg3(tmp+8:length(arg3)),' ;,');
                PHYSMAX = str2num(tmp);
                if isempty(PHYSMAX ),
                        fprintf(2,'invalid PhysMax argument %s',chansel);
                        return;
                else
                        FLAG_PHYSMAX = 1;
                end;
        end;
end;

if isstr(arg1), 
        inpath = fileparts(arg1);
        infile = dir(arg1);	% input  file 
        outfile = arg2;
elseif isstruct(arg1) & isnumeric(arg2),
	HDR  = arg1;
	data = arg2;
else  %if isstruct(arg1) & isnumeric(arg2),
        fprintf(2,'Error SAVE2BKR: invalid input arguments\n');	        
        return;
end;		

if isstruct(arg1),
        %HDR.FileName 	= destfile;	% Assign Filename
	if isfield(HDR,'NS')
		if HDR.NS==size(data,2),
			% It's ok. 
		elseif HDR.NS==size(data,1),
			warning('data is transposed\n');
			data = data';
		else
			fprintf(2,'HDR.NS=%i is not equal number of data columns %i\n',HDR.NS,size(data,2));
			return;
		end;				
	else
	        HDR.NS = size(data,2);	% number of channels
	end;
        if ~isfield(HDR,'NRec'),
                HDR.NRec = 1;		% number of trials (1 for continous data)
        end;	
        HDR.SPR 	= size(data,1)/HDR.NRec;	% number of samples per trial
        %HDR.SampleRate	= 100;		% Sampling rate
        %HDR.Label  	= hdr.Label; % Labels, 
        
        %HDR.PhysMax 	= max(abs(data(:)));	% Physical maximum 
        %HDR.DigMax 	= max(2^15-1);		% Digital maximum
        % --- !!! Previous scaling gave an error up to 6% and more !!! ---
        
        %HDR.Filter.LowPass  = 30;	% upper cutoff frequency
        %HDR.Filter.HighPass = .5;	% lower cutoff frequency
        HDR.FLAG.REFERENCE   = ' ';	% reference '', 'LOC', 'COM', 'LAP', 'WGT'
        %HDR.FLAG.REFERENCE  = HDR.Recording;
        HDR.FLAG.TRIGGERED   = HDR.NRec>1;	% Trigger Flag
        
        if FLAG_REMOVE_DC,
                %y = center(y,1);
                data = data - repmat(mean(data,1),size(data,1),1);
        end;
        if chansel==0,
                HDR.PhysMax = max(abs(data(:))); %gives max of the whole matrix
                data = data*(HDR.DigMax/HDR.PhysMax);	%transpose, da zuerst 1.Sample-1.Channel, dann 
        else
                tmp = data(:,chansel);
                HDR.PhysMax = max(abs(tmp(:))); %gives max of the whole matrix
                for k = 1:HDR.NS,
                        if any(k==chansel),
                                data(:,k) = data(:,k)*HDR.DigMax/HDR.PhysMax;
                        else
	                        mm = max(abs(data(:,k)));
                                data(:,k) = data(:,k)*HDR.DigMax/mm;
                        end;
                end;
        end;
        
        HDR = eegopen (HDR,'wb',0,'UCAL');     	% OPEN BKR FILE
        HDR = eegwrite(HDR,data);  	% WRITE BKR FILE
        HDR = eegclose(HDR);            % CLOSE BKR FILE

	% save Classlabels
	if isfield(HDR,'Classlabel'),
		fid = fopen([HDR.FileName(1:length(HDR.FileName)-4) '.par'],'wt');
        	fprintf(fid, '%i\n', HDR.Classlabel);
        	fclose(fid);
	end;
        return;
end;


for k=1:length(infile);
        filename = fullfile(inpath,infile(k).name);
        [pf,fn,ext] = fileparts(filename);
        
        [y,HDR] = loadeeg(filename);
        if isempty(y), 
                fprintf(2,'Error SAVE2BKR: file %s not found\n',filename);
                return; 
        end; 
        
        if ~isfield(HDR,'NS'),
                warning(['number of channels undefined in ',filename]);
                HDR.NS = size(y,2);
        end;
        if ~isfield(HDR,'NRec'),
                HDR.NRec = 1;
        end;
        if ~isfield(HDR,'SPR'),
                HDR.SPR = size(y,1)/HDR.NRec;
        end;
        
        if FLAG_REMOVE_DC,
                y = y-repmat(mean(y,1),size(y,1),1);
        end;
        if FLAG_DETREND,
                B = -ones(1,HDR.SampleRate)/HDR.SampleRate;
                B(HDR.SampleRate/2) = B(HDR.SampleRate/2)+1;
                HDR.Filter.B = B;
                HDR.Filter.A = 1;
                %HDR.Filter.B=B;%conv(-B, HDR.Filter.B);
                Delay = length(B)/2;        
                HDR.FLAG.FILT = 1;
                HDR.Filter.HighPass = .5;
                
                for k = chansel_dt,
                        tmp = filter(B,1,[y(:,k);zeros(length(B),1)]);
                        y(:,k) = tmp(Delay+1:size(y,1)+Delay);
	        end;                
        end;
        
        HDR.DigMax=2^15-1;
        if FLAG_PHYSMAX,
                HDR.PhysMax = PHYSMAX;
        else
                if chansel==0,
                        HDR.PhysMax = max(abs(y(:))); %gives max of the whole matrix
                else
                        tmp = y(:,chansel);
                        HDR.PhysMax = max(abs(tmp(:))); %gives max of the whole matrix
                end;
        end;
        
        if chansel==0,
                y = y*(HDR.DigMax/HDR.PhysMax);	%transpose, da zuerst 1.Sample-1.Channel, dann 
        else
                for k = 1:HDR.NS,
                        mm = max(abs(y(:,k)));
                        if any(k==chansel),
                                y(:,k) = y(:,k)*HDR.DigMax/HDR.PhysMax;
                        else
                                y(:,k) = y(:,k)*HDR.DigMax/mm;
                        end;
                end;
        end;
        
        tmp = round(HDR.PhysMax);
        fprintf(1,'Rounding of PhysMax yields %f%% error.\n',abs((HDR.PhysMax-tmp)/tmp)*100);
        HDR.PhysMax = tmp;
        HDR.TYPE = 'BKR';
        HDR.FLAG.REFERENCE = ' ';
        
        HDR.VERSION = 207;
        HDR.FILE.Ext= 'bkr';
        
        if isempty(outfile), 	% default destination directory  
                ix = max(find(filename=='.'));
                %HDR.FileName = [filename(1:ix-1),'.bkr'];  % destination directory is same as source directory 
                HDR.FileName  = [HDR.FILE.Name,'.bkr'];     % destination directory is current working directory 
        elseif isdir(outfile),	% output file
                HDR.FILE.Path = outfile;            
	        HDR.FileName  = fullfile(outfile,[HDR.FILE.Name,'.bkr']);
        else
                %[HDR.FILE.Path,HDR.FILE.Path,HDR.FILE.Path]=fileparts(outfile);
                HDR.FileName = outfile;
        end;
        HDR   = eegchkhdr(HDR);
        
        fid   = fopen(HDR.FileName,'w+','ieee-le');
        if fid<0,
                fprintf('Error SAVE2BKR: couldnot open file %s.\n',HDR.FileName);
                return;
        end;
        count = fwrite(fid,207,'short');	        % version number of header
        count = fwrite(fid,HDR.NS  ,'short');	        % number of channels
        count = fwrite(fid,HDR.SampleRate,'short');     % sampling rate
        count = fwrite(fid,HDR.NRec,'uint32');          % number of trials: 1 for untriggered data
        count = fwrite(fid,HDR.SPR ,'uint32');          % samples/trial/channel
        count = fwrite(fid,HDR.PhysMax,'short');        % Kalibrierspannung
        count = fwrite(fid,HDR.DigMax ,'short');        % Kalibrierwert
        count = fwrite(fid,zeros(4,1),'char');        
        
        count = fwrite(fid,[HDR.Filter.LowPass(1),HDR.Filter.HighPass(1)],'float'); 
        count = fwrite(fid,zeros(16,1),'char');         % offset 30
        
        count = fwrite(fid,HDR.FLAG.TRIGGERED,'int16');	% offset 32
        count = fwrite(fid,zeros(24,1),'char');         % offset 46
        tmp   = [strcmp(HDR.FLAG.REFERENCE,'COM')|strcmp(HDR.FLAG.REFERENCE,'CAR'), strcmp(HDR.FLAG.REFERENCE,'LOC')|strcmp(HDR.FLAG.REFERENCE,'LAR'), strcmp(HDR.FLAG.REFERENCE,'LAP'), strcmp(HDR.FLAG.REFERENCE,'WGT')];
        count = fwrite(fid,tmp,'int16'); 			% offset 72 + 4*BOOL
        
        %speichert den rest des BKR-headers
        count = fwrite(fid,zeros(1024-80,1),'char');
        % writes data
        count = fwrite(fid,y','short');
        fclose(fid);
        
        % save classlabels
        if isfield(HDR,'Classlabel'),
                fid = fopen([HDR.FileName(1:length(HDR.FileName)-4) '.par'],'w');
                fprintf(fid, '%i\r\n', HDR.Classlabel);
                fclose(fid);
        end;
end;
