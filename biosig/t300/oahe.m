function [Y,Z] = OAHE(X,Fs)
% Obstructive Apnea/Hypopnea event
%
% Data processing in the BIOSIG toolbox, 
% This functions can be used as template as well as a wrapper 
% around different signal processing methods.  
%


%	$Revision: 1.1 $
%	$Id: oahe.m,v 1.1 2003-10-12 22:12:42 schloegl Exp $
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

X = [X; repmat(nan,  nseg*120*Fs-nr, nc)];
X = reshape(X,5*Fs,24*nseg);

hi_baseline = max(X);
lo_baseline = min(X);

blh = median(hi_baseline(hankel(1:nseg2-25,nseg2-25:nseg2-2))');
bll = median(lo_baseline(hankel(1:nseg2-25,nseg2-25:nseg2-2))');

mh = max(hi_baseline(hankel(25:nseg2-1,nseg2+(-1:0)))');
ml = min(lo_baseline(hankel(25:nseg2-1,nseg2+(-1:0)))');

Y = (blh-bll)'>2*(mh-ml)';





