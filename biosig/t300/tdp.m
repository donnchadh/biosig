function F = tdp(S,p,UC,A)
%  TDP time-domain parameters
%	extracts EEG features very fast (low computational efforts) 
%     	This is function was motivated by the Hjorth parameters and is experimental. 
%	It might a useful EEG feature for BCI classification.  
%           
%  F = TDP(...)
%  
%  [...] = tdp(S,p,0)
%       calculates the log-power of the first p-th derivatives 
%  [...] = tdp(S,p,UC) with 0<UC<1,
%       calculates time-varying power of the first p-th derivatives 
%       using an exponential window 
%  [...] = tdp(S,p,N) with N>1,
%       calculates time-varying log-power of the first p-th derivatives %       using rectangulare window of length N
%  [...] = tdp(S,p,B,A) with B>=1 oder length(B)>1,
%       calculates time-varying log-power of the first p-th derivatives %       using the transfer function B(z)/A(z) for windowing 
%  Input:
%       S       data (each channel is a column)
%	p	number of features
%       UC      update coefficient 
%       B,A     filter coefficients (window function) 
%  Output:
%       F	log-power of each channel and each derivative
%       
% see also: HJORTH, BARLOW, WACKERMANN
%
% REFERENCE(S):

% 	$Id: tdp.m,v 1.1 2008-03-13 09:14:10 schloegl Exp $
% 	Copyright (C) 2008 by Alois Schloegl <a.schloegl@ieee.org>
% 	This is part of the BIOSIG-toolbox http://biosig.sf.net/

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 3
% of the License, or (at your option) any later version.


[N,K] = size(S); 	% number of electrodes K, number of samples N

d0 = S;

FLAG_ReplaceNaN = 1;

if nargin<3, 
        UC = 0; 
end;
if nargin<4,
        if UC==0,
                                
        elseif UC>=1,
                B = ones(1,UC);
                A = UC;
        elseif UC<1,
                FLAG_ReplaceNaN = 1;
                B = UC; 
                A = [1, UC-1];
        end;
else
        B = UC;    
end;

if ~UC,
	d = S;
	F = log(mean(d.^2));
	for k = 1:p,
		d = diff([repmat(NaN,[1,K]);d],[],1);
	        F = [F, log(mean(d.^2))];
        end;
else
       	d = d0;
        if FLAG_ReplaceNaN;
                d(isnan(d)) = 0;
        end;
        F = log(filter(B,A,d.^2)./filter(B,A,(~isnan(d0)).^2));
	for k = 1:p,
		d0 = diff([repmat(NaN,[1,K]);d0],[],1);
		d  = d0;
	        if FLAG_ReplaceNaN;
                	d(isnan(d)) = 0;
        	end;
	        m = filter(B,A,d.^2)./filter(B,A,(~isnan(d0)).^2);
	        F = [F, log(m)];
        end;
end;

