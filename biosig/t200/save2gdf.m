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

% 	$Id: save2gdf.m,v 1.3 2005-04-02 22:22:11 schloegl Exp $
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
        if HDR.FILE.FID<0, 
                fprintf(2,'Error SAVE2GDF: file %s not found\n',filename);
                return; 
        end; 
        HDR.FLAG.UCAL = 1; 
        HDR.FLAG.OVERFLOWDETECTION = 0; 
        [y,HDR] = sread(HDR,inf);
        HDR = sclose(HDR);
        
        
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

	% THRESHOLD, GDFTYP -> Phys/Dig/Min/Max
	if isfield(HDR,'THRESHOLD') 
		HDR.DigMax  = HDR.THRESHOLD(1:HDR.NS,2)';
		HDR.DigMin  = HDR.THRESHOLD(1:HDR.NS,1)';

	elseif ~isfield(HDR,'THRESHOLD')
		fprintf(2,'Warning SAVE2GDF: no THRESHOLD value provided - automated overflow detection not supported');

	        HDR.DigMax = max(y,[],1);
		HDR.DigMin = min(y,[],1);
	end; 
	HDR.PhysMax = [1,HDR.DigMax]*HDR.Calib;
	HDR.PhysMin = [1,HDR.DigMin]*HDR.Calib;
	
	if isfield(HDR,'GDFTYP')
    		%bits = ceil(log2(max(HDR.DigMax-HDR.DigMin+1))/8)*8;    % allows int8, int16, int24, int32, etc. 
    		bits1 = ceil(log2(HDR.DigMax-HDR.DigMin+1));    
	        [datatyp,limits,datatypes] = gdfdatatype(HDR.GDFTYP);
		bits = log2(limits(:,2)-limits(:,1)+1);
    		fprintf(1,'SAVE2GDF: %i bits needed, %i bits used for file %s\n',max(bits1),max(bits),HDR.FileName);

	        % re-scale data to account for the scaling factor in the header
	        %HIS = histo3(y); save HIS HIS
	else	
		tmp = sort(y,1);
		tmp = diff(tmp);
		tmp(tmp<8*eps) = NaN;
		dQ = min(tmp);
	
		digmax = HDR.DigMax; 
		digmin = HDR.DigMin; 

    		bits = ceil(log2(max(digmax-digmin+1)));        % allows any bit-depth
	        if min(dQ)<1,    GDFTYP = 16;  	% float32
		elseif bits==8,  GDFTYP = 1;	% int8
	        elseif bits==16, GDFTYP = 3;	% int16
	        elseif bits==32, GDFTYP = 5;	% int32
	        elseif bits==64, GDFTYP = 7;	% int64
	        elseif ~isempty(bits);	GDFTYP = 255+bits;	% intN
		else        	 GDFTYP = 3; 	% int8
	        end;
		HDR.GDFTYP = GDFTYP; 

	        if length(HDR.GDFTYP)==HDR.NS,
    	        elseif length(HDR.GDFTYP)==1,
    	                HDR.GDFTYP = HDR.GDFTYP*ones(1,HDR.NS);  % int16
    	        else
    	                %% PROBLEM 
    	        end
		% HDR.PhysMax = [1,digmax]*HDR.Calib; %max(y,[],1); %gives max of the whole matrix
	        % HDR.PhysMin = [1,digmin]*HDR.Calib; %min(y,[],1); %gives max of the whole matrix
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
	end;
        
        HDR.FLAG.UCAL = 1;              % data is de-calibrated, no rescaling within SWRITE 
	HDR.TYPE = 'GDF';

        if isempty(outfile), 	% default destination directory  
                ix = max(find(filename=='.'));
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
	        HDR.FLAG.UCAL = 1; 
    		HDR.FLAG.OVERFLOWDETECTION = 0; 
    		[y1,HDR] = sread(HDR,inf);
                HDR = sclose(HDR);
		all(y==y1),
        catch
                fprintf(2,'Error SAVE2GDF: saving file %s failed\n',HDR.FileName);
        end;
end;
