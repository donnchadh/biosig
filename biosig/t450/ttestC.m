function [t]=ttestC(z);
%
% The function [t] = ttestC(zis a subfunction in fdr.m.
% For help use the main-function fdr.m.
%
% Copyright (C) 2006 Claudia Hemmelmann <c.hemmelmann@imsid.uni-jena.de>
% Adapted by A Schloegl <a.schloegl@ieee.org> Dec 2006 
%
%***
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
%
%--------------------------------------------------------------------------


[n,k]=size(z);
zquer1=sum(z,1)/n;
s1=sqrt(sum((z-ones(n,1)*zquer1).^2,1)/(n-1));
t=sqrt(n)*zquer1./s1;
