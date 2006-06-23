function [CC]=train_svm11(D, classlabel, C)
% TRAIN_SVM11 trains a linear classifier using support vector machines with
% linear kernel. For multiclass Problems, 1 vs. 1 as implemented in LibSVM
% [1] is used.
%
% TRAIN_SVM LibSVM for Matlab [1] functions: svmtrain, svmpredict
%-------------------------------------------------
% CC = train_svm11(D,classlabel, C)
%-------------------------------------------------
%INPUTS:
%   D           Training Samples: Has to be a NxN Matrix with: N: Number of Samples, M: Feature Dimension
%   classlabel  Training Labels: Has to be a Nx1 Matrix with: N: Number of Samples
%   C           Constraint violation for C SVM
%OUTPUTS:
%   CC contains the linear classifier 
%   
% see also: TRAIN_SC, TEST_SC, TRAIN_SVM, FINDCLASSIFIER, 
%
% Reference(s):
% [1] LibSVM: LIBSVM -- A Library for Support Vector Machines
%       available at: http://www.csie.ntu.edu.tw/~cjlin/libsvm/
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


[CC.Labels] = unique(classlabel(~isnan(classlabel)));

sz = size(D);
if sz(1)~=length(classlabel),
        error('length of data and classlabel does not fit');
end;

%SET UP PARAMETERS FOR LibSVM
CC.options=sprintf('-c %g -t 0',C);  %use linear kernel, set C

if exist('SVMTrain','file'), 
        FUN = 'SVM:LIB:1vs1';
else
        error('No SVM training algorithm available. Install LibSVM for Matlab.\n'); 
end;

ix = any(isnan([D,classlabel]),2);
D(ix,:)=[];
classlabel(ix,:)=[];

CC.model = svmtrain(classlabel, D, CC.options);    % Call the training mex File     

CC.datatype = ['classifier:',lower(FUN)];