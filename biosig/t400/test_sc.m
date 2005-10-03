function [R]=test_sc(CC,D,mode,classlabel)
% TEST_SC: test statistical classifier
%
%  R = test_sc(CC,D,TYPE [,target_Classlabel]) 
%       R.output     output distance for each class
%       R.classlabel class for output data
%  The target class is optional. If it is provided, the following values are returned. 
%       R.kappa Cohen's kappa coefficient
%       R.ACC   Classification accuracy 
%       R.H     Confusion matrix 
%
%  The following classifier types are provide 
%    TYPE = 'MDA'    mahalanobis distance based classifier
%    TYPE = 'GRB'    Gaussian radial basis function 
%    TYPE = 'QDA'    quadratic discriminant analysis
%    TYPE = 'LD2'    linear discriminant analysis (see LDBC2)
%    TYPE = 'LD3'    linear discriminant analysis (see LDBC2)
%    TYPE = 'LD4'    linear discriminant analysis (see LDBC2)
%    TYPE = 'GDBC'   general distance based classifier
%    TYPE = 'SVM'    support vector machines
% 
% see also: COVM, DECOVM, R2, MDBC, GDBC, LDBC2, LDBC3, LDBC4

%	$Id: test_sc.m,v 1.2 2005-10-03 13:18:56 schloegl Exp $
%	Copyright (C) 2005 by Alois Schloegl <a.schloegl@ieee.org>	
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


if strcmpi(mode,'LD2'),
        d = ldbc2(CC.MD,D);
        [tmp,cl] = max(d,[],2);
elseif strcmpi(mode,'LD3');
        d = ldbc3(CC.MD,D);
        [tmp,cl] = max(d,[],2);
elseif strcmpi(mode,'LD4');
        d = ldbc4(CC.MD,D);
        [tmp,cl] = max(d,[],2);
elseif strcmpi(mode,'MDA');
        d = mdbc(CC.MD,D);
        [tmp,cl] = min(d,[],2);
elseif strcmpi(mode,'GDBC');
        [GDBC,kap,acc,H,MDBC] = gdbc(CC.MD,D);
        d = exp(-MDBC{7}/2);
        [tmp,cl] = max(d,[],2);
elseif strcmpi(mode,'QDA');
        [GDBC,kap,acc,H,MDBC] = gdbc(CC.MD,D);
        d = MDBC{4};
        [tmp,cl] = max(d,[],2);
elseif strcmpi(mode,'GRB');
        d = mdbc(CC.MD,D);
        [tmp,cl] = max(exp(-d/2),[],2);
elseif strcmpi(mode,'SVM');
        d = D*CC.w+CC.b;
        [tmp,cl] = max(d,[],2);
end;
cl(all(isnan(d),2)) = NaN; 

R.output = d; 
R.classlabel = cl; 

if nargin>3,
        tmp = CC.Labels(cl);
        [R.kappa,sd,R.H,z,R.ACC] = kappa(classlabel(:),tmp(:));
end;
