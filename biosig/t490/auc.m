function [AREA,d,SEN,SPEC,ACC] = auc(d,c,color);
% AUC calculates Area-under-ROC curve
%
% function [AREA,TH,SEN,SPEC,ACC] = auc(d,c,color);
% d     DATA
% c     CLASS, vector with 0 and 1 
% color optional, plots ROC curve
% 
% function [AREA,TH,SEN,SPEC,ACC]=auc(d1,d0,color);
% d1    DATA of class 1 
% d2    DATA of class 0
% color optional, plots ROC curve
% 
% OUTPUT:
% AREA    area under ROC curve
% TH      Threshold
% SEN     sensitivity
% SPEC    specificity
% ACC     accuracy

%	$Id: auc.m,v 1.1 2003-11-19 12:32:04 schloegl Exp $
%	Copyright (c) 1997-2003 by  Alois Schloegl
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

MODE = all(size(d)==size(c)) & all(all((c==1) | (c==0)));
d=d(:);
c=c(:);
        
if ~MODE
        d2=c;
        c=[ones(size(d));zeros(size(d2))];
        d=[d;d2];
end;        

if nargin<3
        color='-';
end;

[d,I] = sort(d);
x = c(I);

FN   = cumsum(x==1)/sum(x==1);
TN   = cumsum(x==0)/sum(x==0);
AREA = -trapz(1-TN,1-FN);

if nargin>2,  
        plot((1-TN)*100,(1-FN)*100,color);
end;

if nargout<3, return; end; 
TP = 1-FN;
SEN = TP./(TP+FN);

if nargout<4, return; end; 
FP = 1-TN;
SPEC= TN./(TN+FP);

if nargout<5, return; end; 
ACC = (TP+TN)./(TP+TN+FP+FN);


