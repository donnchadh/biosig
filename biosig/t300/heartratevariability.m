function [X] = heartratevariability(RRI,arg2)
% HeartRateVariability Analysis according to [1]
%
% X = heartratevariability(RRI [,units])
% X = heartratevariability(qrsindex [,Fs])
% X = heartratevariability(HDR)
% X = heartratevariability(HDR.EVENT)
% X = heartratevariability(filename)
% 
% 
% INPUT
%   RRI 	R-R-intervales [in seconds]
%   HDR		as defined in the header structure of BioSig 
%		and returned by QRS-detection
%   filename 	with event information
%   Fs 		sampling rate - used for conversion into time axis
%   units	time units e.g. 'ms' (default: 's')  
%
% OUTPUT
%   X  		struct containing the results as defined by [1]
%   X.meanRR      meanRR = meanNN
%   X.SDRR		standard deviaation of RR intervales
%   X.RMSSD       rmsSD = SDSD
%     NN50count1
%     NN50count2
%     NN50count
%	pNN50
%	SD1		width of Poincaré plot; equivalent to sqrt(2)*RMSSD [2]
%	SD2		length of Poincaré plot; i.e. 2SDRR²+SDSD²/2 [2]
%	r_RR 		correlation coefficient [2]
%
%
% see also: QRSDETECT, BERGER, EVENTCODES.TXT
%
% Reference(s):
% [1] Heart Rate Variability
%       Standards of Measurement, physilogcial interpretation and clinical use.  
%       Taskforce of the European Society for Cardiology and the North Americal Society of Pacing and Electrophysiology.         
%       European Heart Journal (1996) 17, 354-381. 
% [2] M. Brennan, M.Palaniswami, P. Kamen
%	Do Existing Measures of Poincaré Plot Geometriy Reflect Nonlinear Features of Heart Rate Variablilty?
%	IEEE Trans Biomedical Eng. 48(11),2001, 

%	$Id: heartratevariability.m,v 1.4 2006-09-20 12:31:30 schloegl Exp $
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



%%%%%%%% check and convert the input arguments  %%%%%%%%%%
if ischar(RRI)
if exist(RRI,'file'); 
	RRI = sload(RRI); 
end;
end; 

Fs = 1; 
X.PhysDim = 's'; 
if nargin>1,
	if ischar(arg2)
		X.PhysDim = arg2;
	elseif isnumeric(arg2)
		Fs = arg2;	
	end; 
end;

	
if isstruct(RRI)
        Fs0 = NaN; 
        if isfield(RRI,'EVENT')
        	HDR = RRI; 
                EVENT = HDR.EVENT; 
		if isfield(HDR,'T0')
			X.T0 = HDR.T0; 
		end;	
		if isfield(HDR,'Patient')
			X.Patient = HDR.Patient;
		end;	
        else
                EVENT = RRI; 
        end;

        X.PhysDim = 's'; 
        if isfield(EVENT,'SampleRate')
                Fs0 = EVENT.SampleRate; 
        elseif isfield(RRI.SampleRate)
                Fs0 = RRI.EVENT.SampleRate; 
        else    
                warning('Invalid input argument causes unknown source sampleing rate.');
                Fs0 = 1; 
                X.PhysDim = '?'; 
        end;
        if isfield(EVENT,'POS') & isfield(EVENT,'TYP') & isfield(EVENT,'CHN') & isfield(EVENT,'DUR');
                ix = find(EVENT.TYP==hex2dec('0501'));
                if all(EVENT.CHN(ix(1)) == EVENT.CHN(ix));
                        on = EVENT.POS(EVENT.TYP==hex2dec('0501'))/Fs0;
                end;
        elseif isfield(EVENT,'POS') & isfield(EVENT,'TYP');
                on = EVENT.POS(EVENT.TYP==hex2dec('0501'))/Fs0;
        end;
        NN = diff(on);         
        
elseif ~any(diff(RRI)<0),	
        on = RRI(:)/Fs; 
        NN = diff(on);
	X.PhysDim = 's'; 
else	
        NN = RRI(:); 
        on = [0;cumsum(NN)];
end;

if nargout<1,
	% scatterplot for testing of the distribution 
	plot(RRI(1:end-1),RRI(2:end),'x');drawnow;
	return;
end;

%%%%%%%%  convert from any time unit into ms %%%%%%%%%%%%%%%%
[PhysDimCode,scale1] = physicalunits(X.PhysDim); 
X.PhysDim = 'ms';
[X.PhysDimCode,scale2] = physicalunits(X.PhysDim); 
t_scale = scale2./scale1;
NN = NN/t_scale; 
on = on/t_scale;


X.datatype = 'HeartRateVariability'; 

%%%%%%%%% time domain parameters %%%%%%%%%%%%%%%%%%%%
X.meanNN   = mean(NN); 
X.SDNN     = std(NN); 
X.RMSSD    = rms(diff(NN));  % SDSD
X.NN50count1 = sum(-diff(NN)>0.050/t_scale);
X.NN50count2 = sum( diff(NN)>0.050/t_scale);
X.NN50count  = sum(abs(diff(NN))>0.050/t_scale);
X.pNN50    = X.NN50count/sum(~isnan(NN)); 


g = acovf(center(NN(:)'),2);
X.SD1 	= sqrt(g(1)-g(2));
X.SD2 	= sqrt(g(1)+g(2));
X.SD1 	= sqrt(g(1)-g(2));
X.r_RR	= g(2)/g(1);



t   = round(NN * 128)/128; 
HIS = histo3(t(:)); 
X.HRVindex128 = HIS.N/max(HIS.H); 

%still missing 
%X.SDANN = 0;
%X.SDNNindex = 0; 

%SDNNindex
%SDSD  = RMSSD


%%%%%%% AR-based spectrum  analysis %%%%%%%%%%%%%
if 0, 
	y = log(NN); 
	[y,m]=center(y); 
	f0= exp(-m)/t_scale
elseif 0, 
	y = NN; 
	[y,m] = center(y); 
	f0= 1/(m*t_scale)
else
	f0 = 4; 
        [hrv,y] = berger(on*t_scale,f0); % resampleing 
	[y,m] = center(y/t_scale); 
end;

pmax = 100; 	
% choose levinson-durbin or Burg algorithm 
[mx,pe]=durlev(acovf(y(:)',pmax));
%[mx,pe]=lattice(y(:)',pmax); 

n = sum(~isnan(y));
[FPE,AIC,BIC,SBC,MDL,CAT,PHI,optFPE,optAIC,optBIC,optSBC,optMDL,optCAT,optPHI]=selmo(pe/pe(1),n);
X.mops = [optFPE,optAIC,optBIC,optSBC,optMDL,optCAT,optPHI]; 
% select model order 
X.mop = optBIC;
X.mop = optAIC; 
X.mop = 15;
[a,r] = arcext(mx,X.mop);

[h,f] = freqz(sqrt(pe(X.mop+1)/f0),[1,-a],256,f0);
ix = f<0.04;
X.VLF = trapz(f(ix),abs(h(ix)).^2);
ix = (f>0.04) & (f<0.15);
X.LF = trapz(f(ix),abs(h(ix)).^2);
ix = (f>0.15) & (f<0.40);
X.HF = trapz(f(ix),abs(h(ix)).^2);
ix = (f>0.15) & (f<0.40);
X.TotalPower = trapz(f,abs(h).^2);


if 0, 
	% spectral decomposition method
	[w,A,B,R,P,F,ip] = ar_spa(a,f0,pe(X.mop+1));
	ix = (imag(F)==0);

	ixVLF = ((w>=0)  & (w<.04)); F1 = real(F); F1(~ixVLF)= NaN;
	ixLF  = (w>=.04) & (w<=.15); F2 = real(F); F2(~ixLF) = NaN;
	ixHF  = (w>.15)  & (w<=.4) ; F3 = real(F); F3(~ixHF) = NaN;               

	X.VLF = sumskipnan(F1,2); 
	X.LF  = sumskipnan(F2,2); 
	X.HF  = sumskipnan(F3,2); 
	X.TotalPower = sumskipnan(real(F),2);

	% center frequencies
		% mean based on amplitude
	X.VLF_f0 = sumskipnan(w(ixVLF).*A(ixVLF))/sum(A(ixVLF));
	X.LF_f0 = sumskipnan(w(ixLF).*A(ixLF))/sum(A(ixLF));
	X.HF_f0 = sumskipnan(w(ixHF).*A(ixHF))/sum(A(ixHF));
		% mean based on power
	%X.VLF_f0 = sumskipnan(w(ixVLF).*F1(ixVLF))/sum(F1(ixVLF));
	%X.LF_f0 = sumskipnan(w(ixLF).*F2(ixLF))/sum(F2(ixLF));
	%X.HF_f0 = sumskipnan(w(ixHF).*F3(ixHF))/sum(F3(ixHF));

end;

X.LFHFratio = X.LF./X.HF; 
X.LFnu = X.LF./(X.TotalPower-X.VLF);
X.HFnu = X.HF./(X.TotalPower-X.VLF);

%%%%%%% FFT-based spectrum  analysis %%%%%%%%%%%%%
% currently not implemented 


%%%%%%% slope %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if diff(on([1,end])*t_scale)>5*3600, 	%calculate only if more than 5 hours 
	t = (on(1:end-1) + on(2:end))/2;
	t = t * t_scale;
	f1 = 0.0001; f2 = 0.04; 
	[x1,n1] = sumskipnan(NN.*exp(-2*pi*j*f1*t));
	[x2,n2] = sumskipnan(NN.*exp(-2*pi*j*f2*t));
	X.alpha = 2*log(abs((x1*n2)/(x2*n1)))./log(f1/f2);
end; 
