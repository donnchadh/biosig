function [signal,H] = loadeeg(FILENAME,CHAN,TYPE)
% LOADEEG loads EEG data of various data formats
% 
% Currently are the following data formats supported: 
%    EDF, CNT, EEG, BDF, GDF, BKR, MAT(*), 
%    PhysioNet (MIT-ECG), Poly5/TMS32, SMA, RDF 
%
% [signal,header] = loadeeg(FILENAME [,CHANNEL[,TYPE]])
%
% FILENAME      name of file
% channel       list of selected channels
%               default=0: loads all channels
% TYPE (optional) forces a certain dataformat
%
% SLOAD replaces LOADEEG. 
%
% see also: EEGOPEN, EEGREAD, EEGCLOSE
%

%	$Revision: 1.17 $
%	$Id: loadeeg.m,v 1.17 2003-09-06 18:31:07 schloegl Exp $
%	Copyright (C) 1997-2003 by Alois Schloegl 
%	a.schloegl@ieee.org	

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


if nargin<2; CHAN=0; end;

[signal,H] = sload(FILENAME,CHAN);


