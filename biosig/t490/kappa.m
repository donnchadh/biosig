function [kap,se,H,z,p0,SA,R]=kappa(d,c,kk);
% KAPPA estimates Cohen's kappa coefficient
%   and related statistics 
%
% [kap,sd,H,z,ACC,sACC,MI] = kappa(d1,d2);
% [kap,sd,H,z,ACC,sACC,MI] = kappa(H);
% X = kappa(...);
%
% d1    data of scorer 1 
% d2    data of scorer 2 
%
% kap	Cohen's kappa coefficient point
% se	standard error of the kappa estimate
% H	Concordance matrix, i.e. confusion matrix
% z	z-score
% ACC	overall agreement (accuracy) 
% sACC	specific accuracy 
% MI 	Mutual information or transfer information (in [bits])
% X 	is a struct containing all the fields above
%
% Reference(s):
% [1] Cohen, J. (1960). A coefficient of agreement for nominal scales. Educational and Psychological Measurement, 20, 37-46.
% [2] J Bortz, GA Lienert (1998) Kurzgefasste Statistik f|r die klassische Forschung, Springer Berlin - Heidelberg. 
%        Kapitel 6: Uebereinstimmungsmasze fuer subjektive Merkmalsurteile. p. 265-270.
% [3] http://www.cmis.csiro.au/Fiona.Evans/personal/msc/html/chapter3.html
% [4] Kraemer, H. C. (1982). Kappa coefficient. In S. Kotz and N. L. Johnson (Eds.), 
%        Encyclopedia of Statistical Sciences. New York: John Wiley & Sons.
% [5] http://ourworld.compuserve.com/homepages/jsuebersax/kappa.htm
%
%  

%	$Revision: 1.8 $
%	$Id: kappa.m,v 1.8 2007-12-03 19:19:49 schloegl Exp $
%	Copyright (c) 1997-2006 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% version 2 of the License, or (at your option) any later version.
%
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
%
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the
% Free Software Foundation, Inc., 59 Temple Place - Suite 330,
% Boston, MA  02111-1307, USA.
%

if nargin>1,
        [dr,dc] = size(d);
    	[cr,cc] = size(c);

	[X.Label,i,j]=unique([d(:);c(:)]); 
	c = j(1+numel(d):end); 
	d = j(1:numel(d)); 
	

    	N  = min(cr,dr); % number of examples
    	N  = sum(isfinite(d) & isfinite(c));
    	ku = max([d;c]); % upper range
    	kl = min([d;c]); % lower range
    
	
    	if (nargin<3),
            	kk = length(X.Label);  	% maximum element
    	else
            	if kk<ku;  	% maximum element
                    	fprintf(2,'Error KAPPA: some element is larger than arg3(%i)\n',kk);
            	end;
    	end;
    
    	if 0,
        	h = histo([d+c*kk; kk*kk+1; 1]); 
        	H = reshape(h(1:length(h)-1));
        	H(1,1) = H(1,1)-1;
    	else
		if 1;   % exist('OCTAVE_VERSION')>=5;
	        	H = zeros(kk);
    			for k = 1:N, 
    				if ~isnan(d(k)) & ~isnan(c(k)),
		    			H(d(k),c(k)) = H(d(k),c(k)) + 1;
		    		end;	
        		end;
		else
			H = full(sparse(d(1:N),c(1:N),1,kk,kk));
    		end;
	end;
else
	X.Label = 1:min(size(d));
    	H = d(X.Label,X.Label);
    	% if size(H,1)==size(H,2);	
	N = sum(sum(H));
    	% end;
end;

warning('off');
p0  = sum(diag(H))/N;  %accuracy of observed agreement, overall agreement 
%OA = sum(diag(H))/N);

p_i = sum(H); %sum(H,1);
pi_ = sum(H'); %sum(H,2)';

SA  = 2*diag(H)'./(p_i+pi_); % specific agreement 

pe  = (p_i*pi_')/(N*N);  % estimate of change agreement

px  = sum(p_i.*pi_.*(p_i+pi_))/(N*N*N);

%standard error 
kap = (p0-pe)/(1-pe);
sd  = sqrt((pe+pe*pe-px)/(N*(1-pe*pe)));

%standard error 
se  = sqrt((p0+pe*pe-px)/N)/(1-pe);
z = kap/se;

if ((1 < nargout) & (nargout<7)) return; end; 

% Nykopp's entropy
pwi = sum(H,2)/N;                       % p(x_i)
pwj = sum(H,1)/N;                       % p(y_j)
pji = H./repmat(sum(H,2),1,size(H,2));  % p(y_j | x_i) 
pwj(pwj==0) = 1;                        % make sure p*log2(p) is 0, this avoids NaN's 
pji(pji==0) = 1;                        % make sure p*log2(p) is 0, this avoids NaN's 
R   = - sum(pwj.*log2(pwj)) + sum(pwi'*(pji.*log2(pji)));

if (nargout>1) return; end; 

X.kappa = kap; 
X.kappa_se = se; 
X.H = H;
X.z = z; 
X.ACC = p0; 
X.sACC = SA;
X.MI = R;
kap = X;  
