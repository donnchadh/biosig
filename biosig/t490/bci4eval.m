function [o] = bci4eval(tsd,TRIG,cl,pre,post,Fs)
% BCI4eval evaluations a BCI-result as suggested in [1,2]. 
%   - It returns the classification error, the signal to noise ratio, 
%   the mutual information, as well as mean, standard error, 
%   within-class accuarcy and standard deviation for both classes. 
%   - time course of these resulting parameters are supported
%   - Missing values can be encoded as NaN.   
%
% X = bci4eval(tsd,trig,cl,pre,post,Fs)
%       tsd     continous output 
%       trig    trigger time points
%       cl      classlabels
%       pre     offset of trial start 
%       post    offset of trial end 
%       Fs      sampling rate (default Fs =128);
%
%       X is a struct with various results  
%
% see also: SUMSKIPNAN, PLOTA, BCI3EVAL
%
% REFERENCES: 
%  [1] Schlögl A., Neuper C. Pfurtscheller G.
%	Estimating the mutual information of an EEG-based Brain-Computer-Interface
%	Biomedizinische Technik 47(1-2): 3-8, 2002.
%  [2] A. Schlögl, C. Keinrath, R. Scherer, G. Pfurtscheller,
%	Information transfer of an EEG-based Bran-computer interface.
%	Proceedings of the 1st International IEEE EMBS Conference on Neural Engineering, pp.641-644, Mar 20-22, 2003. 
%  [3]  A. Schlögl, Evaluation of the dataset III of the BCI-competition 2003. 
%	http://ida.first.fraunhofer.de/projects/bci/competition/results/TR_BCI2003_III.pdf


%    $Revision: 1.1 $
%    $Id: bci4eval.m,v 1.1 2004-02-13 17:01:21 schloegl Exp $
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

%	$Revision: 1.1 $
%	$Id: bci4eval.m,v 1.1 2004-02-13 17:01:21 schloegl Exp $
%	Copyright (C) 1997-2004 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

if nargin<6
        Fs = 128;
end;

DIM = 2; 
CL = unique(cl);
for k = 1:length(CL),
        [x,sz]=trigg(tsd(:),TRIG(cl==CL(k)),pre,post);
        X{k} = squeeze(reshape(x,sz));
end;

% classification error 
o.ERR = (1-mean(sign([-X{1};X{2}]),DIM))/2;

% within-class accuracy
o.BCG1 = (1 + mean(sign(-X{1}), DIM))/2;
o.BCG2 = (1 + mean(sign( X{2}), DIM))/2;
o.T    = (1 : length(o.BCG1))'/Fs;
o.Fs   = Fs; 

%%%%% 2nd order statistics
[i1.SUM, o.N1, i1.SSQ] = sumskipnan(X{1},DIM);       
[i2.SUM, o.N2, i2.SSQ] = sumskipnan(X{2},DIM);       

o.MEAN1 = i1.SUM./o.N1;	% mean
v1    = i1.SSQ-i1.SUM.*o.MEAN1;	% n*var
o.SD1 = sqrt(v1./o.N1); % standard deviation 
%o.SE1 = sqrt(v1)./o.N1; % standard error of the mean 

o.MEAN2 = i2.SUM./o.N2;
v2    = i2.SSQ-i2.SUM.*o.MEAN2;
o.SD2 = sqrt(v2./o.N2);
%o.SE2 = sqrt(v2)./o.N2;


%%%%% Signal-to-Noise Ratio 

% intra-class variability
vd = var([-X{1},X{2}],[],DIM);        

o.SNR = 1/4*(o.MEAN2-o.MEAN1).^2./vd; 

%%%%% Mutual Information 
o.I   = 1/2*log2(o.SNR+1);

o.datatype = 'TSD_BCI7';  % useful for PLOTA
