function [M,SE,R] = tfmvar(s,TRIG,T,MOP,f,Fs)
% TFMVAR Time-Frequency MVAR analysis
%   time-frequency analysis of 
%   multivariate stochastic processes. 
%
% [R] = tfmvar(s,TRIG,T,MOP,f,Fs)
% [M,SE,R] = tfmvar(s,TRIG,T,MOP,f,Fs)
%     	M and SE contain the mean 
%	and the standard error of the mean  
%       of the following characteristics:
%
%   S		Auto- and Cross-spectra
%   h		transfer function 		
%   logS   	log(abs(S))
%   logh   	log(abs(h))
%   phaseS   	phase of S
%   phaseh   	phase of h
%   COH		coherence
%   coh		coherence neglecting the cross-correlation 
%		  due to the innovation process
%   pCOH 	partial coherence
%   PDC	 	partial directed coherence [2, 5]
%   DTF 	directed transfer function [3, 6]
%   dDTF 	modified DTF [8]
%   ffDTF 	modified DTF [8]
%   AR		MVAR parameters
%   C		covariance matrix of the innovation process	
%   DC		directed granger causality [2,3,5,6]
%
% [R] = tfmvar(s,TRIG,T,MOP,f,Fs)
%   R is a struct containing M and SE as well as a few more 
%     parameters for visualization
%
%  The standard error is calculated with a jackknife-method,
%  based on LEAVE-ONE-TRIAL-OUT. Therefore, the SE need to be 
%  rescaled, depending on the needs. 
%     SE 
%	standard error of the mean from the bootstrap results 
%	This has usually no common meaning (pretty much useless). 
%     SE*(N-1)^(1/2) 
&	standard deviation of the means from the bootstrapping
%	It can be interpreted as the standard error of the mean 
%	across all trials.
%	This value becames smaller if the number of trials increase. 	
%     SE*(N-1) 	
%	standard error of the mean across all trials.
%	This value provides a realistic value for the confidence 
%	interval of the estimates and can be used to test the 
%	significance. 
%     SE*(N-1)*N^(1/2) 
%	[estimated] standard deviation of a single trial estimate
%	This value is important for a single-trial classification.  
%		
%
% see also: tsa/MVAR, tsa/MVFREQZ, PLOTA
%
% Reference(s):
% [1] Kay S. M., Marple S. L., Spectrum Analysis - A Modern Perspective, Proc. IEEE, 1981
% [2] Baccala L. A., Sameshima K., Partial Directed Coherence: A New Concept in Neural Structure Determination, Biological Cybernetics 84, 2001
% [3] Kaminski M., Blinowska K., Szelenberger W., Topographic Analysis of Coherence and Propagation of EEG Activity During Sleep and Wakefulness, Electroencephalography and Clinical Neurophysiology 102, 1997
% [4] Franaszczuk P. J., Bergey G. K., An Autoregressive Method for the Measurement of Synchronization of Interictal and Ictal EEG Signals, Biological Cybernetics 81, 1999
% [5] Sameshima K., Baccala L. A., Using Partial Directed Coherence to Describe Neuronal Ensemble Interactions, Journal of Neuroscience Methods 94, 1999
% [6] Kaminski M., Ding M., Truccolo W. A., Bressler S. L., Evaluating Causal Relations in Neural Systems: Granger Causality, Directed Transfer Function and Statistical Assessment of Significance, Biological Cybernetics 85, 2001
% [7] Liang H., Ding M., Bressler S. L., On the Tracking of Dynamic Functional Relations in Monkey Cerebral Cortex, Neurocomputing, 2000
% [8] Korzeniewska A., Manczak M., Kaminski M., Blinowska K. J., Kasicki S., Determination of Information Flow Direction Among Brain Structures By a Modified Directed Transfer Function (dDTF) Method, Journal of Neuroscience Methods 125, 2003

%	$Revision: 1.1 $
%	$Id: tfmvar.m,v 1.1 2004-02-23 15:43:20 schloegl Exp $
%	Copyright (C) 2004 by Alois Schloegl <a.schloegl@ieee.org>	
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


R.datatype = 'TF-MVAR';
R.N = length(TRIG);
R.T = (T(1,:)+T(2,:))/(2*Fs); 
R.F = f; 
R.MOP = MOP; 
R.nan_ratio = zeros(1,size(T,2));

[N,m] = size(s);

sz = [m,m,length(f),size(T,2)];
R.M.S = zeros(sz);
R.M.h = zeros(sz);
R.M.phaseS = zeros(sz);
R.M.phaseh = zeros(sz);
R.M.logS   = zeros(sz);
R.M.logh   = zeros(sz);
R.M.COH    = zeros(sz);
R.M.coh    = zeros(sz);
R.M.PDC    = zeros(sz);
R.M.DTF    = zeros(sz);
R.M.pCOH   = zeros(sz);
R.M.dDTF   = zeros(sz);
R.M.ffDTF  = zeros(sz);
R.M.pCOH2  = zeros(sz);
sz(3)  = 1;
R.M.DC = zeros(sz);
R.M.C  = zeros(sz);
sz(2)  = m*MOP;
R.M.AR = zeros(sz);

R.SE = R.M;

sz    = [m,m,length(f),length(TRIG)];
S     = zeros(sz);
h     = zeros(sz);
COH   = zeros(sz);
coh   = zeros(sz);
PDC   = zeros(sz);
DTF   = zeros(sz);
pCOH  = zeros(sz);
dDTF  = zeros(sz);
ffDTF = zeros(sz);
pCOH2 = zeros(sz);
sz(3) = 1;
C  = zeros(sz);
DC = zeros(sz);
sz(2) = m*MOP;
AR = zeros(sz);
tic,
for k1 = 1:size(T,2),
        [S0,sz0] = trigg(s,TRIG,T(1,k1),T(2,k1),1+MOP);
	R.nan_ratio(k1) = mean(isnan(S0(:)));
        for k2 = 1:length(TRIG),
                sel = [1:k2-1,k2+1:length(TRIG)];
                [S0,sz0] = trigg(s,TRIG(sel),T(1,k1),T(2,k1),1+MOP);
                %fprintf(2,'\nExtract epochs done\nNumber of Trials: %i : %i\n',length(t0),length(t1));
                
                [A,RC,PE] = mvar(S0',MOP,5);
                X.A = [eye(size(S0,1)),-A];
                X.B = [eye(size(S0,1))];
                X.C = PE(:,MOP*size(S0,1)+(1:size(S0,1)))';
                X.datatype = 'MVAR';

                [S(:,:,:,k2),  h(:,:,:,k2), PDC(:,:,:,k2), COH(:,:,:,k2), DTF(:,:,:,k2), DC(:,:,1,k2), pCOH(:,:,:,k2), dDTF(:,:,:,k2), ffDTF(:,:,:,k2), pCOH2(:,:,:,k2),coh(:,:,:,k2)] = mvfreqz(X.B,X.A,X.C,f,Fs);
                
                AR(:,:,1,k2) = A; 
                C(:,:,1,k2)  = X.C;
                
                fprintf(2,'%.0f\t%.0f\t%.1f\n',[k1,k2,toc]);tic;
        end;
        
        [R.SE.phaseS(:,:,:,k1), R.M.phaseS(:,:,:,k1)] = sem(angle(S),4);
        [R.SE.phaseh(:,:,:,k1), R.M.phaseh(:,:,:,k1)] = sem(angle(h),4);
        [R.SE.S(:,:,:,k1),      R.M.S(:,:,:,k1)     ] = sem(S,4);
        [R.SE.h(:,:,:,k1),      R.M.h(:,:,:,k1)     ] = sem(h,4);
        [R.SE.logS(:,:,:,k1),   R.M.logS(:,:,:,k1)  ] = sem(log(abs(S)),4);
        [R.SE.logh(:,:,:,k1),   R.M.logh(:,:,:,k1)  ] = sem(log(abs(h)),4);
        [R.SE.PDC(:,:,:,k1),    R.M.PDC(:,:,:,k1)   ] = sem(PDC,4);
        [R.SE.COH(:,:,:,k1),    R.M.COH(:,:,:,k1)   ] = sem(COH,4);
        [R.SE.coh(:,:,:,k1),    R.M.coh(:,:,:,k1)   ] = sem(coh,4);
        [R.SE.DTF(:,:,:,k1),    R.M.DTF(:,:,:,k1)   ] = sem(DTF,4);
        [R.SE.pCOH(:,:,:,k1),   R.M.pCOH(:,:,:,k1)  ] = sem(pCOH,4);
        [R.SE.dDTF(:,:,:,k1),   R.M.dDTF(:,:,:,k1)  ] = sem(dDTF,4);
        [R.SE.ffDTF(:,:,:,k1),  R.M.ffDTF(:,:,:,k1) ] = sem(ffDTF,4);
        [R.SE.pCOH2(:,:,:,k1),  R.M.pCOH2(:,:,:,k1) ] = sem(pCOH2,4);
        [R.SE.AR(:,:,1,k1),     R.M.AR(:,:,1,k1)    ] = sem(AR,4);
        [R.SE.DC(:,:,1,k1),     R.M.DC(:,:,1,k1)    ] = sem(DC,4);
        [R.SE.C(:,:,1,k1),      R.M.C(:,:,1,k1)     ] = sem(C,4);
        
        %% In order to obtaine the standard error of the mean, 
        %% this SE must be multiplied by N-1, with N = length(TRIG)
end;

if any(R.nan_ratio),
	fprintf(2,'Warning GMVAR: up to %5.2f are missing values (NaN).\n',max(R.nan_ratio)*100);
	fprintf(2,'This may cause underestimating the standard error.\n');
end;	

if nargout > 1, 
	M = R.M;
	R = rmfield(R,'M');
	SE= R.SE; 
	R = rmfield(R,'SE');
else 
	M = R;
end;

