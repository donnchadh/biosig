function [HDR]=eegseek(HDR,offset,origin)
% EEGSEEK repositions file position indicator
% [HDR]=eegseek(HDR,offset,origin)
%
% The meaning of "offset" and "origin" differs for different formats. 
% 	EDF: number of (EDF) records. 
%	BKR: number of samplepoints	
%
% SSEEK replaces EEGSEEK. 
%
% See also: FSEEK, EEGREAD, EEGWRITE, EEGCLOSE, EEGREWIND, EEGTELL, EEGEOF

%	$Revision: 1.12 $
%	$Id: eegseek.m,v 1.12 2003-09-06 18:31:08 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	

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


HDR = sseek(HDR,offset,origin);

