function TRIG = gettrigger(s,TH);
% GETTRIGGER identifies trigger points 
%
% TRIG = gettrigger( s [,TH]); % trigger at ascending edge
% TRIG = gettrigger(-s [,TH]); % trigger at decending edge
%
% input : s   	signal
%         TH	Threshold; default: (max+min)/2
% output: TRIG	TRIGGER time points
%
% see also TRIGG

%	$Revision: 1.3 $
% 	$Id: gettrigger.m,v 1.3 2003-06-25 15:16:32 schloegl Exp $
%       Version 0.92        16 Jan 2003
%	Copyright (C) 2002-2003 by Alois Schloegl <a.schloegl@ieee.org>		

% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% Version 2 of the License, or (at your option) any later version.
%
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
%
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the
% Free Software Foundation, Inc., 59 Temple Place - Suite 330,
% Boston, MA  02111-1307, USA.


if nargin<2,
	TH = (max(s)+min(s))/2;
end;

TRIG = find(diff(sign(s-TH))>0)+1;

% perform check of trigger points



