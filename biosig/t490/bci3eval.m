function [o] = bci3eval(x1,x2,DIM)
% BCI3eval evaluations a BCI-result as suggested in [1,2]. 
%   - It returns the classification error, the signal to noise ratio, 
%   the mutual information, as well as mean, standard error and 
%   standard deviation for both classes. 
%   - time course of these resulting parameters are supported
%   - Missing values can be encoded as NaN.   
% 
%
% [o] = bci3eval(x1, x2 [,DIM])
%
% x1 is the bci output for class 1 
% x2 is the bci output for class 2
% o is a struct with various results  
%
%
% see also: SUMSKIPNAN, PLOTA
%
% REFERENCES: 
%  [1] Schlögl A., Neuper C. Pfurtscheller G.
%	Estimating the mutual information of an EEG-based Brain-Computer-Interface
%	Biomedizinische Technik 47(1-2): 3-8, 2002.
%  [2] A. Schlögl, C. Keinrath, R. Scherer, G. Pfurtscheller,
%	Information transfer of an EEG-based Bran-computer interface.
%	Proceedings of the 1st International IEEE EMBS Conference on Neural Engineering, pp.641-644, Mar 20-22, 2003. 
%  [3] Evaluation of the dataset III of the BCI-competition 2003. 
%	http://ida.first.gmd.de/~blanker/competition/results/TR_BCI2003_III.pdf
%  [4] BIOSIG.SF.NET

%    $Revision: 1.1 $
%    $Id: bci3eval.m,v 1.1 2003-05-21 13:13:29 schloegl Exp $
%    Copyright (C) 2003 by Alois Schloegl <a.schloegl@ieee.org>	

%    This program is free software; you can redistribute it and/or modify
%    it under the terms of the GNU General Public License as published by
%    the Free Software Foundation; either version 2 of the License, or
%    (at your option) any later version.
%
%    This program is distributed in the hope that it will be useful,
%    but WITHOUT ANY WARRANTY; without even the implied warranty of
%    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%    GNU General Public License for more details.
%
%    You should have received a copy of the GNU General Public License
%    along with this program; if not, write to the Free Software
%    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

%    Copyright (C) 2000-2003 by  Alois Schloegl <a.schloegl@ieee.org>	


if nargin<3,
        DIM=min(find(size(i)>1));
        if isempty(DIM), DIM=1; end;
end;


% classification error 
if DIM==1,
	o.ERR = (1-mean(sign([-x1;x2]),DIM))/2;
else
	o.ERR = (1-mean(sign([-x1,x2]),DIM))/2;
end;



%%%%% 2nd order statistics
[i1.SUM,i1.N,i1.SSQ] = sumskipnan(x1,DIM);       
[i2.SUM,i2.N,i2.SSQ] = sumskipnan(x2,DIM);       

o.MEAN1 = i1.SUM./i1.N;	% mean
v1    = i1.SSQ-i1.SUM.*o.MEAN1;	% n*var
o.SD1 = sqrt(v1./i1.N); % standard deviation 
o.SE1 = sqrt(v1)./i1.N; % standard error of the mean 

o.MEAN2 = i2.SUM./i2.N;
v2    = i2.SSQ-i2.SUM.*o.MEAN2;
o.SD2 = sqrt(v2./i2.N);
o.SE2 = sqrt(v2)./i2.N;


%%%%% Signal-to-Noise Ratio and Mutual Information 

% This formula was used for the first evaluation in [3]. It was corrected on May 21st, 2003. 
% o.SNR = 2*(o.MEAN2-o.MEAN1).^2./(v1./i1.N + v2./i1.N);  

% This is the correct formula in [2]
o.SNR1 = 2/4*(o.MEAN2-o.MEAN1).^2./(v1./i1.N + v2./i2.N);

% This are equivalent definitions
if DIM==1,
	v0 = var([x1;x2],DIM);        
	vd = var([-x1;x2],DIM);        
else
	v0 = var([x1,x2],DIM);        
	vd = var([-x1,x2],DIM);        
end;

o.SNR2 = v0./vd-1;   
o.SNR3 = 2*v0./(var(x1,DIM)+var(x2,DIM))-1;  % [1]
o.SNR4 = 1/4*(o.MEAN2-o.MEAN1).^2./vd; 
% Small differences (within the order of 1/N) can be observed. 
% Hence, for large N the differences can be neglected. 

o.SNR = o.SNR4;  %With this method, the SNR is always positive and unbiased estimation of intra-class variability


o.I   = 1/2*log2(o.SNR+1);

o.datatype = 'SNR';

