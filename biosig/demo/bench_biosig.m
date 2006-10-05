% Benchmark for the BioSig toolbox
%  The benchmark test performs a typical data analysis task.
%  Except for data loading, it tests the performance of the computational speed
%  and can be used to compare the performance of different platforms
%
%  Requirements:
%  Octave 2.1 or higher, or Matlab
%  BioSig4OctMat from http:/biosig.sf.net/
%
%  run bench_biosig and compare the results at other platforms
%   http://bci.tugraz.at/~schloegl/biosig/bench/
%
%  Send your benchmark result to <a.schloegl@ieee.org>
%


%	$Id: bench_biosig.m,v 1.6 2006-10-05 13:50:08 schloegl Exp $
%	Copyright (C) 2005,2006 by Alois Schloegl <a.schloegl@ieee.org>
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


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



if ~exist('l1.gdf','file') % download test file if not available
        system('wget http://hci.tugraz.at/schloegl/bci/bci7/l1.gdf');
end;

tic; K=0;

%[signal,HDR]=sload({[p,'x_train'],[p,'x_test']});
[signal,HDR]=sload('l1.gdf');

K=K+1;jo{K}='sload l1.gdf'; t(K)=toc
z = zscore(signal);

try
        bp = bandpower(z,HDR.SampleRate);
        bp(bp<-10)=NaN;
        K=K+1;jo{K}='bandpower'; t(K)=toc
catch, end;

[SIGMA1,PHI1,OMEGA1,m01,m11,m21] = wackermann(z(:,1:2),HDR.SampleRate);
[SIGMA2,PHI2,OMEGA2,m02,m12,m22] = wackermann(z(:,2:3),HDR.SampleRate);
K=K+1;jo{K}='wackermann'; t(K)=toc

[a,f,s] = barlow(z,HDR.SampleRate);
BARLOW = [a,f,s];
K=K+1;jo{K}='barlow'; t(K)=toc

[A,M,C] = hjorth(z,HDR.SampleRate);
HJORTH = [A,M,C];
K=K+1;jo{K}='hjorth'; t(K)=toc

uc  = 30:5:80;
a = [];
for ch = 1:size(signal,2),
        fprintf(1,'.%i',ch);
        INI.MOP = [0,3,0];
        INI.UC  = 2^(-uc(7)/8);

        X = tvaar(signal(:,ch),INI);
        X = tvaar(signal(:,ch),X);

        a = [a,X.AAR];
        K = K+1; jo{K} = ['aar #',int2str(ch)]; t(K)=toc
end

uc  = 30:5:80;
ab = [];
q = 8; 
for ch = 1:size(signal,2),
        fprintf(1,'.%i',ch);
        INI.MOP = [0,3,0];
        INI.UC  = 2^(-uc(7)/8);

        s = center(filter(ones(1,q),q,abs(diff(signal(:,ch)))));
        X = tvaar(s,INI);
        X = tvaar(s,X);

        ab = [ab,X.AAR];
        K = K+1; jo{K} = ['aar #',int2str(ch)]; t(K)=toc
end


%% K-fold Crossvalidation
K = 5;
ng = ceil([0:length(HDR.Classlabel)-1]'/length(HDR.Classlabel)*K);
Classifiers = {'LD2','LD3','LD4','MDA','MD2','MD3','GRB','GRB2','QDA','MQU','INQ','Cauchy','SVM','SVM11','RBF','REG'};
Classifiers = {'LDA','LDA/GSVD','LD2','LD3','LD4','REG','MDA','MD2','MD3','GRB','GRB2','QDA','MQU','IMQ','Cauchy','LDA/sparse','SVM:LIB','SVM:OSU'};


try,
        CC1 = findclassifier(bp,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],reshape(1:1152,16,72)',[1:72]'>24,'LD3');
        CC1.TSD.T = CC1.TSD.T/HDR.SampleRate;
        K=K+1; jo{K}='findclassifier bp'; t(K)=toc
catch,
end;



CC2 = findclassifier(BARLOW,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],reshape(1:1152,16,72)',[1:72]'>24,'LD3');
CC2.TSD.T = CC2.TSD.T/HDR.SampleRate;
K=K+1; jo{K}='findclassifier barlow LD3'; t(K)=toc
CC3 = findclassifier(HJORTH,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],reshape(1:1152,16,72)',[1:72]'>24,'LD3');
CC3.TSD.T = CC3.TSD.T/HDR.SampleRate;
K=K+1; jo{K}='findclassifier hjorth LD3'; t(K)=toc
CC4 = findclassifier(a,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],reshape(1:1152,16,72)',[1:72]'>24,'LD3');
CC4.TSD.T = CC4.TSD.T/HDR.SampleRate;
K=K+1; jo{K}='findclassifier aar LD3'; t(K)=toc
CC5 = findclassifier([SIGMA1,PHI1,OMEGA1,SIGMA2,PHI2,OMEGA2],HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],reshape(1:1152,16,72)',[1:72]'>24,'LD3');
CC5.TSD.T = CC5.TSD.T/HDR.SampleRate;
K=K+1; jo{K}='findclassifier Wackermann LD3'; t(K)=toc

clear ACC KAP MI AUC
TI = 36; 35;27;
for k=16:17,18:length(Classifiers); 
        try
        CC = findclassifier(a,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],(1:16:1152)',[1:72]'==TI,Classifiers{k});
        CC.TSD.T = CC.TSD.T/HDR.SampleRate;
        K=K+1; jo{K}=['findclassifier AAR ',Classifier{k}]; t(K)=toc
        CC90{k}=CC.TSD; 
        ACC(:,k)=CC90{k}.ACC00; 
        KAP(:,k)=CC90{k}.KAP00; 
        MI(:,k)=CC90{k}.I(:,1); 
        AUC(:,k)=CC90{k}.AUC(:,1); 
        catch        
        end;
end; 
figure(1)
subplot(221),
plot(CC90{k}.T,ACC);
subplot(222),
plot(CC90{k}.T,KAP);
subplot(223),
plot(CC90{k}.T,MI);
subplot(224),
plot(CC90{k}.T,AUC);
legend(Classifiers);

figure(2)
subplot(221),
plot(ACC(500:700,:)');
subplot(222),
plot(KAP(500:700,:)');
subplot(223),
plot(MI(500:700,:)');
subplot(224),
plot(AUC(500:700,:)');
legend(Classifiers);


CC91 = findclassifier(a,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],(1:16:1152)',[1:72]'==TI,'sparse_lda1');
CC91.TSD.T = CC91.TSD.T/HDR.SampleRate;
CC92 = findclassifier(a,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],(1:16:1152)',[1:72]'==TI,'LD3');
CC92.TSD.T = CC92.TSD.T/HDR.SampleRate;
CC93 = findclassifier(a,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],(1:16:1152)',[1:72]'==TI,'LD3/GSVD');
CC93.TSD.T = CC92.TSD.T/HDR.SampleRate;
CC94 = findclassifier(a,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],(1:16:1152)',[1:72]'==TI,'REG');
CC94.TSD.T = CC94.TSD.T/HDR.SampleRate;
CC95 = findclassifier(a,HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('300'))-1,[HDR.Classlabel,ng],(1:16:1152)',[1:72]'==TI,'SVM:LIB');
CC95.TSD.T = CC95.TSD.T/HDR.SampleRate;


if exist('OCTAVE_VERSION','builtin');
        om = 'Octave';
elseif 1,
        om = 'Matlab';
else
        om = 'FreeMat';
end;
outfile = sprintf('bench_biosig1.75+_%s_%s_%s.mrk',computer,om,version);

try
        unix(['cat /proc/cpuinfo >"',outfile,'"'])
catch
end;
fid = fopen(outfile,'a');
fprintf(fid,'\n\nDate:\t%s\n',date);
fprintf(fid,'Revision:\t$Id: bench_biosig.m,v 1.6 2006-10-05 13:50:08 schloegl Exp $\n');
fprintf(fid,'Computer:\t%s\nSoftware:\t%s\nVersion:\t%s\n',computer,om,version);

tmp = [diff([0,t(:)']);t(:)']';
for k=1:K,
        fprintf(fid,'%25s:\t%6.1f\t%6.1f\n',jo{k},tmp(k,:));
end;
fclose(fid);

