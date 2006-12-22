function [H,HDR]=eeg2hist(FILENAME,CHAN);
% EEG2HIST Histogram test 
% Calculates the histogram and derived measures of biosignal recordings. 
% This is useful for quality control of biosignal recordings [1-3].  
%
% [HISTOG,HDR]=EEG2HIST(FILENAME,CHAN)
%
% input: FILENAME   EEG-File
%        CHAN       Channel select
% output: 
%        HISTOG    each column is the histogram of one EDF channel
%	 HDR	   header information
% 
% see also: tsa/hist2res, plota
%
%
% REFERENCES:
% [1] A. Schlögl, B. Kemp, T. Penzel, D. Kunz, S.-L. Himanen,A. Värri, G. Dorffner, G. Pfurtscheller.
% Quality Control of polysomnographic Sleep Data by Histogram and EntropyAnalysis. 
% Clin. Neurophysiol. 1999, Dec; 110(12): 2165 - 2170.
%
% [2] A. Schlögl, G. Klösch, W. Koele, J. Zeitlhofer, P.Rappelsberger, G. Pfurtscheller
% Qualitätskontrolle von Biosignalen,
% Jahrestagung der Österreichischen Gesellschaft für KlinischeNeurophysiologie, 27. Nov. 1999, Vienna.
%
% [3] http://www.dpmi.tu-graz.ac.at/~schloegl/lectures/Q/index.htm
%
% [4] A. Schlögl, Time Series Analysis toolbox for Matlab. 1996-2003
% http://www.dpmi.tu-graz.ac.at/~schloegl/matlab/tsa/

% 	$Id: eeg2hist.m,v 1.4 2006-12-22 15:08:59 schloegl Exp $
%	Copyright (C) 2002-2003 by Alois Schloegl <a.schloegl@ieee.org>		
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

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



if nargin<2, CHAN=0; end; 


HDR = sopen(FILENAME,'r',CHAN,'UCAL');	% open EEG file in uncalibrated mode (no scaling of the data)
if HDR.FILE.FID<0,
        fprintf(2,'EEG2HIST: couldnot open file %s.\n',FILENAME); 
        return;
end;

if CHAN<1, CHAN=1:HDR.NS; end;

H.datatype='HISTOGRAM';

if strcmp(HDR.TYPE,'BKR') | strcmp(HDR.TYPE,'CNT') | strcmp(HDR.TYPE,'EEG'),
        [s,HDR]=sread(HDR);
        
        H.H = zeros(2^16,HDR.NS);
        for l = 1:HDR.NS,
		if exist('OCTAVE_VERSION') > 2,
                	for k = s(:,l)'+2^15+1, H.H(k,l) = H.H(k,l)+1;  end;
		else
                	H.H(:,l)=sparse(s(:,l)'+2^15+1,1,1,2^16,1);
		end;
        end;
        tmp = find(any(H.H,2));
        H.X = tmp-2^15-1; 	%int16
        H.H = H.H(tmp,:);

        
elseif strcmp(HDR.TYPE,'BDF'),
        [s,HDR] = sread(HDR);
        
        H.H = sparse(2^24,HDR.NS);
        for l = 1:HDR.NS,
                H.H(:,l)=sparse(s(:,l)'+2^23+1,1,1,2^24,1);
                %for k = s(:,l)'+2^23+1, H.H(k,l) = H.H(k,l)+1; end;
        end;
        tmp = find(any(H.H,2));
        H.X = tmp-2^23-1; 
        H.H = H.H(tmp,:);
        
elseif strcmp(HDR.TYPE,'EDF');
        NoBlks=ceil(60/HDR.Dur);
        bi=[0;cumsum(HDR.SPR)];     
        ns=length(CHAN);
        
        H.H = zeros(2^16,HDR.NS);
        
        l=1;
        while (l<HDR.NRec) & ~feof(HDR.FILE.FID)
                % READ BLOCKS of DATA
                [S, count] = fread(HDR.FILE.FID,[HDR.AS.spb,NoBlks],'int16');
                if count < HDR.AS.spb*NoBlks
                        fprintf(2,'    Warning EEG2HIST: read error, only %i of %i read\n', count, spb*NoBlks);
                end;
                
                %%%%% HISTOGRAM
                for l=1:ns,
	                h = zeros(2^16,1);
			if exist('OCTAVE_VERSION') > 2,
                        for k=reshape(S(bi(CHAN(l))+1:bi(CHAN(l)+1),:),1,HDR.SPR(l)*NoBlks)+2^15+1, h(k,l) = h(k,l)+1; end;     
                        else
			h = sparse(S(bi(CHAN(l))+1:bi(CHAN(l)+1),:)+2^15+1,1,1,2^16,1);	                        
                        end;
			H.H(:,ns) = H.H(:,ns) + h;
                end;
                
                l=l+NoBlks; 
        end; % WHILE     
        tmp = find(any(H.H,2));
        H.X = tmp-2^15-1; 	%int16
        H.H = H.H(tmp,:);
else
        fprintf(2,'EEG2HIST: format %s not implemented yet.\n',HDR.TYPE);
        
end;
H.N = sum(H.H);

HDR = sclose(HDR);

