function [datatyp,limits,datatypes]=gdfdatatype(GDFTYP)
% GDFDATATYPE converts number into data type according to the definition of the GDF format [1]. 
%
%   [datatyp,limits,datatypes]=gdfdatatype(GDFTYP)
%
%
% See also: SDFREAD, SREAD, SWRITE, SCLOSE
%
% References:
% [1] A. Schlögl, O. Filz, H. Ramoser, G. Pfurtscheller, GDF - A general dataformat for biosignals, Technical Report, 2004.
% available online at: http://www.dpmi.tu-graz.ac.at/~schloegl/matlab/eeg/gdf4/TR_GDF.pdf. 

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

%	$Revision: 1.8 $
%	$Id: gdfdatatype.m,v 1.8 2005-03-25 16:27:21 schloegl Exp $
%	(C) 1997-2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


if ischar(GDFTYP),
        datatyp=GDFTYP;
        return; 
end;

datatyp = [];
datatypes = {};
limits = repmat(NaN,length(GDFTYP),3);

for k=1:length(GDFTYP),
        if GDFTYP(k)==0
                datatyp=('uchar');
                limit = [0,256,256];        
        elseif GDFTYP(k)==1
                datatyp=('int8');
                limit = [-128,127,128];        
        elseif GDFTYP(k)==2
                datatyp=('uint8');
                limit = [0,256,256];        
        elseif GDFTYP(k)==3
                datatyp=('int16');
                limit = [-2^15,2^15-1,-2^15];        
        elseif GDFTYP(k)==4
                datatyp=('uint16');
                limit = [0,2^16,2^16];        
        elseif GDFTYP(k)==5
                datatyp=('int32');
                limit = [-2^32,2^32-1,-2^32];        
        elseif GDFTYP(k)==6
                datatyp=('uint32');
                limit = [0,2^32,2^32];        
        elseif GDFTYP(k)==7
                datatyp=('int64');
                limit = [-2^63,2^63-1,-2^63];        
        elseif GDFTYP(k)==8
                datatyp=('uint64');
                limit = [0,2^64,2^64];        
        elseif GDFTYP(k)==16
                datatyp=('float32');
                limit = [-inf,inf,NaN];        
        elseif GDFTYP(k)==17
                datatyp=('float64');
                limit = [-inf,inf,NaN];        
        elseif GDFTYP(k)==18
                datatyp=('float128');
                limit = [-inf,inf,NaN];        
        elseif (GDFTYP(k)>255) & (GDFTYP(k)<512)
                nbits = GDFTYP(k)-255;
                datatyp=['bit',int2str(nbits)];
                limit = [-(2^(nbits-1)),2^(nbits-1)-1,-(2^(nbits-1))];
        elseif (GDFTYP(k)>511) & (GDFTYP(k)<768)
                nbits = GDFTYP(k)-511;
                datatyp=['ubit',int2str(nbits)];
                limit = [0,2^nbits,2^nbits];
        else 
                fprintf(2,'Error GDFREAD: Invalid GDF channel type\n');
                datatyp='';
                limit = [NaN,NaN,NaN];

        end;
        datatypes{k} = datatyp;
        limits(k,:)  = limit;
end;