function [GDBC,kap,acc,H,MDBC]=gdbc(ECM,Y,CL)
% GDBC General discriminant-based classifier
% [MDBC] = gdbc(ECM);
% GDBC is a multiple discriminator
%
% [GDBC,kap,acc,H] = gdbc(ECM,D,CL);
% calculates the Log-Likelihood to each class
% the maximum CHI2-value and the corresponding class are obtained with
%
% ECM 	is the extended covariance matrix
% D     data
%
% GDBC  classifier
% ll	log-likelihood
% C     classification output
% 
% see also: DECOVM, ECOVM.M, R2.M, MDBC, LDBC
%
% References: 
%  [1] J. Bortz, Statistik für Sozialwissenschaftler, 5. Auflage, Springer (1999).  
%

%	$Revision: 1.7 $
%	$Id: gdbc.m,v 1.7 2006-07-12 19:43:09 schloegl Exp $
%	Copyright (c) 1999-2004,2006 by Alois Schloegl <a.schloegl@ieee.org>	
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


warning('this function is obsolete and replaced by TRAIN_SC and TEST_SC');


NC = size(ECM);
if length(NC)<3,
        if iscell(ECM(1)),
                NC = [max(NC(1:2)),size(ECM{1})];
		tmp = ECM;
		ECM = zeros([NC(1),size(tmp{1})]);
                for k = 1:NC(1),
                        ECM(k,:,:) = tmp{k};
                end;

	elseif isfield(ECM,'COV') & isfield(ECM,'NN')
    		ECM = ECM.COV./ECM.NN; 
    		NC  = size(ECM);
        
	elseif isfield(ECM,'MD') & isfield(ECM,'NN')
    		ECM = ECM.MD./ECM.NN; 
    		NC  = size(ECM);
        
        elseif isstruct(ECM),
                x = ECM;
                NC=[length(x.IR),size(x.IR{1})];
        elseif NC(1)==NC(2)
                ECM{1}=ECM;
        end;

elseif (length(NC)==3) & (NC(2)==NC(3)),
        
elseif isfield(ECM,'COV') & isfield(ECM,'NN')
        ECM = ECM.COV./ECM.NN; 
        NC  = size(ECM);
        
elseif 0; 
        %ECM = num2cell(ECM,[2,3]);
        for k = 1:NC(1),
                IR{k} = squeeze(ECM(k,:,:));
        end;
        ECM = IR;
else
        
end

if nargin>1,
        if NC(2) == size(Y,2)+1;
                Y = [ones(size(Y,1),1),Y];  % add 1-column
                warning('LLBC: 1-column added to data');
        elseif ~all(Y(:,1)==1 | isnan(Y(:,1)))
                warning('first column does not contain ones only'); 
        end;
end;

if exist('x')~=1,
                c  = size(ECM,2);
		ECM0 = sum(ECM,1);
                nn = ECM0(1,1,1);	% number of samples in training set for class k
                XC = squeeze(ECM0(1,:,:))/nn;		% normalize correlation matrix
                M  = XC(1,2:NC(2));		% mean 
                S  = XC(2:NC(2),2:NC(2)) - M'*M;% covariance matrix
                %M  = M/nn; S=S/(nn-1);
                ICOV0 = inv(S);
		ICOV1 = zeros(size(S));
        for k = 1:NC(1);
                %[M,sd,S,xc,N] = decovm(ECM{k});  %decompose ECM
                %c  = size(ECM,2);
                nn = ECM(k,1,1);	% number of samples in training set for class k
                XC = squeeze(ECM(k,:,:))/nn;		% normalize correlation matrix
                M  = XC(1,2:NC(2));		% mean 
                S  = XC(2:NC(2),2:NC(2)) - M'*M;% covariance matrix
                %M  = M/nn; S=S/(nn-1);

		%ICOV(1) = ICOV(1) + (XC(2:NC(2),2:NC(2)) - )/nn

		x.M{k} = M; 
                x.IR{k} = [-M;eye(NC(2)-1)]*inv(S)*[-M',eye(NC(2)-1)];  % inverse correlation matrix extended by mean 
                x.IR0{k} = [-M;eye(NC(2)-1)]*ICOV0*[-M',eye(NC(2)-1)];  % inverse correlation matrix extended by mean 
                d = NC(2)-1;
                x.logSF(k)  = log(nn) - d/2*log(2*pi) - det(S)/2;
                x.logSF2(k) = -2*log(nn/sum(ECM(:,1,1)));
                x.logSF3(k) = d*log(2*pi) + log(det(S));
                x.logSF4(k) = log(det(S)) + 2*log(nn);
                x.logSF5(k) = log(det(S));
                x.logSF6(k) = log(det(S)) - 2*log(nn/sum(ECM(:,1,1)));
                x.logSF7(k) = log(det(S)) + d*log(2*pi) - 2*log(nn/sum(ECM(:,1,1)));
                x.SF(k) = nn/sqrt((2*pi)^d * det(S));
                x.datatype='LLBC';
        end;
end;

if nargin<2,
        GDBC = x;	% inverse correlation matrix
else
        LogLik= zeros(size(Y,1),NC(1)); %alllocate memory
        for k = 1:NC(1);  
                MDBC{1}(:,k) = sum((Y*x.IR{k}).*Y,2); % calculate distance of each data point to each class
                % LogLik(:,k) = x.logSF2(k) - MDBC/2;
                MDBC{2}(:,k) = MDBC{1}(:,k) + x.logSF2(k);
                MDBC{3}(:,k) = MDBC{1}(:,k) + x.logSF4(k);      
                MDBC{4}(:,k) = MDBC{1}(:,k) + x.logSF5(k);      % [1] (18.33) QCF - quadratic classification function  
                MDBC{5}(:,k) = MDBC{1}(:,k) - x.logSF5(k);
                MDBC{6}(:,k) = MDBC{1}(:,k) + x.logSF6(k);      % [1] (16.13, 18.37) minimum CHI² rule, or MC-rule; need to look at this, too
                MDBC{7}(:,k) = MDBC{1}(:,k) + x.logSF7(k);      % 

		% Euclidian Distance 
		tmp = Y(:,2:end) - ones(size(Y,1),1) * x.M{k};
		MDBC{8}(:,k) = sum(tmp.*tmp,2); % calculate distance of each data point to each class

		% NN-Rule
                MDBC{9}(:,k) = sum((tmp*ICOV0).*tmp,2); % calculate distance of each data point to each class
                MDBC{10}(:,k) = MDBC{9}(:,k) + x.logSF2(k);  
        end;
        
        if nargin>2,
                for k = 1:10,
                        tmp = exp(-MDBC{k}/2);
                        p = tmp./repmat(sum(tmp,2),1,size(tmp,2));
                        
                        [tmp,ix] = max(p,[],2);
                        tmp = isnan(tmp);
                        ix(tmp) = NaN; %NC(1)+1;	% not classified; any value but not 1:length(MD)
                        
                        [kap(k), se, H{k}, zscore, p0, SA] = kappa(ix(~isnan(ix)), CL(~isnan(ix)),10);
                        acc(k) = sum(diag(H{k}))/sum(H{k}(:));
                end;
        else
                kap = []; acc=[]; H = [];
        end;
        tmp  = exp(-MDBC{7}/2);
        GDBC = tmp./repmat(sum(tmp,2),1,size(tmp,2));  % Zuordungswahrscheinlichkeit [1], p.601, equ (18.39)
        
        if nargout>=2,
                [tmp,ix] = max(GDBC,[],2);
                ix(isnan(tmp)) = NC(1)+1;	% not classified
        end;
end;
