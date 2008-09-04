
% DEMO2 demonstrates the use of the data set III from the BCI competition 2003 for 
%   The demo shows the offline analysis for obtaining a classifier and 
%   uses a jack-knife method (leave-one-trial out) for validation. 
%   AAR parameters are extracted 
%
%
% References: 
% [1] A. Schlögl, C. Keinrath, R. Scherer, G. Pfurtscheller,
%       Information transfer of an EEG-based Bran-computer interface.
%       Proceedings of the 1st International IEEE EMBS Conference on Neural Engineering, Capri, Italy, Mar 20-22, 2003. 
% [2] Schlögl A., Neuper C. Pfurtscheller G.
%       Estimating the mutual information of an EEG-based Brain-Computer-Interface
%       Biomedizinische Technik 47(1-2): 3-8, 2002.
% [3] Alois Schlögl (2000)
%       The electroencephalogram and the adaptive autoregressive model: theory and applications
%       Shaker Verlag, Aachen, Germany, (ISBN3-8265-7640-3). 
% [4] A. Schlögl, J. Kronegg, J.E. Huggins, S. G. Mason.
%       Evaluation criteria in BCI research.
%       (Eds.) G. Dornhege, J.R. Millan, T. Hinterberger, D.J. McFarland, K.-R.Müller,
%       Towards Brain-Computer Interfacing. MIT Press, 2007.
% [5] A. Schlögl, F.Y. Lee, H. Bischof, G. Pfurtscheller
%   	Characterization of Four-Class Motor Imagery EEG Data for the BCI-Competition 2005.
%   	Journal of neural engineering 2 (2005) 4, S. L14-L22
% [6] A. Schlögl, C. Brunner, R. Scherer, A. Glatz;
%   	BioSig - an open source software library for BCI research.
%   	(Eds.) G. Dornhege, J.R. Millan, T. Hinterberger, D.J. McFarland, K.-R. Müller;
%   	Towards Brain-Computer Interfacing, MIT Press, 2007, p.347-358. 
% [7] A. Schlögl, C. Brunner
%   	BioSig: A Free and Open Source Software Library for BCI Research.
%	Computer (2008, In Press)	

%	$Id: demo2.m,v 1.12 2008-09-04 08:02:51 schloegl Exp $
%	Copyright (C) 1999-2003,2006,2007,2008 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 3
% of the  License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
filename = 'B0101T.gdf';
if ~exist(filename,'file')
	unix('wget http://hci.tugraz.at/~schloegl/bci/competition2008/B0101T.gdf');
end; 
	
% Step 1a: load data ============================%
fprintf(1,'Step 1: Prepare data.\n',filename);
fprintf(1,'\ta: Load data file %s.\n',filename);
[s,HDR]=sload(filename);

% Step 1b: extract trigger and classlabels (if not already available) ==%
%--------- extraction from event table 
fprintf(1,'\tb: Extract trigger and classlabels.\n');
ix = find((HDR.EVENT.TYP>768)&(HDR.EVENT.TYP<777)); % 0x0300..0x03ff
HDR.TRIG = HDR.EVENT.POS(ix)-3*HDR.SampleRate;
HDR.Classlabel = HDR.EVENT.TYP(ix)-768; % 0x0300
%--------  extraction from trigger channel: 
% HDR.TRIG = gettrigger(s(:,triggerchannel)); 


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step 2: Preprocessing and artifact processing ====================%
%   2a: Overflowdetection: eeghist.m, [Schlögl et al. 1999] 
fprintf(1,'Step 2: Preprocessing.\n');
fprintf(1,'\ta: Quality control with histogram analysis [Schloegl et al. 1999].\n');
%Q = eeg2hist(filename); 
Q = eeg2hist(filename,'manual'); % manual selection of threshold
clear H;
H.FileName  = Q.FileName;
H.THRESHOLD = Q.THRESHOLD; 
[HDR]=sopen(H,'r',0);[s,HDR]=sread(HDR);HDR=sclose(HDR);
%   2b: Muscle detection: detectmuscle.m
fprintf(1,'\tb: Detection of muscle artifacts.\n');

%   2c: resampling 
fprintf(1,'\tc: resampling.\n');
DIV = 1; 
s = rs(s,DIV,1);   % downsampling by a factor of DIV; 
%s = rs(s,256,100); % downsampling from 256 to 100 Hz. 
HDR.SampleRate = HDR.SampleRate/DIV; 

%   2d: Correction of EOG artifacts: regress_eog.m, get_regress_eog.m   
% 		[Schlögl et al. 2007]
fprintf(1,'\td: reduce EOG artifacts.\n');
eogchan=identify_eog_channels(filename); 
	% eogchan can be matrix in order to convert 
      	%     monopolar EOG to bipolar EOG channels
eegchan=find(HDR.CHANTYP=='E'); % exclude any non-eeg channel. 
%R = regress_eog(s,eegchan,eogchan); 
%s = s*R.r0; 	% reduce EOG artifacts 

%   2e: spatial filters 
%       spatial filters can be used to focus on specific areas.
%       examples are bipolar (BIP), common average reference (CAR), 
%       Hjorth's Laplace derivation (LAP), Common spatiol patterns (CSP)
fprintf(1,'\te: spatial filters.\n');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step 3: Feature extraction ====================%
eegchan=find(HDR.CHANTYP=='E'); % select EEG channels 
fprintf(1,'Step 3: feature extraction.\n');

p = 9; 
MODE.MOP = [0,p,0];	% order of AAR model
MODE.UC  = 0.0085;		% update coefficient of AAR model %    3a: Time domain parameters 

%    3a: Time-dependent parameters - motivated by log(Hjorth)
fprintf(1,'\ta: TDP (log(Hjorth)).\n');
a1 = tdp(s(:,eegchan),p,MODE.UC);

%    3b: Adaptive Autoregressive Parameters
fprintf(1,'\tb: Adaptive Autoregressive parameters (AAR).\n');
a2 = [];
for ch = 1:length(eegchan),
	X = tvaar(s(:,eegchan(ch)),MODE.MOP,MODE.UC); 
     	X = tvaar(s(:,eegchan(ch)),X);		% AAR estimation
     	a2 = [a2,X.AAR,log(X.PE)];	
end; 

%    3c: bandpower
fprintf(1,'\tc: bandpower.\n');
bands = [10,12;16,24]; % define frequency bands 
win = 1; 	% length of smoothing window in seconds
s1 = s; s1(isnan(s1))=0; 
a3 = bandpower(s, HDR.SampleRate, bands, win);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step 4: Classification ==================== %
fprintf(1,'Step 4: classification.\n');
TrialLen = 9; % seconds
SegmentLen = 100; % samples
NoS = ceil(9*HDR.SampleRate/SegmentLen);
MODE.T   = reshape((1:NoS*SegmentLen),SegmentLen,NoS)';
% valid segments for building classifier must be after cue.
MODE.WIN = MODE.T(:,1) > 3*HDR.SampleRate+1;	% cue @ t=3s.

%%%% X-V based on Jackknife-procedure (leave-K-trials-out)
K = 1; 
NG = ceil([1:length(HDR.Classlabel)]'/K);

TYPE = 'LDA';	% classifier type.0 
% other possible classifiers are: MDA, LD2, LD3, LD4, GDBC, SVM, RBF, NBC, aNBC, LDA/GSVD, MDA/GSVD, LDA/sparse, MDA/sparse 

% search best segment, cross-validation using jackknife procedure, LDA
CC1 = findclassifier(a1, HDR.TRIG, [HDR.Classlabel,NG], MODE.T, MODE.WIN, TYPE);
CC2 = findclassifier(a2, HDR.TRIG, [HDR.Classlabel,NG], MODE.T, MODE.WIN, TYPE);
CC3 = findclassifier(a3, HDR.TRIG, [HDR.Classlabel,NG], MODE.T, MODE.WIN, TYPE);
CC1.TSD.T = CC1.TSD.T/HDR.SampleRate;
CC2.TSD.T = CC2.TSD.T/HDR.SampleRate;
CC3.TSD.T = CC3.TSD.T/HDR.SampleRate;

% For online feedback, the weights of the linear classifier 
%   are available through 
fprintf(1,'\tb: weights of linear classifier.\n');
CC1.weights
CC2.weights
CC3.weights
% the first element represents the bias. 

% The chosen time segment used for computing the classifiers are: 
fprintf(1,'\tc: choosen time segment.\n');
MODE.T(CC1.TI,[1,end])/HDR.SampleRate, 
MODE.T(CC2.TI,[1,end])/HDR.SampleRate, 
MODE.T(CC3.TI,[1:end])/HDR.SampleRate, 
 
% Accordingly, the time-varying distance is available 
d1 = [ones(size(a1,1),1),a1]*CC1.weights;
d2 = [ones(size(a2,1),1),a2]*CC2.weights;
d3 = [ones(size(a3,1),1),a3]*CC3.weights;
% Note, if the same a1,a2,a3 were already used for classifier training, 
%   these results are subject to overfitting. 

% Alternatively, you can setup your own cross-validation procedure 
% using train_sc.m and test_sc.m.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step 5: Show results  ==================== %
fprintf(1,'Step 5: classifier.\n');

% the various evaluation criteria are discussed in Schlogl, Kronegg et 2007. 
figure(1);
plota(CC1.TSD)

figure(2);
plota(CC2.TSD)

figure(3);
plota(CC3.TSD)

figure(4)
M = length(unique(HDR.Classlabel));
clf;
plot(CC1.TSD.T,[sum(CC1.TSD.I,2),sum(CC2.TSD.I,2),sum(CC3.TSD.I,2)]*(M-1)/M);
legend({'TDP','AAR','BP'})

