function [S,HDR] = eegread(HDR,NoS,StartPos)
% Loads selected seconds of an EEG File
%
% [S,EEG] = eegread(EEG [,NoS [,StartPos]] )
% NoS       Number of seconds, default = 1 (second)
% StartPos  Starting position, if not provided the following data is read continously from the EDF file. 
%                    no reposition of file pointer is performed
%
% EEG=eegopen(Filename,'r',CHAN);
% [S,EEG] = eegread(EEG, NoS, StartPos)
%      	reads NoS seconds beginning at StartPos
% 
% [S,EEG] = eegread(EEG, inf) 
%      	reads til the end starting at the current position 
% 
% [S,EEG] = eegread(BKR, N*BKR.Dur) 
%	reads N trials of an BKR file 
% 
%
% SREAD replaces EEGREAD. 
%
% See also: fread, EEGREAD, EEGWRITE, EEGCLOSE, EEGSEEK, EEGREWIND, EEGTELL, EEGEOF

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


%	$Revision: 1.21 $
%	$Id: eegread.m,v 1.21 2003-09-06 18:31:07 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	

if nargin<2, 
	NoS = inf; 
end;

if nargin<3, 
	[S,HDR] = sread(HDR,NoS);
else
	[S,HDR] = sread(HDR,NoS,StartPos);
end;



