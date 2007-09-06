
% DEMO2 demonstrates the use of the data set III from the BCI competition 2003 for 
%   The demo shows the offline analysis for obtaining a classifier and 
%   uses a jack-knife method (leave-one-trial out) for validation. 
%
%
% References: 
% [1} A. Schlögl, C. Keinrath, R. Scherer, G. Pfurtscheller,
%       Information transfer of an EEG-based Bran-computer interface.
%       Proceedings of the 1st International IEEE EMBS Conference on Neural Engineering, Capri, Italy, Mar 20-22, 2003. 
% [2} Schlögl A., Neuper C. Pfurtscheller G.
%       Estimating the mutual information of an EEG-based Brain-Computer-Interface
%       Biomedizinische Technik 47(1-2): 3-8, 2002
% [3] Alois Schlögl (2000)
%       The electroencephalogram and the adaptive autoregressive model: theory and applications
%       Shaker Verlag, Aachen, Germany, (ISBN3-8265-7640-3). 
% [4] A. Schlögl, J. Kronegg, J.E. Huggins, S. G. Mason.
%       Evaluation criteria in BCI research.
%       (Eds.) G. Dornhege, J.R. Millan, T. Hinterberger, D.J. McFarland, K.-R.Müller,
%       Towards Brain-Computer Interfacing. MIT press (accepted)


%	$Revision: 1.7 $
%	$Id: demo2.m,v 1.7 2007-09-06 13:23:20 schloegl Exp $
%	Copyright (C) 1999-2003,2006,2007 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
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

M0  = 7;
MOP = 3;
uc  = 30:5:80;

% load EEG-data S and classlabels cl 
% here, the data set III of the BCI competition 2003 is used. The data is available from  
%     http://hci.tugraz.at/schloegl/bci/competition2003/ (untriggered, raw data) 

fn = 'dataset_BCIcomp1raw.mat'; 
if ~exist(fn,'file')
        system('wget http://hci.tugraz.at/schloegl/bci/competition2003/dataset_BCIcomp1raw.mat');
end;        
load(fn);
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

MODE.T  = reshape((1:1152),16,1152/16)';	% define segments 
MODE.WIN = MODE.T(:,1) > 3*Fs/8+1;	        % valid segments for building classifier
MODE.MOP = [0,3,0];				% order of AAR model
MODE.UC  = 2^(-(7+5)*5/8);			% update coefficient of AAR model 

% estimate AAR model parameters
a2 = [];
for ch = 1:length(eegchan),
	X = tvaar(S(:,eegchan(ch)),MODE.MOP,MODE.UC); % 1st run used to get reasonable initial values 
       	X = tvaar(S(:,eegchan(ch)),X);		% AAR estimation
       	a2 = [a2,X.AAR]; 
end; 

% get classifier 
[cc] = findclassifier(a2, TRIG, cl, MODE.T, MODE.WIN);
cc.TSD.T = cc.TSD.T/Fs;
plota(cc.TSD);

m = {'LDA','NBC','aNBC','LD2','LD3','LD4','LD5','MDA','MD2','MD3','GRB','QDA','GDBC','LDA3/GSVD','SVM','REG'};
for k = 1:length(m);
	cc = findclassifier(a2, TRIG, cl, MODE.T, MODE.WIN, m{k});
	fprintf(1,'%s\t%s\n',m{k},cc.datatype);
	plota(cc.TSD);	
	suptitle(m{k});
end; 

MODE.TYPE = 'LD3';
for k = 0:10,
	MODE.hyperparameter.gamma=k/10;
	cc = findclassifier(a2, TRIG, cl, MODE.T, MODE.WIN, MODE);
	fprintf(1,'gamma=%f\t%s\n',MODE.hyperparameter.gamma,cc.datatype);	plota(cc.TSD);
end; 

	cc = findclassifier(a2, TRIG, cl, MODE.T, MODE.WIN, 'CSP');
	fprintf(1,'%s\n',cc.datatype);	plota(cc.TSD);

	[b,a] = butter(5,[7,30]/Fs*2);
	s = S(:,eegchan);
	s(isnan(s))=0; 
	cc = findclassifier(filter(b,a,s), TRIG, cl, MODE.T, MODE.WIN, 'CSP');
	fprintf(1,'%s\n',cc.datatype);	plota(cc.TSD);


