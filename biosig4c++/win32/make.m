% Matlab script to generate mexSLOAD.mexw32/mexSOPEN.mexw32 
% 
% Requirements:
%   Windows  
%   Matlab for Windows v7.1 or later  
%   MinGW with gcc 4.x (tested with MinGWtdm)
%   gnumex available frome here http://gnumex.sourceforge.net/
%   zlib 1.2.3.x 
%   win32/libbiosig[.a/.lib] 
%     either from 
%       the pre-compiled binary from biosig4c++/win32/ 
%     or  
%       Windows
%       MinGW
%       mingw32-make -f Makefile.win32 libbiosig.a
%     or  
%       Linux
%       mingw32
%       make
%       make win32/libbiosig.lib

%	$Id$
%	Copyright (C) 2009,2010 by Alois Schloegl <a.schloegl@ieee.org>
%	This is part of the BIOSIG-toolbox http://biosig.sf.net/

% LICENSE:
%     This program is free software; you can redistribute it and/or modify
%     it under the terms of the GNU General Public License as published by
%     the Free Software Foundation; either version 3 of the License, or
%     (at your option) any later version.
% 
%     This program is distributed in the hope that it will be useful,
%     but WITHOUT ANY WARRANTY; without even the implied warranty of
%     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%     GNU General Public License for more details.
% 
%     You should have received a copy of the GNU General Public License
%     along with this program; if not, write to the Free Software
%     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
% 

mex ..\mexSLOAD.cpp -l ..\win32\libbiosig.lib -l ..\win32\zlib\lib\zdll.lib -l C:\MinGW\lib\libws2_32.a

mex -D=mexSOPEN  ..\mexSLOAD.cpp -l ..\win32\libbiosig.lib -l ..\win32\zlib\lib\zdll.lib  -l C:\MinGW\lib\libws2_32.a -output mexSOPEN


 
