function [X] = criteria4asyncbci(d, TRIG, rbtime,Fs)
% CRITERIA4ASYNCBCI provides an evaluation criterion of asychronous BCI. 
% based on the discussion at the BCI2005 meeting in Rensellearville.  
%
% X = CRITERIA4ASYNCBCI(D, TRIG, rb_time,Fs)
%   D           detector output (assuming Threshold = 0); 
%   TRIG        list of trigger times in seconds 
%   rb_time     rebouncing time in second
%   Fs          sampling rate (default = 1Hz)
%
%   X.TPR       True Positive Ratio
%   X.FPR       False Positive Ratio / False alarm rate
%   X.rb_time   rebouncing time 
%   X.H         confusion matrix       
%
% References: 
%    http://chil.rice.edu/byrne/psyc540/pdf/StanislawTodorov99.pdf



%    $Id: criteria4asyncbci.m,v 1.2 2005-06-24 14:40:12 schloegl Exp $
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

if nargin>3,
        rbtime = round(rbtime*Fs);
        TRIG = round(TRIG*Fs); 
end;
if sum(size(d)>1)>1,
        error('Detector must be a vector');
end;

TRIG = sort(TRIG); 
if any(diff(TRIG) < rbtime)
        warning('overlapping detection window');
end;

%### OPEN QUESTION: is there a reasonable way to deal with overlapping windows ? 

% Detection of True Positives
[x,sx]= trigg(d(:),TRIG,0,rbtime);      % intervals where detection is counted as hit
x     = reshape(x,sx(2:3));          % and bring it in proper shape

tmp   = any(x>0,1);                      % within interval, is the THRESHOLD reached? 
TP    = sum(tmp);                        % true positives/hits 
X.TPR = TP/length(tmp);                  % true positive ratio


% Detection of False Positives
d = d>0;
for k = 1:length(TRIG)                   % de-select (mark with NaN) all samples within window
        d(TRIG(k):TRIG(k)+rbtime) = NaN;
end;
[FP1,N1] = sumskipnan(d); 
FPR1 = FP1/N1;                         % false positive ratio on a sample-basis

N2  = sum(diff(isnan(d))>1)+1;           % number of intervals 
d(isnan(d)) = [];
tmp = diff(d>0)>0;
FP2 = sum(tmp);                          % false positives
FPR2 = FP2/N2;                         % false positive ratio 

%### OPEN QUESTION: use of FPR1 or FPR2
X.FPR = FPR1; % or 
X.FPR = FPR2; 
X.FPR = '?'

% Confusion Matrix 
X.H = [TP,FP;length(TRIG)-TP,N-FP];      % confusion matrix 
