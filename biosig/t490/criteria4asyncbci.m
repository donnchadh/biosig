function [X] = criteria4asyncbci(D0, TRIG, dbtime,Fs)
% CRITERIA4ASYNCBCI provides an evaluation criterion of asychronous BCI. 
% based on the discussion at the BCI2005 meeting in Rensellearville.  
%
% X = CRITERIA4ASYNCBCI(D-Threshold, TRIG, db_time,Fs)
%   D           detector output
%               each column the output for each of the N states. 
%               if all values are lower than the Threshold, the 
%               no-control (NC) state is assumed. 
%   Threshold   detector threshold
%   TRIG        list of trigger events in seconds 
%               if TRIG is a cell-array of a list of trigger times, 
%                       TRIG{n} contains the list of trigger times for
%                       state n. The number of detector traces (columns)
%                       must be 1+number of states.
%   db_time     debouncing time in second
%   Fs          sampling rate (default = 1Hz)
%
%   X.TPR       True Positive Ratio
%   X.FPR       False Positive Ratio / False alarm rate
%   X.db_time   debouncing time 
%   X.H         confusion matrix       
% 
%
% X = CRITERIA4ASYNCBCI(D, C, ., Fs)
%   D           State of detector output (discrete values, D=0 indicates no-control state) 
%   C           Target state
% 
%
% 
%
% References: 
%    http://chil.rice.edu/byrne/psyc540/pdf/StanislawTodorov99.pdf



%    $Id: criteria4asyncbci.m,v 1.4 2005-06-30 10:44:15 schloegl Exp $
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


% Wanted: 
% Julien Kronegg: 
%	Transition matrix (=extension of the 2-classes FP/FN/TP/TN to N classes) given in percentage of time; also named confusion matrix
%	Response time for activation
%	Response time for deactivation
% 


X.datatype = 'Criteria_Asychronous_BCI'; 
X.db_time = dbtime; 

if nargin<4,
        Fs = 1; 
end;
dbtime = round(dbtime*Fs);

if iscell(TRIG(1))      % multiple states
        if length(TRIG)~=size(D0,2),
                warning('number of states does not fit number of detector traces');     
        end;
        T = [];CL = [];
        for k=1:length(TRIG)
                t = TRIG{k}(:);
                T = [T;t]; 
                CL = [CL; repmat(k,length(t),1)];
        end;  
        TRIG = T; 
else
        CL = ones(numel(TRIG),1);
end;
[TRIG,ix] = sort(TRIG*Fs); 
CL = CL(ix);    % classlabels 

if size(D0,2)>1,
        [tmp,D] = max(D0,[],2);
        D(tmp<0)= 0;  
        D(isnan(tmp)) = NaN; 
else
        if 
        D = D0; 
end; 
C = zeros(size(D));

if any(diff(TRIG) < dbtime)
        warning('overlapping detection window');
end;
%### OPEN QUESTION(s): 
%###    is there a reasonable way to deal with overlapping windows ? 
%###    how to handle more than 1 active class

N0     = length(TRIG);                      % number of trigger events 
% Detection of True Positives
[d,sx] = trigg(D(:),TRIG,0,dbtime);     % intervals where detection is counted as hit
d      = reshape(d,sx(2:3));            % and bring it in proper shape
TP     = sum(any(d>0,1));                   % true positives/hits 

N3     = sum(diff(D>0)>1);                  % total number of detections (JH) 

% Detection of False Positives
d = real(D>0);
c = zeros(size(D));
for k = 1:length(TRIG)                 % de-select (mark with NaN) all samples within window
        d(TRIG(k):TRIG(k)+dbtime) = NaN;
        C(TRIG(k):TRIG(k)+dbtime) = CL(k);
end;
[FP1,N1] = sumskipnan(d);              % false positive ratio on a sample-basis
FP3  = sum(diff(d)>1);                 % number of detections

N2   = ceil((1+sum(~diff(isnan(d))))/2);          % number of intervals between trigger events (with non-overlapping windows) 
d(isnan(d)) = [];
tmp  = diff(d)>0;
FP2  = sum(tmp);                       % false positives
FPR2 = FP2/N2;                         % false positive ratio 

% Confusion Matrix 
% X.H = [TP,FP;length(TRIG)-TP,N-FP];      % confusion matrix 

% signal detection theory applied on a sample-basis
for k=1:size(D0,2),
        X.AS.AUC(k) = auc(D0(:,k),C); 
end;
[X.AS.KAP, X.AS.kapSD, X.AS.H, z] = kappa(D,C);


%  suggestion by Steve. # is this correct? 
X.SM.TP  = TP; 
X.SM.TPR = TP/length(TRIG); 
X.SM.FP  = FP1; 
X.SM.FPR = FP1/N1;
X.SM.N   = N1; 

% Jane Huggins' suggestion 
X.JH.TP  = TP; 
X.JH.TPR = TP/N0; 
X.JH.FP  = FP3; 
X.JH.FPR = FP3/N3; 
X.JH.N   = N3; 
X.JH.HFdiff = X.JH.TPR-X.JH.FPR;        % ???

% multi-state as suggested by Julien could go here


%### OPEN QUESTION: use of FPR1 or FPR2
X.TPR = TP/N0;                % true positive ratio
X.FPR = FP1/N1;                % or 
%X.FPR = FP2/N2;        % dismissed 
%X.FPR = FP3/N3; 
X.FPR = '?';





return;
% summary statistics
X.HFdiff = max(0,X.TPR-X.FPR);         % 
X.Dprime = norminv(X.TPR,0,1)-norminv(X.FPR,0,1);
tmp = X.TPR-X.FPR;
X.Aprime = .5+sign(tmp)*(tmp*tmp+abs(tmp))/(4*max(X.TPR,X.FPR)-4*X.TPR*X.FPR);
tmp2 = [X.TPR*(1-X.TPR),X.FPR*(1-X.FPR)];
X.Bsecond = sign(tmp)*diff(tmp2)/sum(tmp2);