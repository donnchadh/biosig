function [HDR] = ssave(FILENAME,DATA,TYPE,Fs,bits)
% SSAVE saves signal data in various data formats
% 
% Currently are the following data formats supported: 
%    EDF, BDF, GDF, BKR, SND/AU, (WAV, AIF)
%
% HDR = ssave(FILENAME,data,TYPE,Fs);
%
% FILENAME      name of file
% data  signal data, each column is a channel
% TYPE 	determines dataformat
% Fs	sampling rate	
%
% see also: SSAVE, SOPEN, SWRITE, SCLOSE
%

%	$Revision: 1.1 $
%	$Id: ssave.m,v 1.1 2003-09-09 23:14:46 schloegl Exp $
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



if isstruct(FILENAME),
        HDR=FILENAME;
        if isfield(HDR,'FileName'),
                FILENAME=HDR.FileName;
        else
                fprintf(2,'Error LOADEEG: missing FileName.\n');	
                return; 
        end;
end;

HDR.FileName = FILENAME;
HDR.TYPE = TYPE; 	% type of data format
HDR.SampleRate = Fs; 
HDR.bits = bits;

[HDR.SPR,HDR.NS]   = size(DATA);


HDR = sopen(HDR,'wb');

HDR = swrite(HDR,DATA);

HDR = sclose(HDR);

