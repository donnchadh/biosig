function [HDR]=fefopen(HDR)
% HDR=fefopen(HDR)
% Loads one CHANNEL of a CEN/TC251 File  into MATLAB
% <A HREF="ftp://sigftp.cs.tut.fi/pub/eeg-data/standards/cenf060.zip ">About CEN/TC251</A> 
%
% This Matlab function is Under Construction and Testing 
% If anybody is interested in this Converter and/or would like to programm it, contact me.

%	$Revision: 1.1 $
%	$Id: fefopen.m,v 1.1 2004-06-29 18:14:41 schloegl Exp $
%	Copyright (c) 1998, 2004  Alois Schloegl
%	a.schloegl@ieee.org	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

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

TYP_OID16 = 'int16';

tag=fread(HDR.FILE.FID,1,TYP_OID16);
len=fread(HDR.FILE.FID,1,'ubit16');
while ~feof(HDR.FILE.FID),
        value=fread(HDR.FILE.FID,len,'char');
        [tag,len],
        value,
        
        tag=fread(HDR.FILE.FID,1,TYP_OID16);
        len=fread(HDR.FILE.FID,1,'ubit16');
end;
        
return;        


TYP_OID16='int16';
tmp=str2num(LFL);
if tmp==0,
        TYP_LENGTH='uint8';
elseif tmp==1,
        TYP_LENGTH='uint16';
elseif tmp==2,
        TYP_LENGTH='uint32';
elseif tmp==3,
        TYP_LENGTH='uint64';
elseif tmp==4,
        %	case 4: TYP_LENGTH='uint8';
end;

%%%%% (2) Demographic Section %%%%%
DS_begin=ftell(fid,0,'bof');
DS_ID=fread(fid,1,'int16')
DS_Length=fread(fid,1,TYP_LENGTH);

fseek(fid,DS_begin+DS_Length,'bof');

%%%%% (3) Medical Device System Presentation Section %%%%%
MDSP_begin=ftell(fid,0,'bof');
MDSP_ID=fread(fid,1,'int16')
MDSP_Length=fread(fid,1,TYP_LENGTH);

fseek(fid,MDSP_begin+MDSP_Length,'bof');

%%%%% (4) Manufacture Specific Section %%%%%
MS_begin=ftell(fid,0,'bof');
MS_ID=fread(fid,1,'int16')
MS_Length=fread(fid,1,TYP_LENGTH);

fseek(fid,MS_begin+MS_Length,'bof');

%%%%% (5) Image Section %%%%%
IS_begin=ftell(fid,0,'bof');
IS_ID=fread(fid,1,'int16')
IS_Length=fread(fid,1,TYP_LENGTH);

fseek(fid,IS_begin+IS_Length,'bof');

%%%%% (6) Archive & Measurement Section %%%%%
SA_begin=ftell(fid,0,'bof');
OIDSA_ID=fread(fid,1,'int16')
O_Length=fread(fid,1,TYP_LENGTH);


fseek(fid,SA_begin+SA_Length,'bof');

%%%%% (6.1) Test Sub-Section %%%%%


fclose(fid);

return;

function BLOCK=loadsec(fid);
% assuming filepointer points to Blockbegin
% at the end points fp to the next block begin

BLOCK.begin=ftell(fid,0,'bof');
BLOCK.ID=fread(fid,1,'int16')
BLOCK.Length=fread(fid,1,TYP_LENGTH);

fseek(fid,BLOCK.begin+BLOCK.Length,'bof');
%BLOCK.Next=BLOCK.begin+BLOCK.Length;




