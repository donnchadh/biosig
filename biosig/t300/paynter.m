function [B,A]=paynter(tau,fs,mode)
% PAYNTER returns the filter coefficients for a Paynter Filter
%   Usually, the filter is applied to the rectified electromyogram (EMG).
%   Then, the output is amplitude-demodulated EMG 
%
%  The amplitude demodulated EMG can be obtained through
%      y = filter(B,A,abs(x));
%   with 
%       [B,A]=paynter(tau,fs)
%       [B,A]=paynter(tau,fs,'modified')
%       [B,A]=paynter(tau,fs,'bessel-modified')
%
%       tau	time constant
%       fs	sampling rate
%
%
% REFERENCE(S):
% [1] Platt, Ronald S., Eric A. Hajduk, Manuel Hulliger, and Paul A. Easton. 
%    A modified Bessel filter for amplitude demodulation of respiratory electromyograms. 
%    J. Appl. Physiol. 84(1): 378-388, 1998.

%	$Revision: 1.1 $
%	$Id: paynter.m,v 1.1 2004-04-09 11:03:25 schloegl Exp $
%	Copyright (C) 2000-2001, 2004 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


if nargin<3,
        mode='pay';
end;

if length(mode)<3,
        fprintf(2,'Invalid mode in PAYNTER.M\n');
elseif mode(1:3)=='mod'
        mode='mod';
elseif mode(1:3)=='bes'
        mode='bes';
end;

RC=1/tau;
if mode=='pay',
	%[B,A]=bilinear(1,rev([1, 3.2*RC, 4*RC*RC, 3.2*RC^3]),fs);
	[B,A]=bilinear(1,[1, 3.2*RC, 4*RC*RC, 3.2*RC^3],fs);
elseif mode=='mod'
	%[B,A]=bilinear(rev([1,0,RC*RC]),rev([1, 3.2*RC, 4*RC*RC, 3.2*RC^3]),fs);
	[B,A]=bilinear([1,0,RC*RC],[1, 3.2*RC, 4*RC*RC, 3.2*RC^3],fs);
elseif mode=='bes'
	B(1) = 1.6408979251842E-01; A(1) = 1.6408979251842E-01
	B(2) = 0;	A(2) = 1.1486285476290E+00
	B(3) = 1.3754616767430E-01;  A(3) = 3.7109537692628E+00
	B(4) = 0;	A(4) = 7.2157434402333E+00
	B(5) = 3.2736655946486E-02;  A(5) = 9.1836734693878E+00
	B(6) = 0;	A(6) = 7.7142857142857E+00
	B(7) = 1.9259308298786E-03;  A(7) = 4.0000000000000E+00
	B(8) = 0;	A(8) = 1.0000000000000E+00
end;



