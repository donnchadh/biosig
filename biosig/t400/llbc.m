function [LogLik,ix,x]=llbc(ECM,Y,Mode)
% Log-Likelihood based classifier
% [LLBC] = llbc(ECM);
% LLBC is a multiple discriminator
%
% [LL] = llbc(ECM,D);
% calculates the Log-Likelihood to each class
% the maximum log-likelihood and the corresponding class are obtained with
% [ll,C] = max(LL,[],2);
% or with 
% [ll,C] = llbc(ECM,D);
%
% ECM 	is the extended covariance matrix
% D     data
%
% LLBC  classifier
% ll	log-likelihood
% C     classification output
% 
% see also: DECOVM, ECOVM.M, R2.M, MDBC, LDBC

%	Version 1.23	Date: 30 Dec 2002
%	Copyright (c) 1999-2002 by Alois Schloegl <a.schloegl@ieee.org>	

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

NC=size(ECM);
if length(NC)<3, 
        if iscell(ECM(1)),
                NC=[max(NC(1:2)),size(ECM{1})];
        elseif NC(1)==NC(2)
                ECM{1}=ECM;
        end;
else
        %ECM = num2cell(ECM,[2,3]);
        for k = 1:NC(1),
                IR{k} = squeeze(ECM(k,:,:));
        end;
        ECM = IR;
end

if nargin>1,
        if NC(2) == size(Y,2)+1;
                Y = [ones(size(Y,1),1),Y];  % add 1-column
                warning('LLBC: 1-column added to data');
        end;
end;

for k = 1:NC(1);
        %[M,sd,S,xc,N] = decovm(ECM{k});  %decompose ECM
        c  = size(ECM{k},2);
        nn = ECM{k}(1,1);	% number of samples in training set for class k
        XC = ECM{k}/nn;		% normalize correlation matrix
        M  = XC(1,2:c);		% mean 
        S  = XC(2:c,2:c) - M'*M;% covariance matrix
        %M  = M/nn; S=S/(nn-1);
        x.IR{k} = [-M;eye(NC(2)-1)]*inv(S)*[-M',eye(NC(2)-1)];  % inverse correlation matrix extended by mean 
	d = c-1;
	x.logSF(k) = log(nn) - d/2*log(2*pi) - det(S)/2;
	x.logSF2(k)= log(nn) - d/2*log(2*pi) - log(det(S))/2;
	x.SF(k) = nn/sqrt((2*pi)^d * det(S));
end;

if 0, %nargin<2,
        LogLik = x;	% inverse correlation matrix
else
        LogLik=zeros(size(Y,1),NC(1)); %alllocate memory
        for k = 1:NC(1);  
                MDBC = sum((Y*x.IR{k}).*Y,2); % calculate distance of each data point to each class
    		LogLik(:,k) = x.logSF2(k) - MDBC/2;
	end;
        
        if nargout>=2,
                [tmp,ix] = max(LogLik,[],2);
		ix(isnan(tmp)) = NC(1)+1;	% not classified
        end;
end;
