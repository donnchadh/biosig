function [X] = criteria4asyncbci(d, TRIG, rbtime,Fs)
% CRITERIA4ASYNCBCI provides an evaluation criterion of asychronous BCI. 
% based on the discussion at the BCI2005 meeting in Rensellearville.  
%
% X = CRITERIA4ASYNCBCI(D, TRIG, rb_time,Fs)
%   D           detector output (assuming Threshold = 0); 
%   TRIG        trigger time point in seconds
%   rb_time     rebouncing time in second
%   Fs          sampling rate (default = 1Hz)
%
%   X.TPR       True Positive Ratio
%   X.FPR       False Positive Ratio / False alarm rate
%   X.rb_time   rebouncing time 
%   X.H         confusion matrix       
%

%    $Id: criteria4asyncbci.m,v 1.1 2005-06-23 16:49:26 schloegl Exp $
%    Copyright (C) 2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    This is part of the BIOSIG-toolbox http://biosig.sf.net/

%    This program is free software; you can redistribute it and/or modify
%    it under the terms of the GNU General Public License as published by
%    the Free Software Foundation; either version 2 of the License, or
%    (at your option) any later version.
%
%    This program is distributed in the hope that it will be useful,
%    but WITHOUT ANY WARRANTY; without even the implied warranty of
%    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%    GNU General Public License for more details.
%
%    You should have received a copy of the GNU General Public License
%    along with this program; if not, write to the Free Software
%    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA




X.datatype = 'Criteria_Asychronous_BCI'; 
X.rb_time = rbtime; 

if nargin>4,
        rbtime = round(rbtime*Fs);
        TRIG = round(TRIG*Fs); 
end;

% Detection of True Positives
[x,sx] = trigg(d,TRIG,0,rbtime);        % intervals where detection is counted as hit
x     = squeeze(reshape(x,sx));             % and bring it in proper shape

tmp   = any(x>0,1);                       % within interval, is the THRESHOLD reached? 
TP    = sum(tmp);                         % true positives/hits 
X.TPR = TP/length(tmp);                 % true positive ratio

% Detection of False Positives
for k = 1:length(TRIG)                  % de-select (mark with NaN) all samples within window
        d(TRIG(k):TRIG(k)+rbtime) = NaN;
end;
d(isnan(d)) = [];

tmp = diff(d>0)>0;
FP  = sum(tmp);                          % false positives
N   = length(tmp); 
X.FPR = FP/N;                           % false positive ratio 


% Confusion Matrix 
X.H = [TP,FP;length(TRIG)-TP,N-FP];     % confusion matrix 

