%% Copyright (C) 2008 Alois Schloegl
%% $Id: lower.m,v 1.1 2008-01-17 16:28:26 schloegl Exp $
%% This function is part of BioSig http://biosig.sf.net 
%% This function is needed, if your system does not provide it (e.g. FreeMat v3.5)
%%
%% This program is free software; you can redistribute it and/or modify
%% it under the terms of the GNU General Public License as published by
%% the Free Software Foundation; either version 2 of the License, or
%% (at your option) any later version.
%%
%% This program is distributed in the hope that it will be useful,
%% but WITHOUT ANY WARRANTY; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%% GNU General Public License for more details.
%%
%% You should have received a copy of the GNU General Public License
%% along with this program; if not, write to the Free Software
%% Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

%% lower(x)
%%    converts string in lower case characters 
%% 
%% lower, needed for FreeMat 3.5

function x = lower(x);
	ix = (x >= 'A') && (x <= 'Z');
	x(ix) = x(ix)+32; 
	

