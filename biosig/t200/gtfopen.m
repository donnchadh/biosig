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

%	$Id: gtfopen.m,v 1.2 2005-03-31 18:00:40 schloegl Exp $
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

H.i8  = fread(HDR.FILE.FID,inf,'int8');
fclose(HDR.FILE.FID);

HDR.NS   = 24; 
HDR.Dur  = 10; 
HDR.SampleRate = 256; 
HDR.SPR  = 2560; 
HDR.bits = 8; 

HDR.SPR = floor((size(H.i8,1))/HDR.NS);
HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);

ix = find(H.i8==-128);

t=ix(find(H.i8(ix+3)==5));
% 1920 = 10x192 = 24*80 
% 61760-192 = 61568
t2 = t(find(diff(t)>1e4));
t2 = [t2(:); t2(end)+63488];
% 63488 = 61760 + 9*192

HDR.NRec = length(t2);
[s3,sz]  = trigg(H.i8,t2,125,61564);
HDR.data = reshape(s3,[HDR.NS,sz(2)/HDR.NS*HDR.NRec])';

HDR.TYPE = 'native'; 
HDR.THRESHOLD = repmat([-127,127],HDR.NS,1);
HDR.FILE.POS = 0; 

