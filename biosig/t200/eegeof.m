function [status]=eegeof(HDR)
% EEGEOF checks for end of EEG-file
%    status=eegeof(HDR)
%
% returns 1 if End-of-EDF-File is reached
% returns 0 otherwise
%
% SEOF replaces EEGEOF. 

% See also: feof, EEGREAD, EEGWRITE, EEGCLOSE, EEGSEEK, EEGREWIND, EEGTELL, EEGEOF

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the  License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

%	$Revision: 1.9 $
%	$Id: eegeof.m,v 1.9 2003-09-06 18:31:07 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


status = seof(HDR); 

