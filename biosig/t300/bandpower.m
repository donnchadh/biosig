function bp = bandpower(s,arg2,arg3,arg4)
% BANDPOWER calculation 
%       bpX = bandpower(X,Fs, bands, smoothing)
%  
% INPUT:
%    X          raw data, one channel per column
%    Fs         sampling rate
%    bands      each row has two elements indicating the lower and upper frequency 
%               default value is [10,12;16,24] indicating two bands of
%               10-12 and 16-24 Hz. 
%    smoothing  length of smoothing window in seconds. The default value is 1 [s] 
% 
% OUTPUT:
%    bpX        is bandpower of X 
%


%	$Id: bandpower.m,v 1.2 2005-03-25 11:34:55 schloegl Exp $
%	Copyright (c) 2005 by Alois Schloegl <a.schloegl@ieee.org>	
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

bp = [];
tmp = s; tmp(isnan(tmp))=0;         
if 1, % FIR  
        for k=1:size(F,1),
                B  = fir1(HDR.SampleRate,F(k,:)/HDR.SampleRate*2);
                bp = [bp,log(filter(ones(W*HDR.SampleRate,1),W*HDR.SampleRate,[filter(B,1,tmp),filter(B,1,tmp)].^2 ))];
        end;
else
        for k=1:size(F,1),
                [B1,A1] = butter(5,F(k,:)/HDR.SampleRate*2);
                bp = [bp,log(filter(ones(W*HDR.SampleRate,1),W*HDR.SampleRate,[filter(B1,A1,tmp),filter(B2,A2,tmp)].^2 ))];
        end;
end;

