function [IFF]=openiff(fid,LEN)
% OPENIFF is an auxillary function to SOPEN for 
% opening of IFF files 
% 
% Use SOPEN instead of OPENIFF  
% 
% See also: fopen, SOPEN, 
%
% References: 
% [1] 
% [2] 

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

%	$Revision: 1.1 $
%	$Id: openiff.m,v 1.1 2004-11-18 16:03:55 schloegl Exp $
%	(C) 2004 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


fprintf(2,'Warning: OPENIFF is in an experimental state and is most likely not useful to you.\n'); 
fprintf(2,'\t Do not use it unless you are sure know what you do. At least you are warned!\n');


if nargin<2,
        LEN = 8;
end;		

IFF = [];
c   = 1; 
K   = 0; 
K1  = 0; 

COUNT = 0; 
[tmp,c] = fread(fid,[1,4],'char');
while ((LEN>0) | isnan(LEN)) & (c>0),	
        tag     = char(tmp);
        tagsize = fread(fid,1,'uint32');        % which size 
        tagsize0= tagsize + rem(tagsize,2); 
        filepos = ftell(fid);
        LEN = LEN - 8; 
        
        if 0, 
                VAL = openiff(fid,tagsize);
        elseif strcmp(tag,'FORM')
                [tmp,c] = fread(fid,[1,4],'char');
                VAL = setfield(VAL,char(tmp),openiff(fid,tagsize-4));
        elseif strcmp(tag,'RIFF')
                [tmp,c] = fread(fid,[1,4],'char');
                VAL = setfield(VAL,char(tmp),openiff(fid,tagsize-4));
        elseif strcmp(tag,'MThd')
                VAL.MThd = fread(fid,tagsize,'uchar');
                VAL.MIDI = openiff(fid,NaN);
                %LEN = NaN;
                %VAL.MIDI = tmp;
        elseif strcmp(tag,'LIST')
                [tmp,c] = fread(fid,[1,4],'char'); char(tmp),
                VAL = setfield(VAL,char(tmp),openiff(fid,tagsize-4));
        elseif strcmp(tag,'LIST')
                VAL = openiff(fid,tagsize);
        elseif strcmp(tag,'hdrl')
                tagsize,
                [tmp,c] = fread(fid,[1,tagsize],'char')
                VAL = openiff(fid,tagsize-4);
        elseif strcmp(tag,'CAT ')
                VAL = openiff(fid,tagsize);
        elseif strncmp(tag,'(c)',3)
                tag = 'Copyright';
                VAL = char(fread(fid,tagsize,'uchar')');
                %VAL.CopyRight = char(VAL);
        else		
                VAL = fread(fid,tagsize,'uchar');
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
                IFF = setfield(IFF,tag,VAL);
        end;
        status = fseek(fid,filepos+tagsize0,'bof');
        LEN = LEN - tagsize0;
        [tmp,c] = fread(fid,[1,4],'char');
end;

if ~isfield(IFF,'MThd'), % do not check MIDI files
        if (LEN ~= 0) & (c), 
                fprintf(2,'Warning OPENIFF: LEN=%i %i %i %i %s  %i\n',LEN,filepos,tagsize0,ftell(fid),char(tmp),c);
        end;	
end;

