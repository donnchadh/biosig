function [SEN,SPEC,d,ACC,AREA]=roc(d,c,color);
% ROC receiver operator curve
% [SEN, SPEC, TH, ACC, AREA]=roc(d,c);
% d     DATA
% c     CLASS, vector with 0 and 1 
% 
% function [SEN, SPEC, TH, ACC]=roc(d1,d0);
% d1    DATA of class 1 
% d2    DATA of class 0
% 
% OUTPUT:
% SEN     sensitivity
% SPEC    specificity
% TH      Threshold
% ACC     accuracy
% AREA    area under ROC curve

%	$Id: roc.m,v 1.1 2003-11-19 12:32:04 schloegl Exp $
%	Copyright (C) 1997-2003 by  Alois Schloegl
%	a.schloegl@ieee.org	
%
% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% version 2 of the License, or (at your option) any later version.
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

MODE = all(size(d)==size(c)) & all(all((c==1) | (c==0)));
d=d(:);
c=c(:);
        
if ~MODE
        d2=c;
        c=[ones(size(d));zeros(size(d2))];
        d=[d;d2];
        fprintf(2,'Warning ROC: XXX\n')        
end;        

if nargin<3
        color='-';
end;

[D,I] = sort(d);
x = c(I);

% identify unique elements
if 0,
        fprintf(2,'Warning ROC: please take care\n');
        tmp= find(diff(D)>0);
        tmp=sort([1; tmp; tmp+1; length(d)]);%]',2*length(tmp+2),1);
        %D=d(tmp);
end;

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

SEN = [FN TP TN FP SEN SPEC ACC D];

%fill(TN/sum(x==0),FN/sum(x==1),'b');
%SEN=SEN(tmp,:);
%ACC=ACC(tmp);
%d=D(tmp);
d=D;

%plot(SEN(:,1)*100,SPEC*100,color);
plot(FPR*100,TPR*100,color);
%plot(FP*100,TP*100,color);
% fill([1; FP],[0;TP],'c');

%ylabel('true positive [%]');
%xlabel('false positive [%]');

%AREA=-trapz(FPR(tmp),TPR(tmp));

AREA=-trapz(FPR,TPR);
