function [CC]=train_sc(D,classlabel,MODE)
% Train statistical classifier
% 
%  CC = train_sc(D,classlabel)
%  CC = train_sc(D,classlabel,MODE)
%
%  The following classifier types are supported MODE.TYPE
%    'MDA'    mahalanobis distance based classifier
%    'MD2'    mahalanobis distance based classifier
%    'MD3'    mahalanobis distance based classifier
%    'GRB'    Gaussian radial basis function 
%    'QDA'    quadratic discriminant analysis
%    'LD2'    linear discriminant analysis (see LDBC2)
%    'LD3'    linear discriminant analysis (see LDBC3)
%    'LD4'    linear discriminant analysis (see LDBC4)
%    'GDBC'   general distance based classifier
%    'SVM','SVM1r'  support vector machines, one-vs-rest
%               MODE.c_value = 
%    'SVM11'  support vector machines, one-vs-one + voting
%               MODE.c_value = 
%    'RBF'    Support Vector Machines with RBF Kernel
%               MODE.c_value = 
%               MODE.gamma = 
%    'LPM'    Linear Programming Machine
%               MODE.c_value = 
%
%
%     
%              
% CC is a statistical classifier, it contains basically the mean 
% and the covariance of the data of each class. This information 
% is incoded in the so-called "extended covariance matrices".  
%
% CC can be used for various statistical classifiers included
%  LDA, MDA, QDA, GRB, etc. 
%
% see also: TEST_SC, COVM, LDBC2, LDBC3, LDBC4, MDBC, GDBC

%	$Id: train_sc.m,v 1.5 2006-06-23 15:20:21 schloegl Exp $
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

if nargin<3, MODE = ''; end;
if ischar(MODE) 
        tmp = MODE; 
        clear MODE; 
        MODE.TYPE = FUN;
elseif ~isfield(MODE,'TYPE')
        MODE.TYPE=''; 
end;        

% remove all NaN's
ix = any(isnan([D,classlabel]),2);
D(ix,:)=[];
classlabel(ix,:)=[];

[CC.Labels] = unique(classlabel(~isnan(classlabel)));

sz = size(D);
if sz(1)~=length(classlabel),
        error('length of data and classlabel does not fit');
end;

if 0, 

elseif ~isempty(strfind(lower(MODE.TYPE),'lpm'))
        % linear programming machine 
        % CPLEX optimizer: ILOG solver, ilog cplex 6.5 reference manual http://www.ilog.com
        MODE.TYPE = 'LPM';
        if ~isfield(MODE,'c_value')
                MODE.c_value = 1; 
        end
        LPM = train_LPM(D,classlabel,'C',MODE.c_value);
        CC.weights  = sparse([-LPM.b; LPM.w(:)]);
        CC.datatype = ['classifier:',lower(MODE.TYPE)];

        
elseif ~isempty(strfind(lower(MODE.TYPE),'rbf'))
        % Martin Hieden's RBF-SVM        
        if exist('svmpredict','file')==3,
                MODE.TYPE = 'SVM:LIB:RBF';
        else
                error('No SVM training algorithm available. Install LibSVM for Matlab.\n');
        end;
        if ~isfield(MODE,'gamma')
                MODE.gamma = 1; 
        end;
        if ~isfield(MODE,'c_value')
                MODE.c_value = 1; 
        end
        CC.options = sprintf('-c %g -t 2 -g %g', MODE.c_value, MODE.gamma);  %use RBF kernel, set C, set gamma
        CC.model = svmtrain(classlabel, D, CC.options);    % Call the training mex File     
        CC.datatype = ['classifier:',lower(MODE.TYPE)];

        
elseif isempty(strfind(lower(MODE.TYPE),'svm'))
        CC.datatype = ['classifier:statistical:',lower(MODE.TYPE)];
        CC.MD = repmat(NaN,[length(CC.Labels),sz(2)+[1,1]]);
        CC.NN = CC.MD;
        for k=1:length(CC.Labels),
                [CC.MD(k,:,:),CC.NN(k,:,:)] = covm(D(classlabel==CC.Labels(k),:),'E');
        end;        
        if strcmpi(MODE.TYPE,'LD2');
                CC.weights = ldbc2(CC); 
        elseif strcmpi(MODE.TYPE,'LD3');
                CC.weights = ldbc3(CC); 
        elseif strcmpi(MODE.TYPE,'LD4');
                CC.weights = ldbc4(CC); 
        end;

        
elseif ~isempty(strfind(lower(MODE.TYPE),'svm11'))
        if ~isfield(MODE,'c_value')
                MODE.c_value = 1; 
        end
        CC = train_svm11(D,classlabel,MODE.c_value);


elseif ~isempty(strfind(lower(MODE.TYPE),'svm'))
        if ~isfield(MODE,'c_value')
                MODE.c_value = 1; 
        end
        if 0,
        elseif exist('mexSVMTrain','file')==3,
                MODE.TYPE = 'SVM:OSU';
        elseif exist('SVMTrain','file')==3,
                MODE.TYPE = 'SVM:LIB';
        elseif exist('svcm_train','file')==2,
                MODE.TYPE = 'SVM:LOO';
        elseif exist('svmclass','file')==2,
                MODE.TYPE = 'SVM:KM';
        elseif exist('svc','file')==2,
                MODE.TYPE = 'SVM:Gunn';
        else
                error('No SVM training algorithm available. Install OSV-SVM, or LOO-SVM, or libSVM for Matlab.\n');
        end;

        CC = train_svm(D,classlabel,MODE);
        %CC = train_svm(D,classlabel,'SVM:LIB');
end;
