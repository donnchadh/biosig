function [o] = bci4eval(tsd,TRIG,cl,pre,post,Fs)
% BCI4eval evaluates a BCI-result for two and more classes
%
%   Two classes are evaluated like in [1,2]:
%   - It returns the classification error, the signal to noise ratio, 
%   the mutual information, as well as mean, standard error, 
%   within-class accuracy and standard deviation for both classes. 
%   - time course of these resulting parameters are supported
%
%   More than two classes are evaluated with 
%   - Kappa coefficient including standard deviation 
%   - Accuracy
%   
%   Missing values can be encoded as NaN.
%
% X = bci4eval(tsd,trig,cl,pre,post,Fs)
%       tsd     continous output 
%               for 2 classes, tsd must have size Nx1 
%               size NxM for M-classes, for each row the largest value 
%               determines the assigned class 
%       trig    trigger time points
%       cl      classlabels
%       pre     offset of trial start 
%       post    offset of trial end 
%       Fs      sampling rate;
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


%    $Revision: 1.4 $
%    $Id: bci4eval.m,v 1.4 2005-06-04 22:07:44 schloegl Exp $
%    Copyright (C) 2003 by Alois Schloegl <a.schloegl@ieee.org>	
%    This is part of the BIOSIG-toolbox http://biosig.sf.net/

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

if nargin<6
        Fs = 1;
end;

DIM = 2; 
CL = unique(cl);


if any([1,length(CL)]==size(tsd,2))
        [x,sz] = trigg(tsd,TRIG,pre,post);
        D = reshape(x,sz);
        D = squeeze(D);
else
        if size(tsd,1)==length(cl)
                D = tsd';
        elseif size(tsd,2)==length(cl)
                D = tsd;
        else
                error('BCI4EVAL: size of data and size of Classlabels does not fit');
        end;
        sz = [1,size(D)];
        pre=1;
        post=size(D,1);
end;
if size(D,(length(CL)==size(tsd,2))+2) ~= length(cl),
        size(D,(length(CL)==size(tsd,2))+2),
        [size(D), size(cl),size(CL),size(tsd)],
        error('BCI4EVAL: length of Trigger and Length of Classlabels must fit')
end;

% Time axis
o.T = [pre:post]'/Fs;
if (length(CL)==2) & (sz(1)==1),
        for k = 1:length(CL),
                X{k} = squeeze(D(:,cl==CL(k),:));
        end;

        % classification error 
        o.ERR = (1-mean(sign([-X{1},X{2}]),DIM))/2;
        
        % within-class accuracy
        o.BCG1 = (1 + mean(sign(-X{1}), DIM))/2;
        o.BCG2 = (1 + mean(sign( X{2}), DIM))/2;
        
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
        
end

if length(CL)==sz(1),
        [m,IX] = max(D,[],1);
        IX(isnan(m)) = NaN;
        IX = squeeze(IX);

        CMX = repmat(zeros,[size(IX,1),length(CL)*[1,1]]);
        for k = 1:length(CL),
        for j = 1:length(CL),
                CMX(:,k,j)=sum(IX(:,CL(k)==cl)==CL(j),2);
        end;
        end;
        
        o.KAP00 = zeros(size(CMX,1),1);
        o.Ksd00 = zeros(size(CMX,1),1);
        o.ACC00 = zeros(size(CMX,1),1);
        for k   = 1:size(CMX,1),
                [o.KAP00(k),o.Ksd00(k),h,z,o.ACC00(k)]=kappa(squeeze(CMX(k,:,:)));            
        end;
        o.datatype = 'TSD_BCI8';  % useful for PLOTA
        
elseif sz(1)==1,
        
else
%        error('invalid input arguments');
end;
