function [Y,Z] = OAHE(X,Fs)
% Obstructive Apnea/Hypopnea event
%
% Data processing in the BIOSIG toolbox, 
% This functions can be used as template as well as a wrapper 
% around different signal processing methods.  
%


%	$Revision: 1.2 $
%	$Id: oahe.m,v 1.2 2004-06-09 18:21:03 schloegl Exp $
%	Copyright (C) 2003 by Alois Schloegl <a.schloegl@ieee.org>	

% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% Version 2 of the License, or (at your option) any later version.
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



[nr,nc]=size(X);
 
nseg  = ceil (nr/(120*Fs));
nseg2 = floor(nr/(  5*Fs));

X = [repmat(nan,125*Fs,nc); X; repmat(nan, nseg*120*Fs-nr, nc)];
X = reshape(X,5*Fs,24*nseg+25);

hi_baseline = max(X);
lo_baseline = min(X);

ix = hankel(1:nseg2,nseg2:nseg2+25)';

blh = median(hi_baseline(ix(1:24,:)));
bll = median(lo_baseline(ix(1:24,:)));

mh  = max(hi_baseline(ix(25:26,:)));
ml  = min(lo_baseline(ix(25:26,:)));

Y   = real((blh-bll)' > 2*(mh-ml)');
Y(any(isnan([mh;ml;blh;bll]))) = NaN;




