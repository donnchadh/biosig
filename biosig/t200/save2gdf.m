function [HDR] = save2gdf(arg1,arg2,arg3);
% SAVE2GDF loads EEG data and saves it in GDF format
%    It has been tested with data of the following formats:
%       Physiobank, BKR, CNT (Neurscan), EDF, 
%
%       HDR = save2gdf(sourcefile [, destfile [, option]]);  
%
%	HDR = save2gdf(HDR,data);
%
%
% see also: SLOAD, SOPEN, SREAD, SCLOSE, SWRITE

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

% 	$Id: save2gdf.m,v 1.2 2005-03-25 11:20:22 schloegl Exp $
%	Copyright (C) 2003-2005 by Alois Schloegl <a.schloegl@ieee.org>		
%       This file is part of the biosig project http://biosig.sf.net/

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
FLAG_removeDrift = 0;

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
                        fprintf(2,'invalid detrend argument %s',chansel_dt);
                        return;
                else
                        FLAG_DETREND = 1;
                        chansel_dt = tmp;
                end;
        end;
        
        tmp = findstr(lower(arg3),'removedrift');
        if ~isempty(tmp);
                [chansel_dt2,tmp] = strtok(arg3(tmp+11:length(arg3)),' ;,+');
                tmp = str2num(chansel_dt2);
                if isempty(tmp),
                        fprintf(2,'invalid RemoveDrift argument %s',chansel_dt2);
                        return;
                else
                        FLAG_removeDrift = 1;
                        chansel_dt2 = tmp;
                end;
        end;
        tmp = findstr(lower(arg3),'physmax=');
        if ~isempty(tmp);
                [tmp,tmp1] = strtok(arg3(tmp+8:length(arg3)),' ;,');
                PHYSMAX = str2num(tmp);
                if isempty(PHYSMAX ),
                        fprintf(2,'invalid PhysMax argument %s',tmp);
                        return;
                else
                        FLAG_PHYSMAX = 1;
                end;
        end;
end;

if isstr(arg1), 
        inpath = fileparts(arg1);
        infile = dir(arg1);	% input  file 
        if isempty(infile)
                fprintf(2,'ERROR SAVE2GDF: file %s not found.\n',arg1);
                return;
        end;
        outfile = arg2;
elseif isstruct(arg1) & isnumeric(arg2),
        HDR  = arg1;
        data = arg2;
else  %if isstruct(arg1) & isnumeric(arg2),
        fprintf(2,'Error SAVE2GDF: invalid input arguments\n');	        
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
        
        if FLAG_REMOVE_DC,
                %y = center(y,1);
                data = data - repmat(mean(data,1),size(data,1),1);
        end;

        % re-scale data to account for the scaling factor in the header
        HDR.PhysMax = max(data,[],1);
        HDR.PhysMin = min(data,[],1);
        if ~isfield(HDR,'GDFTYP')
                HDR.GDFTYP = 16; 
        end

        if length(HDR.GDFTYP)==HDR.NS,
        elseif length(HDR.GDFTYP)==1,
                HDR.GDFTYP = HDR.GDFTYP*ones(1,HDR.NS);  % int16
        else
                %% PROBLEM 
        end
        [datatyp,limits,datatypes] = gdfdatatype(HDR.GDFTYP);
        HDR.DigMax = limits(:,2)';
        HDR.DigMin = limits(:,1)';
        
        for k = 1:HDR.NS,
                K = (HDR.DigMax(k)-HDR.DigMin(k))/(HDR.PhysMax(k)-HDR.PhysMin(k));
                data(:,k) = (data(:,k)-HDR.PhysMin(k))*K + HDR.DigMin(k);
        end;
        
        HDR.FLAG.UCAL = 1;              % data is de-calibrated, no rescaling within SWRITE 
        %HDR = eegchkhdr(HDR);   
        HDR.TYPE = 'GDF';
        
        HDR = sopen (HDR,'w');     	% OPEN GDF FILE
        HDR = swrite(HDR,data);  	% WRITE GDF FILE
        HDR = sclose(HDR);            % CLOSE FILE
        
        % final test 
        try
                HDR = sopen(HDR.FileName,'r');
                HDR = sclose(HDR);
        catch
                fprintf(2,'Error SAVE2GDF: saving file %s failed\n',HDR.FileName);
        end;
        return;
end;

for k=1:length(infile);
        filename = fullfile(inpath,infile(k).name);
        [pf,fn,ext] = fileparts(filename);
        
        % load eeg data 
        %[y,HDR] = sload(filename);
        HDR = sopen(filename,'r',0);
        HDR.FLAG.UCAL = 1; 
        [y,HDR] = sread(HDR,inf);
        HDR = sclose(HDR);
        
        if isempty(y), 
                fprintf(2,'Error SAVE2GDF: file %s not found\n',filename);
                return; 
        end; 
        
        if ~isfield(HDR,'NS'),
                warning(['number of channels undefined in ',filename]);
                HDR.NS = size(y,2);
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
        
        if FLAG_removeDrift,
                B = .5*(1 - cos(2*pi*(1:4*HDR.SampleRate+1)'/(4*HDR.SampleRate+2))); 
                B = -B/sum(B);
                B(2*HDR.SampleRate) = B(HDR.SampleRate)+1;
                
                B = -ones(1,HDR.SampleRate)/HDR.SampleRate;
                B(HDR.SampleRate/2) = B(HDR.SampleRate/2)+1;
                %B(1) = B(1)+1;
                
                HDR.Filter.B = B;
                HDR.Filter.A = 1;
                %HDR.Filter.B=B;%conv(-B, HDR.Filter.B);
                Delay = (length(B)-1)/2;        
                HDR.FLAG.FILT = 1;
                HDR.Filter.HighPass = .5;
                
                for k = chansel_dt2,
                        y(:,k) = filtfilt(B,1,y(:,k));
                        %y(:,k) = tmp(Delay+1:size(y,1)+Delay);
                end;                
        end;
        % re-scale data to account for the scaling factor in the header
        %HIS = histo3(y); save HIS HIS
        digmax = max(y,[],1);
        digmin = min(y,[],1);
        if 1; %~isfield(HDR,'GDFTYP')
                bits1 = ceil(log2(max(digmax-digmin+1)));      
                %bits = ceil(log2(max(digmax-digmin+1))/8)*8;    % allows int8, int16, int24, int32, etc. 
                bits = ceil(log2(max(digmax-digmin+1)));        % allows any bit-depth
                fprintf(1,'SAVE2GDF: %i bits needed, %i bits used for file %s\n',bits1,bits,HDR.FileName);
                if     bits==8,  GDFTYP = 1;
                elseif bits==16, GDFTYP = 3;
                elseif bits==32, GDFTYP = 5;
                elseif bits==64, GDFTYP = 7;
                else             GDFTYP = 255+bits;
                end;
                HDR.GDFTYP = GDFTYP; 
        end
        
        if ~isfield(HDR,'GDFTYP')
                HDR.GDFTYP = 3; 
        end;
        if length(HDR.GDFTYP)==HDR.NS,
        elseif length(HDR.GDFTYP)==1,
                HDR.GDFTYP = HDR.GDFTYP*ones(1,HDR.NS);  % int16
        else
                %% PROBLEM 
        end
                
        HDR.PhysMax = [1,digmax]*HDR.Calib; %max(y,[],1); %gives max of the whole matrix
        HDR.PhysMin = [1,digmin]*HDR.Calib; %min(y,[],1); %gives max of the whole matrix
        [datatyp,limits,datatypes] = gdfdatatype(HDR.GDFTYP);
        c0 = 0; 
        while any(digmin'<limits(:,1)),
                c = 2^ceil(log2(max(limits(:,1)-digmin')))
                digmin = digmin + c;
                digmax = digmax + c;
                c0 = c0 + c;
        end;
        while any(digmax'>limits(:,2)),
                c = 2^ceil(log2(max(digmax'-limits(:,2))))
                digmin = digmin - c;
                digmax = digmax - c;
                c0 = c0 - c;
        end;
        while any(digmin'<limits(:,1)),
                c = 2^ceil(log2(max(limits(:,1)-digmin')))
                digmin = digmin + c;
                digmax = digmax + c;
                c0 = c0 + c;
        end;
        y = y + c0;
        
        HDR.DigMax = digmax; %limits(:,2); %*ones(1,HDR.NS);
        HDR.DigMin = digmin; %limits(:,1); %*ones(1,HDR.NS);
        %fprintf(1,'Warning SAVE2GDF: overflow detection not implemented, yet.\n');
        
        HDR.FLAG.UCAL = 1;              % data is de-calibrated, no rescaling within SWRITE 
        HDR.TYPE = 'GDF';

        if isempty(outfile), 	% default destination directory  
                ix = max(find(filename=='.'));
                %HDR.FileName = [filename(1:ix-1),'.bkr'];  % destination directory is same as source directory 
                HDR.FileName  = [HDR.FILE.Name,'.gdf'];     % destination directory is current working directory 
        elseif isdir(outfile),	% output file
                HDR.FILE.Path = outfile;            
                HDR.FileName  = fullfile(outfile,[HDR.FILE.Name,'.gdf']);
        else
                [HDR.FILE.Path,HDR.FILE.Name,Ext] = fileparts(outfile);
                HDR.FileName = fullfile(HDR.FILE.Path,[HDR.FILE.Name,Ext]);
        end;

        HDR = sopen(HDR,'w');
        if HDR.FILE.FID < 0,
                fprintf(1,'Error SAVE2GDF: couldnot open file %s.\n',HDR.FileName);
                return;
        end;
        HDR = swrite(HDR,y(:,1:HDR.NS));  	% WRITE GDF FILE
        HDR = sclose(HDR);
        
        % final test 
        try
                HDR = sopen(HDR.FileName,'r');
                HDR = sclose(HDR);
        catch
                fprintf(2,'Error SAVE2GDF: saving file %s failed\n',HDR.FileName);
        end;
end;
