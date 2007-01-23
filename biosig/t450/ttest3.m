function t=ttest3(X,n1)
%
% The function t=ttest3(X,n1) is a subfunction in fdr.m.
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


[n,k]=size(X);
n2=n-n1;
X1=X(1:n1,:);
X2=X(n1+1:n,:);
z1=sum(X1,1)/n1;
z2=sum(X2,1)/n2;
s1=X1-ones(n1,1)*sum(X1,1)/n1;s1=sum(s1.*s1,1)/(n1-1);
s2=X2-ones(n2,1)*sum(X2,1)/n2;s2=sum(s2.*s2,1)/(n2-1);
s=sqrt((n1-1)*s1+(n2-1)*s2);
t=(z1-z2).*(ones(1,k)*sqrt(n1*n2*(n-2)/n)./s);
