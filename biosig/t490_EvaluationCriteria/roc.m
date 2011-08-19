function [SEN,SPEC,d,ACC,AREA,YI,c]=roc(d,c,varargin);
% ROC plots receiver operator curve and computes derived statistics.
%   In order to speed up the plotting, no more than 10000 data points are shown at most. 
%   [if you need more, you can change the source code] 	 
% 
% [...] = roc(d,c);
% [...]=roc(d1,d0);
% [...] = roc(...,s);
% d	DATA
% c	CLASS, vector with 0 and 1
% d1	DATA of class 1
% d2	DATA of class 0
% s	line style (as used in plot)
%
% [SEN, SPEC, TH, ACC, AUC,Yi,idx]=roc(...);
% OUTPUT:
% SEN     sensitivity
% SPEC    specificity
% TH      Threshold
% ACC     accuracy
% AUC     area under ROC curve
% Yi	  max(SEN+SPEC-1), Youden index
% c	  TH(c) is the threshold that maximizes Yi
%
% see also: AUC, PLOT

%	$Id$
%	Copyright (c) 1997-2003,2005,2007,2010,2011 by  Alois Schloegl <alois.schloegl@gmail.com>
%	This is part of the BIOSIG-toolbox http://biosig.sf.net/
%
% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% version 3 of the License, or (at your option) any later version.
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
%


% Problem : Wenn die Schwellwerte mehrfach vorkommen, kann es zu Ambiguiten kommen, welche sich auf die AUC auswirken.

MODE = all(size(d)==size(c)) && all(all((c==1) | (c==0) | isnan(c)));
d=d(:);
c=c(:);

if ~MODE
        d2=c;
        c=[ones(size(d));zeros(size(d2))];
        d=[d;d2];
        fprintf(2,'Warning ROC: XXX\n')
else
	ix = ~any(isnan([d,c]),2);
	c = c(ix); 
	d = d(ix); 
end;

% handle (ignore) NaN's
c = c(~isnan(d));
d = d(~isnan(d));

if nargin<3
        color='-';
end;

[D,I] = sort(d);
x = c(I);

FNR = cumsum(x==1)/sum(x==1);
TPR = 1-FNR;

TNR = cumsum(x==0)/sum(x==0);
FPR = 1-TNR;

FN = cumsum(x==1);
TP = sum(x==1)-FN;

TN = cumsum(x==0);
FP = sum(x==0)-TN;

SEN = TP./(TP+FN);
SPEC= TN./(TN+FP);
ACC = (TP+TN)./(TP+TN+FP+FN);

% SEN = [FN TP TN FP SEN SPEC ACC D];

d=D;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%  display only 10000 points at most. 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
len = length(FPR);
delta = max(1,floor(len/5000));
ix = [1:delta:len-1,len];
plot(FPR(ix)*100,TPR(ix)*100, varargin{:});

%ylabel('Sensitivity (true positive ratio) [%]');
%xlabel('1-Specificity (false positive ratio) [%]');

% area under the ROC curve
AREA = -diff(FPR)' * (TPR(1:end-1)+TPR(2:end))/2;

% Youden index
[YI,c] = max(SEN+SPEC-1);

