function [HDR]=gtfopen(HDR,PERMISSION,arg3,arg4,arg5,arg6)
% GTFOPEN reads files in the Galileo Transfer format (GTF) 
%
% HDR = gtfopen(Filename,PERMISSION);
%
% HDR contains the Headerinformation and internal data
%
% see also: SOPEN, SREAD, SSEEK, STELL, SCLOSE, SWRITE, SEOF


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

%	$Id: gtfopen.m,v 1.1 2005-03-30 14:23:41 schloegl Exp $
%	(C) 2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


if nargin<1, PERMISSION='rb'; end;
if ischar(HDR)
        tmp=HDR;
        HDR=[];
        HDR.FileName=tmp;
end;


fprintf(2,'Warning GTFOPEN (%s): implementation of GTF-support is not ready for production use.\n',HDR.FileName);

HDR.FILE.FID = fopen(HDR.FileName,'r','ieee-le');

% read 3 header blocks 
HDR.H1 = fread(HDR.FILE.FID,512,'uint8');
HDR.H2 = fread(HDR.FILE.FID,15306,'int8');
HDR.H3 = fread(HDR.FILE.FID,8146,'uint8');           

HDR.L1 = char(reshape(HDR.H3(1:650),65,10)');                   
HDR.L2 = char(reshape(HDR.H3(650+(1:20*16)),16,20)');
HDR.L3 = reshape(HDR.H3(1070+32*3+(1:232*20)),232,20)';

HDR.Label = char(reshape(HDR.H3(1071:1070+32*3),3,32)');        % channel labels

HDR.data  = fread(HDR.FILE.FID,inf,'int16');
HDR.NS    = 24; 
HDR.Dur   = 5; 
HDR.SampleRate = 256; 
HDR.AS.spb = 1280 + 408; 

HDR.SPR = floor((size(HDR.data,1))/HDR.NS);
HDR.d1  = reshape(HDR.data(1:HDR.NS*HDR.SPR),[HDR.NS,HDR.SPR])'; 
fclose(HDR.FILE.FID);

HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);

        