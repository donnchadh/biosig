function bp = bandpower(s,arg2,arg3,arg4,mode)
% BANDPOWER calculation 
%       bp = bandpower(s, Fs, bands, smoothing, mode)
%  
% INPUT:
%    s          raw data, one channel per column
%    Fs         sampling rate
%    bands      each row has two elements indicating the lower and upper frequency 
%               default value is [10,12;16,24] indicating two bands of
%               10-12 and 16-24 Hz. 
%    smoothing  length of smoothing window in seconds. The default value is 1 [s]
%    mode       mode == 1 uses FIR filter and log10
%               mode == 2 uses Butterworth IIR filter and log10
%               mode == 3 udes FIR filter and ln
%               mode == 4 uses Butterworth IIR filter and ln
%               the default value is mode == 2 (Butterworth filter and log10)
% 
% OUTPUT:
%    bp is log(bandpower) of s
%       the order of the features is 
%       [f1(#1), f1(#2),...,f1(#n), f2(#1),...,f2(#n),...,fm(#1),...,fm(#n)]
%       First, the first frequency band of all channels, is followed by
%       the the second band of all channels, until the last 
%       last f-band of all channels 

%	$Id: bandpower.m 2205 2009-10-27 12:18:15Z schloegl $
%	Copyright (C) 2007 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

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


if ischar(s) 
        [s,HDR]=sload(s);
else
        if size(s,1)<size(s,2),
                error('signal data must be ordered in columns')
        end;
        HDR.SampleRate = arg2;
end;

if nargin<3     
        F = [10,12;16,24];
else
        F = arg3;
end;
if nargin<4     
        W = 1; 
else
        W = arg4;
end;
if nargin<5
    mode = 4;
end;

bp = [];
tmp = s; tmp(isnan(tmp))=0;         
if mode == 1  % FIR and log10
        for k=1:size(F,1),
                B  = fir1(HDR.SampleRate,F(k,:)/HDR.SampleRate*2);
                bp = [bp,log10(filter(ones(W*HDR.SampleRate,1),W*HDR.SampleRate,filter(B,1,tmp).^2 ))];
        end;
elseif mode == 2  % Butterworth and log10
        for k=1:size(F,1),
                [B,A] = butter(5,F(k,:)/HDR.SampleRate*2);
                bp = [bp,log10(filter(ones(W*HDR.SampleRate,1),W*HDR.SampleRate,filter(B,A,tmp).^2 ))];
        end;
elseif mode == 3  % FIR and ln
        for k=1:size(F,1),
                B  = fir1(HDR.SampleRate,F(k,:)/HDR.SampleRate*2);
                bp = [bp,log(filter(ones(W*HDR.SampleRate,1),W*HDR.SampleRate,filter(B,1,tmp).^2 ))];
        end;       
elseif mode == 4  % Butterworth and ln
        for k=1:size(F,1),
                [B,A] = butter(5,F(k,:)/HDR.SampleRate*2);
                bp = [bp,log(filter(ones(W*HDR.SampleRate,1),W*HDR.SampleRate,filter(B,A,tmp).^2 ))];
        end;        
end;

