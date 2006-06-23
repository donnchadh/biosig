function [tsd]=test_svm11(CC, D, classlabel)
% TEST_SVM11: apply svm classifier with 1 vs 1 voting strategy to test data 
%-------------------------------------------------
%  tsd = test_svm11(CC,D) 
%-------------------------------------------------
%INPUTS:
%CC         the trained classifier from train_svm11
%D          test samples
%classlabel test classlabels (please note that this input is mandatory, 
%           if the class labels are not available, just use arbitrary values.
%           eg.: dummy = ones(size((D),1), 1);
%
%OUTPUTS:
%R.output     "output distance", these are binary values which result from
%             the voting strategy but might be used by other functions
%R.classlabel class for output data
%
%If the classlabel input is provided correctly, the following values are returned.
%Otherwise, they are just dummy values.
%
%R.kappa      Cohen's kappa coefficient
%R.ACC        Classification accuracy 
%R.H          Confusion matrix 
%  
% see also: TRAIN_SVM11

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

ix = any(isnan([D,classlabel]),2);
D(ix,:)=[];
classlabel(ix,:)=[];

[cl, accuracy] = svmpredict(classlabel, D, CC.model);   %Use the classifier

%Create a pseudo tsd matrix for bci4eval
tsd = zeros(size(cl,1), CC.model.nr_class);
for i = 1:size(cl,1)
    tsd(i,(cl(i)))=1;
end
