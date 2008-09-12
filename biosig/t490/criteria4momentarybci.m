function [K1,K2] = criteria4momentarybci(TO,Fs,trig,ERW)
% CRITERIA4MOMENTARYBCI evaluates the output of momentary self-paced BCI  
%
% [K1,K2] = criteria4momentarybci(TO,Fs,TRIG,ERW) 
%
% Input: 
% 	TO transducer output: 0 is no control state, i>0 is i-th control state
% 	Fs sampleing rate
% 	TRIG 	trigger information - its a vector in case of a single IC state
%		othewise TRIG{K} are the triggers for the K-th IC state
% 	ERW 	expected response window in seconds, default=[-.5,+.5]
%
% output:
% 	K1 	result the rule-based (heuristic) approach 
% 	K2 	results of the sample-by-sample approach 
%
% 	K1.HF  		hf-difference
% 	K{12}.kappa 	Cohen's kappa coefficient
% 	K{12}.H 	Confusion matrix 


%    $Id: criteria4momentarybci.m,v 1.2 2008-09-12 10:33:56 schloegl Exp $
%    Copyright (C) 2008 by Alois Schloegl <a.schloegl@ieee.org>	
%    This is part of the BIOSIG-toolbox http://biosig.sf.net/
%
%    BioSig is free software: you can redistribute it and/or modify
%    it under the terms of the GNU General Public License as published by
%    the Free Software Foundation, either version 3 of the License, or
%    (at your option) any later version.
%
%    BioSig is distributed in the hope that it will be useful,
%    but WITHOUT ANY WARRANTY; without even the implied warranty of
%    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%    GNU General Public License for more details.
%
%    You should have received a copy of the GNU General Public License
%    along with BioSig.  If not, see <http://www.gnu.org/licenses/>.


if nargin<4
	ERW = [-.5,.5];
end; 	

EUI = zeros(size(TO));
if iscell(trig)
	CL = [];
	T = [] 
	for K = 1:length(trig)
		T = [T; trig{K}(:)];
		CL = [CL; repmat(K,length(trig{K}),1)];
		for k=1:length(TRIG)
			EUI((TRIG(k)+ERW(1)*Fs) : (TRIG(k)+ERW(2)*Fs))=K; 
		end;
	end; 
	[T,ix]=sort(T); 
	CL = CL(ix);
else 
	TRIG = trig; 
	CL   = ones(size(TRIG));
	for k= 1:length(TRIG)
		EUI((TRIG(k)+ERW(1)*Fs) : (TRIG(k)+ERW(2)*Fs)) = CL(k); 
	end; 
end; 



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Jane's rules for the evaluation 

EV  = zeros(length(TO),2);  % default value is TN 

for k=1:length(TRIG)

% 1) The parameters ERWstart and ERWend are chosen to define the ERW 
%	around each momentary event label. For the present comparison, 
%	ERWstart is .5 seconds before the trigger, and ERWend is .5 seconds after the trigger.

	ix = max(1,TRIG(k)+ERW(1)*Fs):min(TRIG(k)+ERW(2)*Fs,length(TO)); 

% 2) The first detection within the range of ERWstart to ERWend 
%	relative to the trigger is considered a TP.

	F = find(TO(ix));
	if ~isempty(F)
		EV(ix(F(1)),1)=CL(k);  	
		EV(ix(F(1)),2)=TO(ix(F(1)));  	
		
% 3) Any additional detection(s) within the ERW after the first is considered a FP.

		EV(ix(F(2:end)),1) = 0; 		
		EV(ix(F(2:end)),2) = TO(ix(F(2:end)));  		

% 4) If there are no detections within the window, a single 
%	sample in the center of the window is marked FN.

	else 
		EV(TRIG(k),:)=[1,0]; 
	end; 

end;

% 5) Any detection outside of an ERW is considered a FP.

	ix = find(~EUI);
	F  = find(TO(ix));
	EV(ix(F),1) = 0;  % FP 		
	EV(ix(F),2) = TO(ix(F));  % FP 		
		
% 6) All other points (both inside and outside of ERWs) are considered TN.

	% initial value of EV	


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% summary statistics: compute kappa and hf-difference
K1 = kappa(EV(:,1),EV(:,2)); 

if length(K1.H)==2,
	%% HF difference = TP/(TP+FN) - FP/(TP+FP)
	K1.HF = K1.H(2,2)/sum(K1.H(2,:)) - K1.H(1,2)/sum(K1.H(:,2));  
end; 	


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% sample-based evaluation 
%% generate padding window - make padding window has the same size than ECW
T1 = TO;   
i0 = find(TO);
plot([EUI,TO]);
for k = i0(:)',
	cl = TO(k); 
	ix = max(1,k + ERW(1)*Fs) : min(k + ERW(2)*Fs,length(TO));
	if any((T1(ix)~=cl) & (T1(ix)>0))
		warning('overlapping windows');
	end; 
	T1(ix) = cl; 
end; 
plot([EUI,T1]);
K2 = kappa(EUI,T1);

