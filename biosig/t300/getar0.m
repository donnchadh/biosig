function [a0,A0] = getar0(S,P,NTR,NN);
% GETAR0 calculates average AR parameters for initialization of 
% AAR estimation
% [a0,A0] = getar0(S,P,NTR,NN);
%    S  signal 
%    P  list of model orders
%    NTR  number of realizations
%    NN   length each segment
%    a0 average AR-parameters	
%    A0 covariance 
%

%	$Revision: 1.1 $
%	$Id: getar0.m,v 1.1 2003-06-17 09:15:59 schloegl Exp $
%	Copyright (c) 1996-2003 by Alois Schloegl
%	e-mail: a.schloegl@ieee.org	


S  = S(:);
tmp= floor(rand(1,NTR)*(length(S)-NN));
z0 = zeros(NTR,NN);
for k = 1:length(tmp),
        z0(k,:) = S(tmp(k) + (1:NN))';
end;
[MX,pe] = durlev(acovf(z0,max(P)));

for k=1:length(P),
        a = MX(:,P(k)*(P(k)-1)/2+(1:P(k)));
        a0{k} = mean(a);
        A0{k} = covm(a,'D');
end;	

