function x=trigg(s,TRIG,pre,post,gap)
% TRIGG cuts continous sequence into segments
%
% X = trigg(s, TRIG, pre, post [,gap])
% 
%  s 	is the continous sequence
%  TRIG defines the trigger points
%  pre	offset of the start of each segment (relative to trigger) 
%  post	offset of the end of each segment (relative to trigger) 
%  gap  number of NaN's to separate trials (default=0)
%  X	is a matrix of size(post-pre+1,length(TRIG))
%
%  if s has more than one column 
%    X.datatype = 'triggered'	
%    X.sz	= [post-pre+1+gap, length(TRIG)];
%    X.data is of size (post-pre+1+gap)*length(TRIG),size(s,2))
%

% see also: GETTRIGGER

%
%	Copyright (c) 1999-2002 by Alois Schloegl <a.schloegl@ieee.org>
%	02.07.2002 Version 1.31
%

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the  License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

if nargin<5,
	gap = 0;
end;

[nr,nc] = size(s);
N = post-pre+1+gap;


if isempty(TRIG),
        if nc==1,
                x = zeros(N,length(TRIG));
        else
                x.datatype = 'triggered';
		x.sz	   = [N, length(TRIG)];
                x.data     = zeros(0,nc);
        end;
        return;
end;

% include leading nan's
off = min(min(TRIG)+pre-1,0);
s   = [repmat(nan,-off,nc);s];
TRIG= TRIG-off;        

% include following nan's
off = max(max(TRIG)+post-length(s),0);
s   = [s; repmat(nan,off,nc)];

% devide into segments
if nc==1,
        x = repmat(NaN,N,length(TRIG));
        for m = 1:length(TRIG),
                x(1:N-gap,m) = s(TRIG(m)+(pre:post)');
        end;
else
        x.datatype = 'triggered';
	x.sz	   = [N, length(TRIG)];
        x.data     = repmat(NaN, N*length(TRIG), nc);
        for m = 1:length(TRIG),
                x.data(m*N + (1-N:-gap),:) = s(TRIG(m)+(pre:post)',:);	
        end;
end;
