function [CC]=train_sc(D,classlabel)
% Train statistical classifier
% 
%  CC = train_sc(D,classlabel)
%       
% CC is a statistical classifier, it contains basically the mean 
% and the covariance of the data of each class. This information 
% is incoded in the so-called "extended covariance matrices".  
%
% CC can be used for various statistical classifiers included
%  LDA, MDA, QDA, GRB, etc. 
%
% see also: TEST_SC, COVM, LDBC2, LDBC3, LDBC4, MDBC, GDBC

%	$Id: train_sc.m,v 1.2 2005-03-07 17:11:20 schloegl Exp $
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
CC.NN = CC.MD;
for k=1:length(CC.Labels),
        [CC.MD(k,:,:),CC.NN(k,:,:)] = covm(D(classlabel==CC.Labels(k),:),'E');
end;        
