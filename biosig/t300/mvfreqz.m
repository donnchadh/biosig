function [h,S,COH2,DTF,H]=mvfreqz(B,A,C,N,Fs)
% MVFREQZ multivariate frequency responce
% [h,SP,COH,DTF,H]=mvfreqz(B,A,C,N,Fs)
%
% SP  power spectrum
% COH coherence
% DTF directed transfer fucntion	
%
% see also: FREQZ, 
% 
% REFERENCE(S):
% H. Liang et al. Neurocomputing, 32-33, pp.891-896, 2000. 
% L.A. Baccala and K. Samashima, Biol. Cybern. 84,463-474, (2001). 

%	$Revision: 1.1 $
%	$Id: mvfreqz.m,v 1.1 2003-02-07 13:25:45 schloegl Exp $
%	Copyright (C) 1996-2003 by Alois Schloegl <a.schloegl@ieee.org>	

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


[K1,K2] = size(A);
p = K2/K1-1;
%a=ones(1,p+1);
[K1,K2] = size(B);
q = K2/K1-1;
%b=ones(1,q+1);

if nargin<3
        C=ones(K1,K1);
end;
if nargin<4
        N=512;
end;
if nargin<5
	Fs=pi*2;        
end;
if all(size(N)==1)	
        f = (1:N)/N/2*Fs;
else
        f = N;
        N = length(N);
end;
s = exp(i*2*pi*f/Fs);
z = i*2*pi/Fs;

h=zeros(K1,K1,N);
SP=zeros(K1,K1,N);
DTF=zeros(K1,K1,N);
COH=zeros(K1,K1,N);
COH2=zeros(K1,K1,N);
PDC=zeros(K1,K1,N);
PDCF=zeros(K1,K1,N);
invC=inv(C);
tmp1=zeros(1,K1);
tmp2=zeros(1,K1);
if 0;
        for k1=1:K1;
                for k2=1:K2;
                        for k = 1:p+1,
                                atmp(k) = A{k}(k1,k2);
                        end;        
                        a{k1,k2} = atmp(1:p+1);
                        for k = 1:q+1,
                                btmp(k) = B{k}(k1,k2);
                        end;       
                        b{k1,k2} = btmp(1:q+1);
                        h(k1,k2,:) = polyval(btmp,s)./polyval(atmp,s);        
                end;
        end;
        for n=1:N,
                S(:,:,n) = h(:,:,n)*C*h(:,:,n)';        
        end;
else
        
        for n=1:N,
                atmp = zeros(K1,K1);
                for k = 1:p+1,
                        atmp = atmp + A(:,k*K1+(1-K1:0))*exp(z*(k-1)*f(n));
                end;        
                btmp = zeros(K1,K2);
                for k = 1:q+1,
                        btmp = btmp + B(:,k*K1+(1-K1:0))*exp(z*(k-1)*f(n));
                end;        
                h(:,:,n) = atmp\btmp;        
                S(:,:,n) = h(:,:,n)*C*h(:,:,n)';        
                
                
                for k1 = 1:K1,
                        tmp = squeeze(atmp(:,k1,n));
	                tmp1(k1) = sqrt(tmp'*tmp);
	                tmp2(k1) = sqrt(tmp'*invC*tmp);
                end;
                PDCF(:,:,n,kk) = abs(atmp)./tmp2(ones(1,K1),:);
                PDC(:,:,n,kk)  = abs(atmp)./tmp1(ones(1,K1),:);
        end;
end;

DC = zeros(K1);
for k = 1:p,
        DC = DC + A(:,k*K1+(1:K1)).^2;
end;        

%%%%% directed transfer function
for k1=1:K1;
        DEN=(sum(abs(h(k1,:,:)).^2,2));	        
for k2=1:K2;
        COH2(k1,k2,:) = abs(S(k1,k2,:).^2)./(abs(S(k1,k1,:).*S(k2,k2,:)));
	DTF(k1,k2,:) = abs(h(k1,k2,:).^2)./DEN;	        
end;
end;

