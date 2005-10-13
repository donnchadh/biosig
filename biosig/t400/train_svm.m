function [CC]=train_svm(D,classlabel,FUN)
% TRAIN_SVM trains linear classifier using support vector machines with
% linear kernel. 
%
% TRAIN_SVM requires OSU-SVM [3] or LOO-SVM [2] or LibSVM for Matlab [1]
% 
% CC = train_svm(D,classlabel)
%
%   CC contains the linear classifier 
%   CC.Labels   list of classes
%   CC.W        bias and weights  
%
% see also: TRAIN_SC, TEST_SC, FINDCLASSIFIER, 
%
% Reference(s):
% [1] LibSVM: LIBSVM -- A Library for Support Vector Machines
%       available at: http://www.csie.ntu.edu.tw/~cjlin/libsvm/
% [2] LOO-SVM: Matlab code for SVM incremental learning and decremental unlearning (LOO validation).
%       available at: http://bach.ece.jhu.edu/pub/gert/svm/incremental/
% [3] OSU-SVM: A Support Vector Machine Toolbox for MATLAB
%       available at: http://svm.sourceforge.net/
% [4]	
%	http://www.isis.ecs.soton.ac.uk/resources/svminfo/
% [5] S. Canu  and Y. Grandvalet and A. Rakotomamonjy (2003)
% 	SVM and Kernel Methods Matlab Toolbox
%	Perception Systèmes et Information, INSA de Rouen, Rouen, France",
%	http://asi.insa-rouen.fr/~arakotom/toolbox/index.html

%	$Id: train_svm.m,v 1.1 2005-10-13 08:25:30 schloegl Exp $
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

%C              Cost of constrain violation for C-SVM (Use C = 1 as default value)
C = 1; 
%SET UP PARAMETERS FOR OSU_SVM FUNCTION
Parameters = [0 1 1 1 C]; % Linear Kernel, C=1;

if nargin>2,
        
elseif exist('SVMTrain','file')==3, 
        FUN = 'SVM:LIB';
elseif exist('mexSVMTrain','file')==3, 
        FUN = 'SVM:OSU';
elseif exist('svcm_train','file')==2, 
        FUN = 'SVM:LOO';
elseif exist('svmclass','file')==2, 
        FUN = 'SVM:KM';
elseif exist('svc','file')==2, 
        FUN = 'SVM:Gunn';
else
        error('No SVM training algorithm available. Install OSV-SVM, or LOO-SVM, or libSVM for Matlab.\n'); 
end;

ix = any(isnan([D,classlabel]),2);
D(ix,:)=[];
classlabel(ix,:)=[];

M = length(CC.Labels);
if M==2, M=1; end; 
CC.weights = repmat(NaN, sz(2)+1,M);
for k = 1:M,
        cl = sign((classlabel~=CC.Labels(k))-.5);
        if strcmp(FUN, 'SVM:LIB');
                model = svmtrain(cl, D, '-s 0 -c 1 -t 0 -d 1');    % C-SVC, C=1, linear kernal, degree = 1,     
                w = -cl(1) * model.SVs' * model.sv_coef;  %Calculate decision hyperplane weight vector
                % ensure correct sign of weight vector and Bias according to class label
                Bias = -model.rho * cl(1);
                
        elseif strcmp(FUN, 'SVM:OSU');
                [AlphaY, SVs, Bias, Parameters, nSV, nLabel] = mexSVMTrain(D', cl', [0 1 1 1 C]);    % Linear Kernel, C=1; degree=1, c-SVM
                w = -SVs * AlphaY'*cl(1);  %Calculate decision hyperplane weight vector
                % ensure correct sign of weight vector and Bias according to class label
                Bias = -Bias * cl(1);
                
        elseif strcmp(FUN, 'SVM:LOO');
                [a, Bias, g, inds, inde, indw]  = svcm_train(D, cl, 1); % C = 1; 
                w = D(inds,:)' * (a(inds).*cl(inds)) ;
                
        elseif strcmp(FUN, 'SVM:Gunn');
                [nsv, alpha, Bias,svi]  = svc(center(D), cl,1,1); % linear kernel, C = 1; 
                w = D(svi,:)' * alpha(svi) * cl(1);
                Bias = mean(D*w);
                
        elseif strcmp(FUN, 'SVM:KM');
                [xsup,w1,Bias,inds,timeps,alpha] = svmclass(D,cl,1,1,'poly',1); % C = 1; 
                w = -D(inds,:)' * w1;
                
        else
                fprintf(2,'Error TRAIN_SVM: no SVM training algorithm available\n');  
                return;
        end
        
        CC.weights(1,k) = -Bias; 
        CC.weights(2:end,k) = w;
end;

if length(CC.Labels)==2, 
	CC.weights(:,2) = -CC.weights;
end;

CC.datatype = ['classifier:',lower(FUN)];
