% Demostration for generating GDF-files
% DEMO3 is part of the biosig-toolbox
%    and it tests also Matlab/Octave for its correctness. 
% 
%  computer	version 		test
%            MATLAB
%   PCWIN	5.3.0.10183 (R11)	OK
%   PCWIN	6.5.0.180913a (R13)	OK
%   LNX86	5.3.0.10183 (R11)	OK
%   LNX86	6.5.0.180913a (R13)	OK
% 	     OCTAVE
%   i686-pc-cygwin	2.1.42		failed ## fwrite extends 0x0a to 0x0d 0x0a
%

%	$Revision: 1.2 $
%	$Id: demo3.m,v 1.2 2003-07-18 15:29:57 schloegl Exp $
%	Copyright (C) 2000-2003 by Alois Schloegl <a.schloegl@ieee.org>	

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

x=randn(10000,5)+(1:1e4)'*ones(1,5); % test data
x=(1:1e4)'*ones(1,5); % test data
clear GDF;

VER=version;
cname=computer;
GDF.TYPE='GDF';GDF.FileName = ['TEST_',VER([1,3]),cname(1:3),'.GDF'];

%GDF.TYPE='EDF';GDF.FileName = ['TEST_',VER([1,3]),cname(1:3),'.EDF'];

%GDF.TYPE='BDF'; GDF.FileName = ['TEST_',VER([1,3]),cname(1:3),'.BDF'];

GDF.PID = 'person identification';	% e.g. 'MCH-0234567 F 02-MAY-1951 Haagse_Harry'
GDF.RID = 'recording identification';	% e.g. 'EMG561 BK/JOP Sony. MNC R Median Nerve.'
GDF.Dur = 1;
GDF.SPR = 100;
GDF.NS = size(x,2);	% number of channels
GDF.GDFTYP = 16*ones(1,GDF.NS);
GDF.SPR = [100;100;100;100;100];	% samples per block;
GDF.Label=['chan 1  ';'chan 2  ';'chan 3  ';'chan 4  ';'chan 5  '];
GDF.Transducer = [' ';' ';' ';' ';' '];
GDF.PreFilt = [' ';' ';' ';' ';' '];		% Prefiltering

GDF.PhysMax = [100;100;100;100;100];
GDF.PhysMin = [0;0;0;0;0];
GDF.DigMax = [100;100;100;100;100];
GDF.DigMin = [0;0;0;0;0];
GDF.PhysDim = [' ';' ';' ';' ';' '];
GDF.T0 = [0,0,0,0,0,0];	% recording time [YYYY MM DD hh mm ss.ccc]
GDF=gdfcheck(GDF);

GDF=eegopen(GDF,'w');
%GDF.SIE.RAW = 0; % [default] channel data mode, one column is one channel 
%GDF.SIE.RAW = 1; % switch to raw data mode, i.e. one column for one EDF-record

GDF=eegwrite(GDF,x);

GDF=eegclose(GDF);

[s0,GDF0]=loadeeg(GDF.FileName);	% test file 
plot(s0-x)
