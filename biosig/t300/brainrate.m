function [br,sef90,sef95] = brainrate(s,Fs,UC,A)
%  BRAINRATE estimates single coefficient parameters according to [1] 
%           
%  [BRAINRATE, SEF90, SEF95] = brainrate(...)
%  
%  [...] = brainrate(S,Fs,0)
%       calculates stationary brainrate parameter 
%  [...] = hjorth(S,Fs,UC) with 0<UC<1,
%       calculates time-varying brainrate parameter using 
%       exponential window 
%  [...] = hjorth(S,Fs,N) with N>1,
%       calculates time-varying brainrate parameter using 
%       rectangulare window of length N
%  [...] = hjorth(S,Fs,B,A) with B>=1 oder length(B)>1,
%       calulates time-varying brainrate parameters using
%       transfer function B(z)/A(z) for windowing
%
%       S       data (each channel is a column)
%       UC      update coefficient 
%       B,A     filter coefficients (window function)
%            
%
% REFERENCE(S):
% [1] Nada Pop-Jordanova and Jordan Pop-Jordanov
%     Spectrum-weighted EEG frequency ("Brainrate") as a quantitative 
%     indicator of arousal 

%	$Id: brainrate.m,v 1.1 2006-09-25 16:55:36 schloegl Exp $
%	Copyright (C) 2006 by Alois Schloegl <a.schloegl@ieee.org>
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

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


[N,K] = size(s); 	% number of electrodes K, number of samples N
MOP = 15; 		% order of autoregressive model 

if nargin<2, 
        UC = 0; 
end;
if nargin<3;
        if UC==0,
		br = size(1,K); 
		sef90 = size(1,K); 
		sef95 = size(1,K); 
        elseif UC>=1,
                B = ones(1,UC);
                A = UC;
		br = size(N,K); 
		sef90 = size(N,K); 
		sef95 = size(N,K); 
        elseif UC<1,
                FLAG_ReplaceNaN = 1;
                B = UC; 
                A = [1, UC-1];
		br = size(N,K); 
		sef90 = size(N,K); 
		sef95 = size(N,K); 
        end;
else
        B = UC;    
end;

s = center(s,1);
for k2=1:K,
	if ~UC,
		a = lattice(s(:,k2)',MOP); 	
	else
		X = tvaar(s(:,k2)',MOP,UC); 
		X = tvaar(s(:,k2)',X);
		a = X.AAR;  
	end;

	for k1 = 1:size(a,1),
		f = .1:.1:20; 
		h = freqz(1,[1,-a(k1,:)],f,Fs); 
		h2= abs(h).^2; 
		br(k1,k2) = sum(f.*h2)/sum(h2); 
		sef90(k1,k2) = f(min(find(h2>=.90*h2(end)))); 
		sef95(k1,k2) = f(min(find(h2>=.95*h2(end)))); 
	end; 

	if 0,nargout>1,
	for k1 = 1:size(a,1),
		f = (0:256)/512*Fs; 
		h = freqz(1,[1,-a(k1,:)],f,Fs); 
		h2= cumsum(abs(h2).^2); 
		sef90(k1,k2) = f(min(find(h2>=.90*h2(end)))); 
		sef95(k1,k2) = f(min(find(h2>=.95*h2(end)))); 
	end; 
	end; 
end;
