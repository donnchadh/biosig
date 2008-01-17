%% any(x [,DIM])
%%     For a vector argument, return 1 if any element of the vector is
%%     nonzero.
%%
%%     For a matrix argument, return a row vector of ones and zeros with
%%     each element indicating whether any of the elements of the
%%     corresponding column of the matrix are nonzero.  For example,
%%
%%          any (eye (2, 4))
%%          => [ 1, 1, 0, 0 ]
%%
%%     If the optional argument DIM is supplied, work along dimension
%%     DIM.  For example,
%%
%%          any (eye (2, 4), 2)
%%          => [ 1; 1 ]
%%

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


function y = any(x,DIM);
	if nargin<2,
		DIM = []; 
	end;
	if isempty(DIM), 
	        DIM = min(find(size(x)>1));
	        if isempty(DIM), DIM = 1; end;
	        if (DIM<1) DIM = 1;end;  
	end;
	x(isnan(x))=0; 
	y = sum(~~x,DIM)>0;

