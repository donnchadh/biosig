function [LDBC,ix]=ldbc(ECM,Y)
% Linear discriminant based classifier
% [LDBC] = ldbc(ECM);
% LDBC is a multiple discriminator
%
% [LMD] = ldbc(ECM,D);
% calculates the MD to each class
% the minimum distance and the corresponding class are obtained with
% [ld,C] = min(MD,[],2);
% or with 
% [ld,C] = ldbc(ECM,D);
%
% ECM 	is the extended covariance matrix
% D     data
%
% LDBC  classifier
% LD    mahalanobis distance
% C     classification output
% 
% see also: DECOVM, ECOVM.M, R2.M, MDBC

%	Copyright (C) 1999-2002 by Alois Schloegl
%	a.schloegl@ieee.org	
%	22.11.2000 Version 1.21

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
                fprintf(2,'Warning LDBC: 1-column added to data \n');
        end;
end;

C0 = zeros(NC(2));
for k = 1:NC(1);
        %[M,sd,S,xc,N] = decovm(ECM{k});  %decompose ECM
        c  = size(ECM{k},2);
        nn = ECM{k}(1,1);	% number of samples in training set for class k
        XC = ECM{k}/nn;		% normalize correlation matrix
        M  = XC(1,2:c);		% mean 
        S  = XC(2:c,2:c) - M'*M;% covariance matrix
        C0 = C0 + ECM{k};
        m{k}=M;
        %M  = M/nn; S=S/(nn-1);
        IR{k} = [-M;eye(NC(2)-1)]*inv(S)*[-M',eye(NC(2)-1)];  % inverse correlation matrix extended by mean 
end;
[M0,sd,COV0,xc,N,R2] = decovm(C0);

K=1;
for k = 1:NC(1);
for l = k+1:NC(1);
	w     = COV0\(m{l}'-m{k}');
	w0    = -M0*w;
        LDC(:,K) = [w0; w];
        K=K+1;
end;
end;

if nargin<2,
        LDBC = LDC;	% inverse correlation matrix
else
        LDBC=zeros(size(Y,1),size(LDC,2)); %allocate memory
        %for k = 1:size(LDC,2);  
                %MDBC(:,k) = sqrt(sum((Y*IR{k}).*Y,2)); % calculate distance of each data point to each class
        %end;
        LDBC = Y*LDC;
        
        %if nargout>1,
        %        [LDBC,ix] = min(LDBC,[],2);
        %end;
end;
