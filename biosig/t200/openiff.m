function [IFF]=openiff(fid,LEN)
% OPENIFF is an auxillary function to SOPEN for 
% opening of IFF files 
% 
% Use SOPEN instead of OPENIFF  
% 
% See also: fopen, SOPEN
%
% References: 

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

%	$Id: openiff.m,v 1.3 2005-11-05 20:46:17 schloegl Exp $
%	Copyright (C) 2004,2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


FLAG.CLOSE = 0; 
if ischar(fid)
        fid = fopen(fid,'r');
        FLAG.CLOSE = 1; 
end;

if nargin<2,
        LEN = 8;
end;		

IFF = [];
K   = 0; 
K1  = 0; 

[tmp,c] = fread(fid,[1,4],'char');
while ((LEN>0) | isnan(LEN)) & (c>0),	
        tag     = char(tmp);
        tagsize = fread(fid,1,'uint32');        % which size 
        tagsize0= tagsize + rem(tagsize,2); 
        filepos = ftell(fid);
        LEN = LEN - 8; 
        
%       fprintf(1,'tag: %6s\tpos: %8i\tsize: %8i\n',tag,filepos,tagsize);
        
        if 0, 
                VAL = openiff(fid,tagsize);
        elseif strcmp(tag,'FORM')
                [tmp,c] = fread(fid,[1,4],'char');tag,
                VAL = setfield([],char(tmp),openiff(fid,tagsize-4));
        elseif strcmp(tag,'RIFF')
                [tmp,c] = fread(fid,[1,4],'char');
                %val = fread(fid,tagsize-4,'char');
                val = openiff(fid,tagsize-4);
                VAL = setfield([],char(tmp),val);
        elseif strcmp(tag,'MThd')
                VAL.MThd = fread(fid,tagsize,'uchar');
                VAL.MIDI = openiff(fid,NaN);
                %LEN = NaN;
                %VAL.MIDI = tmp;
        elseif strcmp(tag,'LIST')
                [tmp,c] = fread(fid,[1,4],'char'); 
                VAL = setfield([],char(tmp),openiff(fid,tagsize-4));
        elseif strcmp(tag,'LIST')
                VAL = openiff(fid,tagsize);
        elseif strcmp(tag,'chan')
                VAL = fread(fid,[1,tagsize/2],'uint16');
        elseif strncmp(tag,'ep  ',4),
                VAL = fread(fid,[1,tagsize/4],'uint32');
        elseif strcmp(tag,'hdrl')
                [tmp,c] = fread(fid,[1,4],'char');
                VAL = setfield([],char(tmp),openiff(fid,tagsize-4));
        elseif strcmp(tag,'CAT ')
                VAL = openiff(fid,tagsize);
        elseif strncmp(tag,'(c)',3)
                tag = 'Copyright';
                VAL = char(fread(fid,tagsize,'uchar')');
                %VAL.CopyRight = char(VAL);
        else
                if 1, %tagsize<1024*2,
                        VAL = fread(fid,tagsize,'uchar');
                else
			VAL = [];
                        VAL.handle = ftell(fid);
                        VAL.size = tagsize; 
                        status = fseek(fid,tagsize,'cof');
                end
        end;
	        
        if strcmp(tag(3:4),'dc')
                K = K+1;
                ix = (tag(1:2)-48)*[16;1]+1;
                IFF.dc{K,ix} = VAL;
        elseif strcmp(tag(3:4),'wb')
                K1 = K1+1;
                ix = (tag(1:2)-48)*[16;1]+1;
                IFF.wb{K1,ix} = VAL;
        else
                try,
	                IFF = setfield(IFF,tag,VAL);
                catch,
                end;
        end;
        status = fseek(fid,filepos+tagsize0,'bof');
        LEN = LEN - tagsize0;
        [tmp,c] = fread(fid,[1,4],'char');
end;

if ~isfield(IFF,'MThd'), % do not check MIDI files
        if ((LEN ~= 0) & c), 
                fprintf(2,'Warning OPENIFF: LEN=%i %i %i %i %s  %i\n',LEN,filepos,tagsize0,ftell(fid),char(tmp),c);
        end;	
end;

if FLAG.CLOSE,
        fclose(fid);
end;
