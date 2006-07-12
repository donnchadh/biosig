function [LDBC,ix]=ldbc(ECM,Y)
% Linear discriminant based classifier
% [LDBC] = ldbc(ECM);
% LDBC is a multiple discriminator
%
% The difference to LDBC.M is, that LDBC1.M uses the within-class
% covariance matrices, whereas LDBC.M uses the overall covariance matrix.
%
% [LD] = ldbc(ECM,D);
% calculates the LD to each class
%
% ECM 	is the extended covariance matrix
% D     data
%
% LDBC  classifier
% LD    mahalanobis distance
% C     classification output
% 
% see also: LDBC1, DECOVM, R2, MDBC

%	$Revision: 1.4 $
%	$Id: ldbc.m,v 1.4 2006-07-12 19:43:09 schloegl Exp $
%	Copyright (C) 1999-2003 by Alois Schloegl <a.schloegl@ieee.org>	
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
        elseif ~all(Y(:,1)==1 | isnan(Y(:,1)))
                warning('first column does not contain ones only') 
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
cl = [];
for k = 1:NC(1);
for l = k+1:NC(1);
        cl(K,:) = [k,l];
	w     = COV0\(m{l}'-m{k}');
	w0    = -M0*w;
        LDC(:,K) = [w0; w];
        K=K+1;
end;
end;
for k = 1:NC(1);
        W(:,k)=(cl(:,2)==k)-(cl(:,1)==k);
end;

if nargin<2,
        LDBC = LDC;	% inverse correlation matrix
else
        if nargout>1,
                [LDBC,ix] = max(Y*LDC,[],2);
        else
                LDBC = Y*LDC;
        end;
end;
