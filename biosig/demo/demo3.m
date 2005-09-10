% Demostration for generating EDF/BDF/GDF-files
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
%   i686-pc-cygwin	2.1.42		OK
%   i586-pc-linux-gnu   2.1.40		OK

%	$Revision: 1.6 $
%	$Id: demo3.m,v 1.6 2005-09-10 20:44:37 schloegl Exp $
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

x = randn(10000,5)+(1:1e4)'*ones(1,5); % test data
x = (1:1e4)'*ones(1,5)/1000; % test data
x = reshape(mod(1:5e4,100),5,1e4)';
clear HDR;

VER   = version;
cname = computer;

% select file format 
HDR.TYPE='GDF';
HDR.TYPE='EDF';
%HDR.TYPE='BDF'; 

% set Filename
HDR.FileName = ['TEST_',VER([1,3]),cname(1:3),'.',HDR.TYPE];

% person identification, max 80 char
HDR.Patient.ID = 'P0000';	
HDR.Patient.Sex = 'F';
HDR.Patient.Birthday = [1951 05 13 0 0 0];
HDR.Patient.Name = 'X';		% for privacy protection  
HDR.Patient.Handedness = 0; 	% unknown, 1:left, 2:right, 3: equal


% recording identification, max 80 char.
HDR.RID = 'recording identification';

% recording time [YYYY MM DD hh mm ss.ccc]
HDR.T0 = clock;	

% number of channels
HDR.NS = size(x,2);

% Duration of one block in seconds
HDR.Dur = 1;

% Samples within 1 block
%HDR.SPR = [100;100;100;100;100];	% samples per block;
HDR.EDF.SampleRate = [100;100;100;100;100];	% samples per block;

% channel identification, max 80 char. per channel
HDR.Label=['chan 1  ';'chan 2  ';'chan 3  ';'chan 4  ';'chan 5  '];

% Transducer, mx 80 char per channel
HDR.Transducer = ['Ag-AgCl ';'Airflow ';'xyz     ';'        ';'        '];

% define datatypes (GDF only, see GDFDATATYPE.M for more details)
HDR.GDFTYP = 3*ones(1,HDR.NS);

% define scaling factors 
HDR.PhysMax = [100;100;100;100;100];
HDR.PhysMin = [0;0;0;0;0];
HDR.DigMax  = [100;100;100;100;100];
HDR.DigMin  = [0;0;0;0;0];
HDR.Filter.Lowpass = [0,0,0,NaN,NaN];
HDR.Filter.Highpass = [100,100,100,NaN,NaN];
HDR.Filter.Notch = [0,0,0,0,0];


% define physical dimension
HDR.PhysDim = ['uV ';'   ';'   ';'   ';'   '];

t = [100:100:size(x,1)]';
HDR = sopen(HDR,'wb');
%HDR.SIE.RAW = 0; % [default] channel data mode, one column is one channel 
%HDR.SIE.RAW = 1; % switch to raw data mode, i.e. one column for one EDF-record
HDR = swrite(HDR,x);

HDR.EVENT.POS = t;
HDR.EVENT.TYP = t/100;
HDR = sclose(HDR);

[s0,HDR0] = sload(HDR.FileName);	% test file 

plot(s0-x)


