function R = evoked_potential(fn,CHAN,t1,t2)
% EVOKED_POTENTIAL estimates evoked potentials (EP's)
%
%  R = EVOKED_POTENTIAL(filename, CHAN, t_start, t_end)
%     filename  filename
%     CHAN      channel selection; default: 0 (all)
%     t_start   start time (relative to trigger time point
%     t_end     end time relative to trigger 
%
%  The trigger information must be available in the biosig file. 
%  The EP is calculated for each selected channel, if classlabels 
%  are available, the EP is calculated for each class
% 
%  The 

%	$Id: evoked_potential.m,v 1.2 2006-05-12 19:41:59 schloegl Exp $
%	Copyright (C) 2005 by Alois Schloegl <a.schloegl@ieee.org>	
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


%% 
[S,HDR]=sload(fn,CHAN);

if ~isfield(HDR,'TRIG') 
        error('trigger information is missing')
end;        
if ~isfield(HDR,'Classlabel');
        HDR.Classlabel = ones(size(HDR.TRIG)); 
end;        

if nargin<4,
        t1 = 2; t2 = 4;
end;
%HDR.Classlabel = HDR.EVENT.TYP;
%HDR.TRIG = HDR.EVENT.POS;

R0 = []; se=[];m=[];
CL = unique(HDR.Classlabel(:))';
for cl = 1:length(CL), 
        t  = [t1*HDR.SampleRate:t2*HDR.SampleRate]'/HDR.SampleRate;
        [s,sz] = trigg(S,HDR.TRIG(HDR.Classlabel==CL(cl)),t1*HDR.SampleRate,t2*HDR.SampleRate); 
        N(cl)  = length(HDR.TRIG);
        [se(:,:,cl), m(:,:,cl)] = sem(reshape(s,sz),3); 
        RES = statistic(reshape(s,sz),3); 
        R0.SUM(:,:,cl) = RES.SUM';
        R0.N(:,:,cl) = RES.N';
        R0.SSQ(:,:,cl) = RES.SSQ';
end; 
R0.datatype = 'MEAN+STD';
R0.T = t;
%R0.trigger = 3*HDR.SampleRate; 

R = R0; 
if all(size(CHAN)>1), 
elseif (CHAN==0)
	R.Label = HDR.Label; 
elseif all(CHAN>0) 
	R.Label = HDR.Label(CHAN,:); 
end;

if nargout>0, return; end

R0.MEAN = R0.SUM ./ R0.N;			% mean 
R0.SSQ0	= R0.SSQ - real(R0.SUM).*real(R0.MEAN) - imag(R0.SUM).*imag(R0.MEAN);	% sum square of mean removed

sz = [size(R.SUM),1];
R.SUM = reshape(R.SUM,[sz(1),prod(sz(2:3))])'; 
R.N   = reshape(R.N,  [sz(1),prod(sz(2:3))])'; 
R.SSQ = reshape(R.SSQ,[sz(1),prod(sz(2:3))])'; 

R.MEAN 	= R.SUM./R.N;			% mean 
R.MSQ  	= R.SSQ./R.N;;			% mean square
R.RMS  	= sqrt(R.MSQ);			% root mean square
%R.SSQ0	= R.SSQ-R.SUM.*R.MEAN;		% sum square of mean removed
R.SSQ0	= R.SSQ - real(R.SUM).*real(R.MEAN) - imag(R.SUM).*imag(R.MEAN);	% sum square of mean removed
    n1 	= max(R.N-1,0);			% in case of n=0 and n=1, the (biased) variance, STD and SEM are INF
R.VAR  	= R.SSQ0./n1;	     		% variance (unbiased) 
R.STD  	= sqrt(R.VAR);		     	% standard deviation
R.SEM  	= sqrt(R.SSQ0./(R.N.*n1)); 	% standard error of the mean


if nargout>0, return; end

for k1 = 1:sz(2),
        for k2=1:sz(3),
                nf(k1,k2)=subplot(sz(2),sz(3),(k1-1)*sz(3)+k2); 
        end;
end;
if all(size(CHAN>1)), 
elseif (CHAN==0)
	R.Label = HDR.Label; 
elseif all(CHAN>0) 
	R.Label = HDR.Label(CHAN,:); 
end;
plota(R,nf)

for k=1:size(nf,1),
	set(nf(k),'YLabel',HDR.Label(CHAN(k),:))
end;	
for k=1:size(nf,2),
%	set(nf(k),'Title',sprintf('Class #))
end;	


