function [INI,S,E] = detectmuscle(S, iter)
% Muscle detection with inverse filtering
% Artifacts are indicated with NaN's. 
%
% [INI,S,E] = detectmuscle(S [,iter])
%
% iter		number of iterations [default:1]
% INI.MU	mean of S
% INI.InvFilter	coefficients of inverse filter 
% S		outlier replaced by NaN
% E		innan(E) indicates muscle artifact


% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

%	$Revision: 1.1 $
%	$Id: detectmuscle.m,v 1.1 2003-07-17 17:32:16 schloegl Exp $
%	Copyright (C) 2003 by Alois Schloegl <a.schloegl@ieee.org>	


if nargin<2,
        iter=1;
end;

% Muscle Detection 
TH = 5; 
[se,INI.MU] = sem(S);
if TH*se<abs(INI.MU),
	[S] = center(S);
end;

while iter>0,
        [AR,RC,PE]= lattice(S',10);
	INI.InvFilter = ar2poly(AR);
        E = zeros(size(S));
        for k = 1:size(S,2),
                E(:,k) = filter(INI.InvFilter(k,:),1,S(:,k));
        end;
        INI.V  = std(E);
        INI.TH = INI.V * TH; 
        iter   = iter-1;
        
        for k = 1:size(S,2),
                S(E(:,k)>(INI.TH(k)) | E(:,k)<(-INI.TH(k)),k) = NaN;
        end;
end; 

if nargout<3, return; end;

% the following part demonstrates a possible correction 
for k = 1:size(S,2),
                E(:,k) = filter(INI.InvFilter(k,:),1,S(:,k));
end;
% isnan(E), returns Artifacts