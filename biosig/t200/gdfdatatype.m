function datatyp=gdfdatatype(x)
% GDFDATATYPE converts number into data type according to the definition of the GDF format [1]. 
%
% See also: SDFREAD, SDFWRITE, SDFCLOSE
%
% References:
% A. Schlögl, O. Filz, H. Ramoser, G. Pfurtscheller, GDF - A general dataformat for biosignals, Technical Report, 1999.
% http://www.dpmi.tu-graz.ac.at/~schloegl/matlab/eeg/gdf4/tr_gdf.ps


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


%
%       Version 0.90,   03 Jan 2003
% 	(C) 1997-2003 Alois Schloegl 
%	<a.schloegl@ieee.org>  



k=1;
EDF.GDFTYP(1)=x;
if EDF.GDFTYP(k)==0
        datatyp=('uchar');
elseif EDF.GDFTYP(k)==1
        datatyp=('int8');
elseif EDF.GDFTYP(k)==2
        datatyp=('uint8');
elseif EDF.GDFTYP(k)==3
        datatyp=('int16');
elseif EDF.GDFTYP(k)==4
        datatyp=('uint16');
elseif EDF.GDFTYP(k)==5
        datatyp=('int32');
elseif EDF.GDFTYP(k)==6
        datatyp=('uint32');
elseif EDF.GDFTYP(k)==7
        datatyp=('int64');
elseif 0; EDF.GDFTYP(k)==8
        datatyp=('uint64');
elseif EDF.GDFTYP(k)==16
        datatyp=('float32');
elseif EDF.GDFTYP(k)==17
        datatyp=('float64');
elseif 0;EDF.GDFTYP(k)>255 & EDF.GDFTYP(k)< 256+64
        datatyp=(['bit' int2str(EDF.GDFTYP(k))]);
elseif 0;EDF.GDFTYP(k)>511 & EDF.GDFTYP(k)< 511+64
        datatyp=(['ubit' int2str(EDF.GDFTYP(k))]);
elseif EDF.GDFTYP(k)==256
        datatyp=('bit1');
elseif EDF.GDFTYP(k)==512
        datatyp=('ubit1');
elseif EDF.GDFTYP(k)==255+12
        datatyp=('bit12');
elseif EDF.GDFTYP(k)==511+12
        datatyp=('ubit12');
elseif EDF.GDFTYP(k)==255+22
        datatyp=('bit22');
elseif EDF.GDFTYP(k)==511+22
        datatyp=('ubit22');
elseif EDF.GDFTYP(k)==255+24
        datatyp=('bit24');
elseif EDF.GDFTYP(k)==511+24
        datatyp=('ubit24');
else 
        fprintf(2,'Error GDFREAD: Invalid GDF channel type\n');
        datatyp='';
end;
