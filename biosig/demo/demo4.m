% Demo for Writing BKR files %
% DEMO4 is part of the biosig-toolbox
%     it demonstrates generating BKR files 
%     and contains a few tests 
% 

%	$Revision: 1.1 $
%	$Id: demo4.m,v 1.1 2003-07-31 12:16:57 schloegl Exp $
%	Copyright (C) 2003 by Alois Schloegl <a.schloegl@ieee.org>	

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



F{1}='test1.bkr';
F{2}='test2.bkr';
s = randn(1000,5);	% Generate Test data

HDR.TYPE='BKR';		% Define file format
HDR.FileName = F{1};	% Assign Filename
HDR.NS = size(s,2);	% number of channels
HDR.SampleRate = 100;	% Sampling rate
HDR.NRec = 1;		% number of trials (1 for continous data)
HDR.PhysMax = max(abs(s(:)));	% Physical maximum 
HDR.DigMax  = max(2^15-1);	% Digital  maximum
HDR.Filter.LowPass  = 30;	% upper cutoff frequency
HDR.Filter.HighPass = .5;	% lower cutoff frequency 

% 1st way to generate BKR-file
HDR.FileName = F{1};	% Assign Filename
HDR = eegopen(HDR,'w'); 	% OPEN BKR FILE
dig_values = s'*HDR.DigMax/HDR.PhysMax; 	% digital values without scaling, each row is a channel.
fwrite(HDR.FILE.FID,dig_values,'int16'); 
HDR = eegclose(HDR);            % CLOSE BKR FILE

% 2nd way to generate BKR-file
% test 'w'
HDR.FileName = F{2};	% Assign Filename
HDR = eegopen(HDR,'w'); 	% OPEN BKR FILE
HDR = eegwrite(HDR,s);  	% WRITE BKR FILE
HDR = eegclose(HDR);            % CLOSE BKR FILE


% read file 
HDR = eegopen(HDR,'r'); 	% OPEN BKR FILE
[s0,HDR] = eegread(HDR);  	% WRITE BKR FILE
HDR = eegclose(HDR);





 