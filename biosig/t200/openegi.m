function [HDR] = openegi(arg1,PERMISSION,CHAN,arg4,arg5,arg6)
% EGIOPEN opens EGI files
% HDR=openegi(Filename, PERMISSION, [, ChanList ]);
%
% FILENAME 
% PERMISSION is one of the following strings 
%	'r'	read 
% ChanList	(List of) Channel(s)
%		default=0: loads all channels

%	Version 1.96  Date: 03 Jan 2003
%	CopyLeft (C) 1997-2003 by  Alois Schloegl
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

if nargin<2, PERMISSION='r'; end;
if nargin<3, CHAN=0; end;

if isstruct(arg1),
	HDR=arg1;
	if HDR.FILE.OPEN,
		fseek(HDR.FILE.FID,0,'bof');	
	else
		HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-be');          
		HDR.FILE.OPEN= 1;
	end;
else
	HDR.FileName = arg1;
	HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-be');          
	if HDR.FILE.FID<0,
		fprintf(2,'Error EGIOPEN: file %s not found.\n',HDR.FileName); 
		return;
	end;
	HDR.FILE.OPEN = 1;
end;



% Output:
%   HDR- structure containing header information.
%          Structure fields are:
%           VERSION     = 2,3,4,5,6,or 7
%           SampleRate  = sampling rate in samples/s
%           NS          = number of EEG channels
%           gain        = gain of amplifier
%           bits        = number of bits/sample
%           PhysMax     = abs(max. value)
%           NRec	= number of epochs
%           categories  = number of categories
%           catname     = cell array of category names
%           SPR    	= number of samples/segment
%           eventtypes  = number of event types
%           eventcode   = string array of event codes
%



HDR.VERSION = fread(fid,1,'integer*4');

if ~( HDR.VERSION >= 2 & HDR.VERSION <= 7 ),
        error('EGI Simple Binary Versions 2-7 supported only.');
end;
% do we have segmented data?
HDR.FLAG.TRIGGERED = any(HDR.VERSION==[3,5,7]);

HDR.T0 = fread(fid,6,'integer*2');
millisecond = fread(fid,1,'integer*4');
HDR.T0(6) = HDR.T0(6) + millisecond/1000;

HDR.SampleRate = fread(fid,1,'integer*2');
HDR.NS = fread(fid,1,'integer*2');
HDR.gain = fread(fid,1,'integer*2');
HDR.bits = fread(fid,1,'integer*2');
HDR.DigMax  = 2^HDR.bits;
HDR.PhysMax = fread(fid,1,'integer*2');

HDR.samples    = 0;
HDR.NRec       = 0;
HDR.SPR        = 0;
HDR.eventtypes = 0;
HDR.categories = 0;
HDR.catname    = {};
HDR.eventcode  = '';

if CHAN<1, CHAN=1:HDR.NS; end;
HDR.SIE.ChanSelect = CHAN;
HDR.SIE.InChanSelect = CHAN;

if any(HDR.VERSION==[2,4,6]),
   	HDR.samples = fread(fid, 1 ,'integer*4');
elseif any(HDR.VERSION==[3,5,7]),
	HDR.categories = fread(fid,1,'integer*2');
        if (HDR.categories),
               	for i=1:HDR.categories,
                       	catname_len(i) = fread(fid,1,'uchar');
                       	HDR.catname{i} = char(fread(fid,catname_len(i),'uchar'))';
		end
        end
        HDR.NRec = fread(fid,1,'integer*2');
	HDR.SPR  = fread(fid,1,'integer*4');
else
        error('Invalid EGI version');
end

% get datatype from version number
if any(HDR.VERSION==[2,3]),
	HDR.datatype = 'integer*2';
elseif any(HDR.VERSION==[4,5]),
	HDR.datatype = 'float32';
elseif any(HDR.VERSION==[6,7]),
	HDR.datatype = 'float64';
else
	error('Unknown data format');
end

HDR.eventtypes = fread(fid,1,'integer*2');

if isequal(HDR.eventtypes,0),
	HDR.eventcode(1,1:4) = 'none';
else
        for i = 1:HDR.eventtypes,
                HDR.eventcode(i,1:4) = fread(fid,[1,4],'uchar');
        end
end
