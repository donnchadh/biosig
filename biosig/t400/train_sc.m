function [CC]=train_sc(D,classlabel,MODE)
% Train a (statistical) classifier
% 
%  CC = train_sc(D,classlabel)
%  CC = train_sc(D,classlabel,MODE)
%
%  The following classifier types are supported MODE.TYPE
%    'MDA'      mahalanobis distance based classifier [1]
%    'MD2'      mahalanobis distance based classifier [1]
%    'MD3'      mahalanobis distance based classifier [1]
%    'GRB'      Gaussian radial basis function     [1]
%    'QDA'      quadratic discriminant analysis    [1]
%    'LD2'      linear discriminant analysis (see LDBC2) [1]
%    'LD3'      linear discriminant analysis (see LDBC3) [1]
%    'LD4'      linear discriminant analysis (see LDBC4) [1]
%    'GDBC'     general distance based classifier  [1]
%    'LDA/GSVD' LDA based on Generalized Singulare Value Decomposition [2,3]
%    'LD2/GSVD' LDA based on Generalized Singulare Value Decomposition [2,3]
%    'LD3/GSVD' LDA based on Generalized Singulare Value Decomposition [2,3]
%    'LD4/GSVD' LDA based on Generalized Singulare Value Decomposition [2,3]
%    ''         statistical classifier, requires Mode argument in TEST_SC	
%    '/GSVD'	GSVD and statistical classifier, requires Mode argument in TEST_SC	
%    'SVM','SVM1r'  support vector machines, one-vs-rest
%               MODE.hyperparameters.c_value = 
%    'SVM11'    support vector machines, one-vs-one + voting
%               MODE.hyperparameters.c_value = 
%    'RBF'      Support Vector Machines with RBF Kernel
%               MODE.hyperparameters.c_value = 
%               MODE.hyperparameters.gamma = 
%    'LPM'      Linear Programming Machine
%               MODE.hyperparameters.c_value = 
%
% 
% CC contains the model parameters of a classifier. Some time ago,     
% CC was a statistical classifier containing the mean 
% and the covariance of the data of each class (encoded in the 
%  so-called "extended covariance matrices". Nowadays, also other 
% classifiers are supported. 
%
% see also: TEST_SC, COVM, LDBC2, LDBC3, LDBC4, MDBC, GDBC
%
% References: 
% [1] R. Duda, P. Hart, and D. Stork, Pattern Classification, second ed. 
%       John Wiley & Sons, 2001. 
% [2] Peg Howland and Haesun Park,
%       Generalizing Discriminant Analysis Using the Generalized Singular Value Decomposition
%       IEEE Transactions on Pattern Analysis and Machine Intelligence, 26(8), 2004.
% [3] http://www-static.cc.gatech.edu/~kihwan23/face_recog_gsvd.htm
% [4] Jieping Ye, Ravi Janardan, Cheong Hee Park, Haesun Park
%       A new optimization criterion for generalized discriminant analysis
%       on undersampled problems.
%       The Third IEEE International Conference on Data Mining, Melbourne, Florida, USA
%       November 19 - 22, 2003

%	$Id: train_sc.m,v 1.9 2006-06-29 06:57:26 schloegl Exp $
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

if nargin<3, MODE = ''; end;
if ischar(MODE) 
        tmp = MODE; 
        clear MODE; 
        MODE.TYPE = tmp;
elseif ~isfield(MODE,'TYPE')
        MODE.TYPE=''; 
end;        

sz = size(D);
if sz(1)~=length(classlabel),
        error('length of data and classlabel does not fit');
end;

% remove all NaN's
ix = any(isnan([D,classlabel]),2);
D(ix,:)=[];
classlabel(ix,:)=[];

[CC.Labels] = unique(classlabel);

sz = size(D);
if sz(1)~=length(classlabel),
        error('length of data and classlabel does not fit');
end;
if ~isfield(MODE,'hyperparameter')
        MODE.hyperparameter = [];
end


if 0, 

elseif ~isempty(strfind(lower(MODE.TYPE),'lpm'))
        % linear programming machine 
        % CPLEX optimizer: ILOG solver, ilog cplex 6.5 reference manual http://www.ilog.com
        MODE.TYPE = 'LPM';
        if ~isfield(MODE.hyperparameter,'c_value')
                MODE.hyperparameter.c_value = 1; 
        end

        M = length(CC.Labels);
        if M==2, M=1; end;   % For a 2-class problem, only 1 Discriminant is needed 
        for k = 1:M,
                LPM = train_LPM(D,(classlabel==CC.Labels(k)),'C',MODE.hyperparameter.c_value);
                CC.weights(:,k) = [-LPM.b; LPM.w(:)];
        end;
        CC.hyperparameters.c_value = MODE.hyperparameter.c_value; 
        CC.datatype = ['classifier:',lower(MODE.TYPE)];

        
elseif ~isempty(strfind(lower(MODE.TYPE),'rbf'))
        % Martin Hieden's RBF-SVM        
        if exist('svmpredict','file')==3,
                MODE.TYPE = 'SVM:LIB:RBF';
        else
                error('No SVM training algorithm available. Install LibSVM for Matlab.\n');
        end;
        if ~isfield(MODE.hyperparameter,'gamma')
                MODE.hyperparameter.gamma = 1; 
        end
        if ~isfield(MODE.hyperparameter,'c_value')
                MODE.hyperparameter.c_value = 1; 
        end
        CC.options = sprintf('-c %g -t 2 -g %g', MODE.hyperparameter.c_value, MODE.hyperparameter.gamma);  %use RBF kernel, set C, set gamma
        CC.hyperparameters.c_value = MODE.hyperparameter.c_value; 
        CC.hyperparameters.gamma = MODE.hyperparameter.gamma; 
        CC.model = svmtrain(classlabel, D, CC.options);    % Call the training mex File     
        CC.datatype = ['classifier:',lower(MODE.TYPE)];


elseif ~isempty(strfind(lower(MODE.TYPE),'/gsvd'))
	% [1] Peg Howland and Haesun Park, 2004. 
        %       Generalizing Discriminant Analysis Using the Generalized Singular Value Decomposition
        %       IEEE Transactions on Pattern Analysis and Machine Intelligence, 26(8), 2004.
        % [3] http://www-static.cc.gatech.edu/~kihwan23/face_recog_gsvd.htm

        Hw = zeros(size(D)); 
	m0 = mean(D); 
	for k = 1:length(CC.Labels)
		ix = find(classlabel==CC.Labels(k));
		N(k) = length(ix); 
		[Hw(ix,:), mu(k,:)] = center(D(ix,:));
		Hb(k,:) = sqrt(N(k))*(mu(k,:)-m0);
	end;
        [P,R,Q] = svd([Hb;Hw],0);
	t = rank(R);

        clear Hw Hb mu; 
        %[size(D);size(P);size(Q);size(R)]
        R = R(1:t,1:t);
        %P = P(1:size(D,1),1:t); 
        %Q = Q(1:t,:);
        [U,E,W] = svd(P(1:size(D,1),1:t),0);
        %[size(U);size(E);size(W)]
        clear U E P;  
        %[size(Q);size(R);size(W)]
        G = Q(1:t,:)'*[R\W];
        %G = G(:,1:t);  % not needed 
        
        CC = train_sc(D*G,classlabel,MODE.TYPE(1:find(MODE.TYPE=='/')));
        CC.G = G; 
        CC.weights = [CC.weights(1,:); G*CC.weights(2:end,:)];
        CC.datatype = ['classifier:statistical:',lower(MODE.TYPE)];


elseif ~isempty(strfind(lower(MODE.TYPE),'sparse_lda'))
	% J.D. Tebbens and P.Schlesinger, Improving Implementation of Linear Discriminant Analysis for the Small Sample Size Problem
			 
	% X = D';
	[n,p] = size(D);
	g = length(CL); 
	
		G = sparse(n,g);
		for k = 1:g 
			M(k,:) = mean(D(:,classlabel==CL(k)),2)';
			G(find(classlabel==CL(k)),k) = 1; 
		end; 
	% step1 
		mu = mean(D,2);
		CC = covm(D','M');
		X1 = CC * ones(n,1);
		
		tmp1 = -ones(n,1)*X1'/n + sum(CC(:)); 
		CC   = CC - X1*ones(1,n)/n + tmp1;   
		
		[v,d]= eig(CC); 
		[D1,ix] = diag(d);
		D1 = diag(D1);  
		V1 = v(:,ix); 
		clear v d; 
		
	% step2 
		B1 = (G*M*D - G*M*mu*ones(1,p) + tmp1)*V1*inv(D1);

	% step3 
		[v2,d2]=eigs(B1'*B1);
		[tmp,ix]=sort(-abs(diag(d2)));
		%V2 = v2(:,ix(1:g-1))* ;
		
		%eigs % eig 

	% step4b 

	% step4b 
		%eig

	% step5 
		D*(V1*(D.^(-1/2)) - ones(n,1)*(ones(1,n)*V1*(D.^(-1/2))/n));

        warning('sparse LDA not ready (yet)');
        CC.weights = zeros(size(D,2)+1,1);
                
        
elseif ~isempty(strfind(lower(MODE.TYPE),'svm11'))
        % 1-versus-1 scheme 
        if ~isfield(MODE.hyperparameter,'c_value')
                MODE.hyperparameter.c_value = 1; 
        end
        %CC = train_svm11(D,classlabel,MODE.hyperparameter.c_value);

        CC.options=sprintf('-c %g -t 0',MODE.hyperparameter.c_value);  %use linear kernel, set C
        CC.hyperparameters.c_value = MODE.hyperparameter.c_value; 

        CC.model = svmtrain(classlabel, D, CC.options);    % Call the training mex File

        FUN = 'SVM:LIB:1vs1';
        CC.datatype = ['classifier:',lower(FUN)];


elseif ~isempty(strfind(lower(MODE.TYPE),'svm'))
        if ~isfield(MODE.hyperparameter,'c_value')
                MODE.hyperparameter.c_value = 1; 
        end
        if any(MODE.TYPE==':'),
                % nothing to be done
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

        %%CC = train_svm(D,classlabel,MODE);

        M = length(CC.Labels);
        if M==2, M=1; end;
        CC.weights = repmat(NaN, sz(2)+1, M);
        for k = 1:M,
                cl = sign((classlabel~=CC.Labels(k))-.5);
                if strcmp(MODE.TYPE, 'SVM:LIB');
                        if isfield(MODE,'options')
                                CC.options = MODE.options;
                        else
                                CC.options = sprintf('-s 0 -c %f -t 0 -d 1', MODE.hyperparameter.c_value);      % C-SVC, C=1, linear kernel, degree = 1,
                        end;
                        model = svmtrain(cl, D, CC.options);    % C-SVC, C=1, linear kernel, degree = 1,
                        w = -cl(1) * model.SVs' * model.sv_coef;  %Calculate decision hyperplane weight vector
                        % ensure correct sign of weight vector and Bias according to class label
                        Bias = -model.rho * cl(1);

                elseif strcmp(MODE.TYPE, 'SVM:OSU');
                        [AlphaY, SVs, Bias, Parameters, nSV, nLabel] = mexSVMTrain(D', cl', [0 1 1 1 MODE.hyperparameter.c_value]);    % Linear Kernel, C=1; degree=1, c-SVM
                        w = -SVs * AlphaY'*cl(1);  %Calculate decision hyperplane weight vector
                        % ensure correct sign of weight vector and Bias according to class label
                        Bias = -Bias * cl(1);

                elseif strcmp(MODE.TYPE, 'SVM:LOO');
                        [a, Bias, g, inds, inde, indw]  = svcm_train(D, cl, MODE.hyperparameter.c_value); % C = 1;
                        w = D(inds,:)' * (a(inds).*cl(inds)) ;

                elseif strcmp(MODE.TYPE, 'SVM:Gunn');
                        [nsv, alpha, Bias,svi]  = svc(center(D), cl, 1, MODE.hyperparameter.c_value); % linear kernel, C = 1;
                        w = D(svi,:)' * alpha(svi) * cl(1);
                        Bias = mean(D*w);

                elseif strcmp(MODE.TYPE, 'SVM:KM');
                        [xsup,w1,Bias,inds,timeps,alpha] = svmclass(D, cl, MODE.hyperparameter.c_value, 1, 'poly', 1); % C = 1;
                        w = -D(inds,:)' * w1;

                else
                        fprintf(2,'Error TRAIN_SVM: no SVM training algorithm available\n');
                        return;
                end

                CC.weights(1,k) = -Bias;
                CC.weights(2:end,k) = w;
        end;
        CC.hyperparameters.c_value = MODE.hyperparameter.c_value; 
        CC.datatype = ['classifier:',lower(MODE.TYPE)];


else          % Linear and Quadratic statistical classifiers 
        CC.datatype = ['classifier:statistical:',lower(MODE.TYPE)];
        CC.MD = repmat(NaN,[length(CC.Labels),sz(2)+[1,1]]);
        CC.NN = CC.MD;
        for k = 1:length(CC.Labels),
                [CC.MD(k,:,:),CC.NN(k,:,:)] = covm(D(classlabel==CC.Labels(k),:),'E');
        end;        
        if strcmpi(MODE.TYPE,'LD2');
                CC.weights = ldbc2(CC); 
        elseif strcmpi(MODE.TYPE,'LD4');
                CC.weights = ldbc4(CC); 
        elseif strncmpi(MODE.TYPE,'LD3',2);
                CC.weights = ldbc3(CC); 
        end;
    
end;
