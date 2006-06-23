function [R]=test_sc(CC,D,mode,classlabel)
% TEST_SC: apply statistical and SVM classifier to test data 
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
%    TYPE = 'MD2'    mahalanobis distance based classifier
%    TYPE = 'MD3'    mahalanobis distance based classifier
%    TYPE = 'GRB'    Gaussian radial basis function 
%    TYPE = 'QDA'    quadratic discriminant analysis
%    TYPE = 'LD2'    linear discriminant analysis (see LDBC2)
%    TYPE = 'LD3'    linear discriminant analysis (see LDBC3)
%    TYPE = 'LD4'    linear discriminant analysis (see LDBC4)
%    TYPE = 'GDBC'   general distance based classifier
%    TYPE = 'SVM'    support vector machines
% 
% see also: MDBC, GDBC, LDBC2, LDBC3, LDBC4, TRAIN_SC, TRAIN_SVM

%	$Id: test_sc.m,v 1.6 2006-06-23 15:20:21 schloegl Exp $
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

if nargin<3, 
        mode = [];
end;
[t1,t] = strtok(CC.datatype,':');
[t2,t] = strtok(t,':');
[t3,t] = strtok(t,':');
if ~strcmp(t1,'classifier'), return; end; 

if 0, 
        
elseif strcmp(CC.datatype,'classifier:svm:lib:1vs1');
        d = test_svm11(CC, D, classlabel); 
        
elseif strcmp(CC.datatype,'classifier:svm:lib:rbf');
        ix = any(isnan([D,classlabel]),2);
        D(ix,:) = [];
        classlabel(ix,:) = [];

        [cl, accuracy] = svmpredict(classlabel, D, CC.model);   %Use the classifier

        %Create a pseudo tsd matrix for bci4eval
        d = zeros(size(cl,1), CC.model.nr_class);
        for i = 1:size(cl,1)
                d(i,(cl(i)))=1;
        end
        
elseif isfield(CC,'weights'); %strcmpi(t2,'svm') | (strcmpi(t2,'statistical') & strncmpi(t3,'ld',2)) ;
        % linear classifiers like: LDA, SVM, LPM 
        % d = [ones(size(D,1),1), D] * CC.weights;
        d = D * CC.weights(2:end,:) + CC.weights(1);
        
elseif strcmp(t2,'statistical');
        if isempty(mode)
                mode = upper(t3); 
        end;
        if strcmpi(mode,'LD2'),
                d = ldbc2(CC.MD,D);
        elseif strcmpi(mode,'LD3');
                d = ldbc3(CC.MD,D);
        elseif strcmpi(mode,'LD4');
                d = ldbc4(CC.MD,D);
        elseif strcmpi(mode,'MDA');
                d = -(mdbc(CC.MD,D).^2);
        elseif strcmpi(mode,'MD2');
                d = -mdbc(CC.MD,D);
        elseif strcmpi(mode,'GDBC');
                [GDBC,kap,acc,H,MDBC] = gdbc(CC.MD,D);
                d = exp(-MDBC{7}/2);
        elseif strcmpi(mode,'MD3');
                [GDBC,kap,acc,H,MDBC] = gdbc(CC.MD,D);
                d = -GDBC;
        elseif strcmpi(mode,'QDA');     
                [GDBC,kap,acc,H,MDBC] = gdbc(CC.MD,D);
                d = MDBC{4};
        elseif strcmpi(mode,'GRB');     % Gaussian RBF
                d = exp(-mdbc(CC.MD,D)/2);
        end;
        
else
        fprintf(2,'Error TEST_SC: unknown classifier\n'); 
        return; 
end;

[tmp,cl] = max(d,[],2);
cl(isnan(tmp)) = NaN; 

R.output = d; 
R.classlabel = cl; 

if nargin>3,
        tmp = CC.Labels(cl(~isnan(cl)));
        [R.kappa,R.sd,R.H,z,R.ACC] = kappa(classlabel(:),tmp(:));
end;
