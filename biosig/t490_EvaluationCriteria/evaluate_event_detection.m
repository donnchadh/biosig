function RES = evaluate_event_detection(D, trig)
% EVALUATE_EVENT_DETECTION quantifizes the performance of an 
%  event detection algorithm producing a detection trace D. 
%  The detection trace is compared to the reference trigger values, 
%  
%  A sample-based evaluation is used for measuring the performance
%  and for optimizing unknown parameters, like delay factor (TAU),
%  window length (WINLEN), and the detection threshold (TH). 
%
%  Based on the found detection threshold, parameter of an event-based
%  evaluation are reported. 

%       $Id$
%       Copyright (c) 2011 by  Alois Schloegl <alois.schloegl@gmail.com>
%       This is part of the BIOSIG-toolbox http://biosig.sf.net/
%
% Biosig is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% version 3 of the License, or (at your option) any later version.
%
% Biosig is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
%
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the
% Free Software Foundation, Inc., 59 Temple Place - Suite 330,
% Boston, MA  02111-1307, USA.
%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% rank sample data, 
% a simple algorithm not resolving ties is used.  
% d = ranks(D);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
d = zeros(size(X));
[sX, ix] = sort(X,1);
[tmp,d]  = sort(ix,1);     % r yields the rank of each element         
d(isnan(X)) = nan;
    

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Find onset and offset of optimal detection window. 
% This is a 2-dimensional optimization problem 
% with window size larger than 0.
% For optimization, the correlation on the ranked samples is
% used because it is faster than AUC or optimum Kappa
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%  initialize
IX1 = 0; 
IX2 = 0;
%% reference
r = zeros(size(D));
r(trig) = 1;
R0 = corrcoef(d,r);

while (1) 
	IX1 = IX1+1;
	r(trig + IX1) = 1;
	R1 = corrcoef(d,r);
	d11 = R1-R0;
	if R1 > R0,
		R0 = R1; 
		d12 = 0;  
	else 
		% no improvement, revert. 
		r(trig + IX1) = 0;
		IX1 = IX1-1;
		% go opposite direction 
		r(trig + IX1) = 0;
		IX1 = IX1-1;
		R1  = corrcoef(d,r);
		d12 = -(R1 - R0);
		if R1 > R0,
			R0 = R1; 
		else
			% no improvement, revert. 
			IX1=IX1+1;
			r(trig + IX1) = 1;
		end 
	end		

	IX2 = IX2-1;
	r(trig + IX2) = 1;
	R2 = corrcoef(d,r);
	d21 = R2 - R0;
	if R2 > R0,
		R0 = R2;
		d22 = 0;  
	else 
		% no improvement, revert. 
		r(trig + IX2) = 0;
		IX2 = IX2+1;
		% go opposite direction 
		r(trig + IX2) = 0;
		IX2 = IX2-1;
		R2  = corrcoef(d,r);
		d22 = -(R2 - R0);
		if R2 > R0,
			R0  = R2; 
		else
			% no improvement, revert. 
			IX2 = IX2-1;
			r(trig + IX2) = 1;
		end 
	end		
	if all([d11,d21,d22,d12] <= 0) break; end; 
	if -IX2 >= IX1, break end; 	% no convergence
end;
RES.WIN = [IX2:IX1];
if isempty(RES.WIN) return; end; 


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  ROC analysis: get AUC, and Threshold with maximum kappa
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
R = roc2(D,r); 

R.THRESHOLD = 



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Event-based analysis 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




