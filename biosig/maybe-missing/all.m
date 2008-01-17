%% all(x [,DIM])
%%     The function `all' behaves like the function `any', except that it
%%     returns true only if all the elements of a vector, or all the
%%     elements along dimension DIM of a matrix, are nonzero.


%% Copyright (C) 2008 Alois Schloegl 
%% needed for FreeMat 3.5
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

function y = all(x,DIM);

	if nargin<2,
		DIM = []; 
	end;
	if isempty(DIM), 
	        DIM = min(find(size(x)>1));
	        if isempty(DIM), DIM = 1; end;
	        if (DIM<1) DIM = 1; end; 
	end;
	
	y = sum(~x,DIM)==0;
end


