function [LDBC,ix]=ldbc(ECM,Y)
% Linear discriminant based classifier
% [LDBC] = ldbc3(ECM);
% LDBC is a multiple discriminator
%
% The difference to LDBC.M is, that LDBC2.M can be used with 
% more than two classes. The within-class covariance matrices, 
% are used, whereas LDBC3.M uses the overall covariance matrix.
%
% [LD] = ldbc3(ECM,D);
%  calculates the LD to each class
% [LD,ix] = ldbc3(ECM,D);
%  ix is the selected class
%
% ECM 	is the extended covariance matrix
% D     data
%
% LDBC  classifier
% LD    mahalanobis distance
% C     classification output
% 
% see also: DECOVM, R2, MDBC, LDBC, LDBC2

%	$Revision: 1.1 $
%	$Id: ldbc3.m,v 1.1 2005-02-08 14:47:27 schloegl Exp $
%	Copyright (C) 1999-2004 by Alois Schloegl <a.schloegl@ieee.org>	
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
                fprintf(2,'Warning LDBC: 1-column added to data \n');
        elseif ~all(Y(:,1)==1 | isnan(Y(:,1)))
                warning('first column does not contain ones only') 
        end;
end;

ECM0 = squeeze(sum(ECM,1));  %decompose ECM
[M0,sd,COV0,xc,N,R2] = decovm(ECM0);

for k = 1:NC(1);
	ecm = squeeze(ECM(k,:,:));
        [M1,sd,COV1,xc,N,R2] = decovm(ECM0-ecm);
        [M2,sd,COV2,xc,N,R2] = decovm(ecm);
        w     = COV0\(M2'-M1');
        w0    = -M0*w;
        LDC(:,k) = [w0; w];
end;

if nargin<2,
        LDBC = LDC;
else
        LDBC = Y * LDC;
	if nargout>1,
		tmp = all(isnan(LDBC,2));
        	[LDBC,ix] = max(LDBC,[],2);
		ix(tmp) = NaN; 
	end;    
end;
