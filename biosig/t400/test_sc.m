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
%  The same classifier as in TRAIN_SC are supported. If a statistical 
% classifier is used, TYPE can be used to modify the classifier. 
%    TYPE = 'MDA'    mahalanobis distance based classifier
%    TYPE = 'MD2'    mahalanobis distance based classifier
%    TYPE = 'MD3'    mahalanobis distance based classifier
%    TYPE = 'GRB'    Gaussian radial basis function 
%    TYPE = 'QDA'    quadratic discriminant analysis
%    TYPE = 'LD2'    linear discriminant analysis (see LDBC2)
%    TYPE = 'LD3'    linear discriminant analysis (see LDBC3)
%    TYPE = 'LD4'    linear discriminant analysis (see LDBC4)
%    TYPE = 'GDBC'   general distance based classifier
% 
% see also: TRAIN_SC, MDBC, GDBC, LDBC2, LDBC3, LDBC4, 

%	$Id: test_sc.m,v 1.10 2006-06-29 06:57:54 schloegl Exp $
%	Copyright (C) 2005,2006 by Alois Schloegl <a.schloegl@ieee.org>	
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

POS1 = strfind(CC.datatype,'/gsvd');

if 0, 
        
elseif strcmp(CC.datatype,'classifier:svm:lib:1vs1') | strcmp(CC.datatype,'classifier:svm:lib:rbf');
        %d = test_svm11(CC, D, classlabel); 
        [cl, accuracy] = svmpredict(classlabel, D, CC.model);   %Use the classifier

        %Create a pseudo tsd matrix for bci4eval
        d = zeros(size(cl,1), CC.model.nr_class);
        for i = 1:size(cl,1)
                tsd(i,(cl(i)))=1;
        end
        
        
elseif isfield(CC,'weights'); %strcmpi(t2,'svm') | (strcmpi(t2,'statistical') & strncmpi(t3,'ld',2)) ;
        % linear classifiers like: LDA, SVM, LPM 
        %d = [ones(size(D,1),1), D] * CC.weights;
        d = repmat(NaN,size(D,1),size(CC.weights,2));
        for k = 1:size(CC.weights,2),
                d(:,k) = D * CC.weights(2:end,k) + CC.weights(1,k);
        end;        
        
        
elseif ~isempty(POS1)	% GSVD
	CC.datatype = CC.datatype(1:POS1(1)-1);
	r = test_sc(CC,D*CC.G);
	d = r.output; 


elseif strcmp(t2,'statistical');
        if isempty(mode)
                mode.TYPE = upper(t3); 
        end;
        if strcmpi(mode.TYPE,'LD2'),
                d = ldbc2(CC,D);
        elseif strcmpi(mode.TYPE,'LD3');
                d = ldbc3(CC,D);
        elseif strcmpi(mode.TYPE,'LD4');
                d = ldbc4(CC,D);
        elseif strcmpi(mode.TYPE,'MDA');
                d = -(mdbc(CC,D).^2);
        elseif strcmpi(mode.TYPE,'MD2');
                d = -mdbc(CC,D);
        elseif strcmpi(mode.TYPE,'GDBC');
                [GDBC,kap,acc,H,MDBC] = gdbc(CC,D);
                d = exp(-MDBC{7}/2);
        elseif strcmpi(mode.TYPE,'MD3');
                [GDBC,kap,acc,H,MDBC] = gdbc(CC,D);
                d = GDBC;
        elseif strcmpi(mode.TYPE,'QDA');     
                [GDBC,kap,acc,H,MDBC] = gdbc(CC,D);
                d = MDBC{4};
        elseif strcmpi(mode.TYPE,'GRB');     % Gaussian RBF
                d = exp(-mdbc(CC,D)/2);
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
