function [MDBC,ix]=mdbc(ECM,Y)
% Mahalanobis distance based classifier
% [MDBC] = mdbc(ECM);
% MDBC is a multiple discriminator
%
% [MD] = mdbc(ECM,D);
% calculates the MD to each class
% the minimum distance and the corresponding class are obtained with
% [md,C] = min(MD,[],2);
% or with 
% [md,C] = mdbc(ECM,D);
%
% ECM 	is the extended covariance matrix
% D     data
%
% MDBC  classifier
% MD    mahalanobis distance
% C     classification output
% 
% see also: DECOVM, ECOVM.M, R2.M
%
% REFERENCE(S):
% P. C. Mahalanobis, Proc. Natl. Institute of Science of India, 2, 49, (1936)

%	$Revision: 1.3 $
%	$Id: mdbc.m,v 1.3 2003-11-06 16:36:11 schloegl Exp $
%	Copyright (c) 1999-2003 by Alois Schloegl <a.schloegl@ieee.org>	

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
        if (NC(2) == size(Y,2)+1) 
                Y = [ones(size(Y,1),1),Y];  % add 1-column
                warning('MDBC: 1-column added to data');
        elseif ~all(Y(:,1)==1 | isnan(Y(:,1)))
                warning('first column does not contain ones only') 
        end;
end;

if 0,	% after a discussion with Carmen Vidaurre, we found out together, 
        % that the following procedure is equivalent to the one below. 
        % Therefore, the simplier version is used as default. 
        for k = 1:NC(1);
                %[M,sd,S,xc,N] = decovm(ECM{k});  %decompose ECM
                c  = size(ECM{k},2);
                nn = ECM{k}(1,1);	% number of samples in training set for class k
                XC = ECM{k}/nn;		% normalize correlation matrix
                M  = XC(1,2:c);		% mean 
                S  = XC(2:c,2:c) - M'*M;% covariance matrix
                %M  = M/nn; S=S/(nn-1);
                IR{k} = [-M;eye(NC(2)-1)]*inv(S)*[-M',eye(NC(2)-1)];  % inverse correlation matrix extended by mean 
                %x.logSF{k} = -(nn*log(2*pi) + det(S))/2;
                %x.logSF2{k} = -(nn*log(2*pi) + log(det(S)))/2;
                %x.SF{k} = sqrt((2*pi)^nn*det(S));
        end;
else
        for k = 1:NC(1);
                IR{k} = inv(ECM{k}/ECM{k}(1,1));
                IR{k}(1,1) = IR{k}(1,1) - 1 ;
        end;
end;

if nargin<2,
        MDBC = IR;	% inverse correlation matrix
else
        MDBC = zeros(size(Y,1),NC(1)); %allocate memory
        for k = 1:NC(1);  
                MDBC(:,k) = sum((Y*IR{k}).*Y,2); % calculate distance of each data point to each class
	end;
        
	MDBC = sqrt(MDBC);
        if nargout==2,
                [MDBC,ix] = min(MDBC,[],2);
		ix(isnan(MDBC)) = NC(1)+1;	% not classified
        end;
end;
