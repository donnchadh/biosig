% DEMO2 demonstrates the use of the data set III from the BCI competition 2003 for 
%   The demo shows the offline analysis for obtaining a classifier and 
%   uses a jack-knife method (leave-one-trial out) for validation. 
%
%
% References: 
% [1} A. Schlögl, C. Keinrath, R. Scherer, G. Pfurtscheller,
%     Information transfer of an EEG-based Bran-computer interface.
%     Proceedings of the 1st International IEEE EMBS Conference on Neural Engineering, Capri, Italy, Mar 20-22, 2003. 
% [2} Schlögl A., Neuper C. Pfurtscheller G.
%     Estimating the mutual information of an EEG-based Brain-Computer-Interface
%     Biomedizinische Technik 47(1-2): 3-8, 2002
% [3] Alois Schlögl (2000)
%     The electroencephalogram and the adaptive autoregressive model: theory and applications
%     Shaker Verlag, Aachen, Germany, (ISBN3-8265-7640-3). 


%	$Revision: 1.1 $
%	$Id: demo2.m,v 1.1 2003-06-17 07:21:06 schloegl Exp $
%	Copyright (C) 1999-2003 by Alois Schloegl <a.schloegl@ieee.org>	

cname=computer;

M0  = 7;
MOP = 3;
uc  = 30:5:80;

% load EEG-data S and classlabels cl 
% here, the data set III of the BCI competition 2003 is used. The data is available from  
%     http://www.dpmi.tu-graz.ac.at/~schloegl/bci/competition2003/ (untriggered, raw data) 

load s:\projects\bci\competition\dataset_BCIcomp1raw.mat
cl = c1;
Fs = 128;
trigchan = 4;
eegchan  = [1,3];

TRIG = gettrigger(S(:,trigchan))-2*Fs;
if length(TRIG)~=length(cl);
        fprintf(2,'number of Triggers (%i) does not fit size of class information (%i)',length(TRIG),length(cl));
	return;        
end;

if ~any(size(eegchan)==1)
	S = S(:,1:size(eegchan,1))*eegchan;
	eegchan=1:size(eegchan,2); 
end;

randn('state',0);
[a0,A0] = getar0(S(:,1:2),1:M0,1000,Fs/2);

T  = reshape((1:1152),16,1152/16)';
t0 = zeros(1152/16,1);
t0(25:72) = 1;
t0 = logical(t0);

p  = 3;
k  = 7;
UC0= 2^(-uc(k)/8);

% feature extraction for each chaannel
for ch = 1:length(eegchan),
        [ar{ch},e,REV(ch)] = aar(S(:,eegchan(ch)), [2,3], p, UC0, a0{p},A0{p});
end;

% get classifier 
[cc] = findclassifier1([cat(2,ar{:})],TRIG, cl,T,t0,3);

plota(cc.MDA.TSD);
