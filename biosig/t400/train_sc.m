function [CC]=train_sc(D,classlabel,FUN)
% Train statistical classifier
% 
%  CC = train_sc(D,classlabel)
%  CC = train_sc(D,classlabel,TYPE)
%
%  The following classifier types are supported TYPE = 
%    'MDA'    mahalanobis distance based classifier
%    'MD2'    mahalanobis distance based classifier
%    'MD3'    mahalanobis distance based classifier
%    'GRB'    Gaussian radial basis function 
%    'QDA'    quadratic discriminant analysis
%    'LD2'    linear discriminant analysis (see LDBC2)
%    'LD3'    linear discriminant analysis (see LDBC3)
%    'LD4'    linear discriminant analysis (see LDBC4)
%    'GDBC'   general distance based classifier
%    'SVM','SVM1r'  support vector machines, one-vs-rest
%    'SVM11'  support vector machines, one-vs-one + voting
%              
% CC is a statistical classifier, it contains basically the mean 
% and the covariance of the data of each class. This information 
% is incoded in the so-called "extended covariance matrices".  
%
% CC can be used for various statistical classifiers included
%  LDA, MDA, QDA, GRB, etc. 
%
% see also: TEST_SC, COVM, LDBC2, LDBC3, LDBC4, MDBC, GDBC

%	$Id: train_sc.m,v 1.4 2006-05-25 21:34:44 schloegl Exp $
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

if nargin<3, FUN = ''; end;
        
% remove all NaN's
ix = any(isnan([D,classlabel]),2);
D(ix,:)=[];
classlabel(ix,:)=[];

[CC.Labels] = unique(classlabel(~isnan(classlabel)));

sz = size(D);
if sz(1)~=length(classlabel),
        error('length of data and classlabel does not fit');
end;

if isempty(strfind(lower(FUN),'svm'))
        CC.datatype = ['classifier:statistical:',lower(FUN)];
        CC.MD = repmat(NaN,[length(CC.Labels),sz(2)+[1,1]]);
        CC.NN = CC.MD;
        for k=1:length(CC.Labels),
                [CC.MD(k,:,:),CC.NN(k,:,:)] = covm(D(classlabel==CC.Labels(k),:),'E');
        end;        
        if strcmpi(FUN,'LD2');
                CC.weights = ldbc2(CC); 
        elseif strcmpi(FUN,'LD3');
                CC.weights = ldbc3(CC); 
        elseif strcmpi(FUN,'LD4');
                CC.weights = ldbc4(CC); 
        end;
elseif ~isempty(strfind(lower(FUN),'svm11'))
        CC = train_svm11(D,classlabel);
elseif ~isempty(strfind(lower(FUN),'svm'))
        CC = train_svm(D,classlabel);
        %CC = train_svm(D,classlabel,'SVM:LIB');
end;
