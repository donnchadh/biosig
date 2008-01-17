%% Copyright (C) 2008 Alois Schloegl 
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

%% isfield(x,f)
%%    true if f is a field of struct x  

function t = isstruct(x,g);
	f = fieldnames(x);
	lf= length(f); 
	t = 0; 
	k = 0;
	while (~t & k<lf) 
		k = k+1;
		if all(size(f{k})==size(g))
			t = all(f{k}==g);
		end;	
	end

