function [CC]=train_sc(D,classlabel)
% Train statistical classifier
% 
% see also: COVM, DECOVM, R2, MDBC, LDBC, LDBC2, LDBC3, LDBC4

%	$Id: train_sc.m,v 1.1 2005-03-07 16:12:45 schloegl Exp $
%	Copyright (C) 2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

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


[CC.Labels] = unique(classlabel(~isnan(classlabel)));

sz = size(D);
if sz(1)~=length(classlabel),
        error('length of data and classlabel does not fit');
end;

CC.datatype = 'classifier:statistical';
CC.MD = repmat(NaN,[length(CC.Labels),sz(2)+[1,1]]);
for k=1:length(CC.Labels),
        CC.MD(k,:,:) = covm(D(classlabel==CC.Labels(k),:),'E');
end;        
