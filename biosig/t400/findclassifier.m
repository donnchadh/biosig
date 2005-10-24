function [CC,Q,tsd,md]=findclassifier(D,TRIG,cl,T,t0,FUN)
% FINDCLASSIFIER3
%       is very similar to FINDCLASSIFIER1 but uses a different 
%       criterion for selecting the time segment.
% [CC,Q,TSD,MD]=findclassifier2(D,TRIG,Class,class_times,t_ref);
%
% D 	data, each row is one time point
% TRIG	trigger time points
% Class class information
% class_times	classification times, combinations of times must be in one row 
% t_ref	reference time for Class 0 (optional)
%
% CC 	contains LDA and MD classifiers
% Q  	is a list of classification quality for each time of 'class_times'
% TSD 	returns the LDA classification 
% MD	returns the MD  classification 
%
% [CC,Q,TSD,MD]=findclassifier2(AR,find(trig>0.5)-257,~mod(1:80,2),reshape(1:14*128,16,14*8)');
%
%
% Reference(s): 
% [1] Schlögl A., Neuper C. Pfurtscheller G.
% 	Estimating the mutual information of an EEG-based Brain-Computer-Interface
%  	Biomedizinische Technik 47(1-2): 3-8, 2002.
% [2] A. Schlögl, C. Keinrath, R. Scherer, G. Pfurtscheller,
%	Information transfer of an EEG-based Bran-computer interface.
%	Proceedings of the 1st International IEEE EMBS Conference on Neural Engineering, Capri, Italy, Mar 20-22, 2003 


%   $Id: findclassifier.m,v 1.2 2005-10-24 14:28:46 schloegl Exp $
%   Copyright (C) 1999-2005 by Alois Schloegl <a.schloegl@ieee.org>	
%   This is part of the BIOSIG-toolbox http://biosig.sf.net/


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

CC =[]; Q = [];tsd=[];md=[];

if nargin<6,
        FUN='LD3';
end;
if nargin>4,
        if isempty(t0),
                t0=logical(ones(size(T,1),1));
        end;
end;
tmp=cl;tmp(isnan(tmp))=0;
if any(rem(tmp,1) & ~isnan(cl)),
        fprintf(2,'Error %s: class information is not integer\n',mfilename);
        return;
end;
if length(TRIG)~=length(cl);
        fprintf(2,'number of Triggers do not match class information');
end;

cl = cl(:);
TRIG = TRIG(:);
TRIG(isnan(cl))=[];
cl(isnan(cl))=[];
CL = unique(cl);
[CL,iCL] = sort(CL);

% add sufficient NaNs at the beginning and the end 
tmp = min(TRIG)+min(min(T))-1;
if tmp<0,
        TRIG = TRIG - tmp;
        D = [repmat(nan,[-tmp,size(D,2)]);D];
end;        
tmp = max(TRIG)+max(max(T))-size(D,1);
if tmp>0,
        D = [D;repmat(nan,[tmp,size(D,2)])];
end;        

% estimate classification result for all time segments in T - without crossvalidation 
CMX = zeros([size(T,1),length(CL)*[1,1]]);
KAPPA = repmat(NaN,size(t0));
for k = 1:size(T,1),
        if t0(k),
                c = []; d = [];
                for l = 1:length(CL), 
                        t = perm(TRIG(cl==CL(l)),T(k,:));
                        d = [d; D(t(:),:)];
                        c = [c; repmat(CL(l),prod(size(t)),1)];
                end;
                cc{k} = train_sc(d,c,FUN); 
                r  = test_sc(cc{k},d,FUN,c);
                KAPPA(k)  = r.kappa;
        end;
end;	
[maxQ(2),TI] = max(KAPPA.*t0); %d{K},
CC = cc{TI};
CC.KAPPA = KAPPA;
CC.TI = TI;

%% cross-validation with jackknife (trial-based leave-one-out-method)
nc  = max(max(T))-min(min(T))+1;

tsd  = repmat(nan,[nc*length(TRIG),length(CL)]);
JKGD  = tsd;
JKLL  = JKGD;
MDA   = JKGD;
%LDA   = JKGD;
LDA2  = JKGD;
LDA3  = JKGD;
LDA4  = JKGD;
tt    = MDA(:,1);
IX = find(~isnan(cl(:)'));
cc = CC; 
for l = IX;1:length(cl);
        c = find(cl(l)==CL);
        t = TRIG(l)+T(CC.TI,:);
        %t = t(t<=size(D,1));
        
        % decremental learning 
        if ~isempty(strfind(CC.datatype,'statistical')), 
                cc = untrain_sc(CC,c,D(t(:),:));
        elseif ~isempty(strfind(CC.datatype,'svm')),
                ix = IX; ix(l)=[];
                t = perm(TRIG(ix),T(k,:));
                c = repmat(cl(ix)',size(T,2),1);
                cc = train_svm(D(t(:),:),c(:));                    
        else  %if 1 % manual untraining of statistical classifier 
                [tmp,tmpn] = covm(D(t(:),:),'E');
                cc.MD 	= CC.MD;
                cc.MD(c,:,:) = CC.MD(c,:,:)-reshape(tmp,[1,size(tmp)]);         
        end;
        
        t = TRIG(l)+(min(min(T)):max(max(T)));
        if any(~isnan(tt((l-1)*nc+1:l*nc)))
                fprintf(2,'WARNING FINDCLASSIFIER#: overlapping segments\n',sum(~isnan(tt((l-1)*nc+1:l*nc))));
        end;
        tt((l-1)*nc+1:l*nc)   = t;
        
        r  = test_sc(cc,D(t(:),:),FUN);
        tsd((l-1)*nc+1:l*nc,:) = r.output;
end; 

CC.TSD  = bci4eval(tsd, (0:length(cl)-1)'*nc,cl,1,nc);

