function H=plota(X,arg2,arg3,arg4,arg5,arg6,arg7)
% PLOTA plots all kind of data types
%
% PLOTA(X [,Mode]) 
%
% X.datatype determines type of data
%   'MVAR-COHERENCE'
%   'MVAR-SPECTRUM'
%   'HISTOGRAM'
%   'DBI-EPs'
%   'TSD1'
%   'TSD_BCI7'
%   'MDist-matrix'
%   'MD'
%   'SCATTER'
%   'STAT2'     
%   ''
%   ''
%   'REV' Mode='3D'
%   'REV' Mode='2D'
%
% 
%
% REFERENCE(S):

%
%	Copyright (c) 1999-2002 by Alois Schloegl <a.schloegl@ieee.org>
%	17.07.2002 Version 1.33
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

h=[];
if strcmp(X.datatype,'MVAR-COHERENCE'),
        if length(size(X.COH))==3,
                M = size(X.COH,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                tmp = abs(squeeze(X.COH(k1,k2,:)));        
                                plot(X.f,[abs(tmp),tanh([atanh(tmp)*[1,1]+ones(size(tmp))*X.ci*sqrt(1/2/(min(X.N)-X.p))*[1,-1]])]);        
                                axis([0,max(X.f),0,1])
                        end;
                end;
                suptitle('Ordinary coherence')
        elseif (length(size(X.COH))==4) & (size(X.COH,4)==2),
                M = size(X.COH,1);
                for k1 = 1:M;
                        for k2 = k1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                tmp = abs(squeeze(X.COH(k1,k2,:,1)));
                                h=plot(X.f,[abs(tmp),tanh([atanh(tmp)*[1,1]+ones(size(tmp))*X.ci*sqrt(1/2/(min(X.N(1,:))-X.p))*[1,-1]])]);        
                                set(h(1),'color',[0,0,1])
                                set(h(2),'color',[.5,.5,1])
                                set(h(3),'color',[.5,.5,1])
                                hold on
                                tmp = abs(squeeze(X.COH(k1,k2,:,2)));
                                h=plot(X.f,[abs(tmp),tanh([atanh(tmp)*[1,1]+ones(size(tmp))*X.ci*sqrt(1/2/(min(X.N(2,:))-X.p))*[1,-1]])]);        
                                set(h(1),'color',[0,1,0])
                                set(h(2),'color',[.5,1,.5])
                                set(h(3),'color',[.5,1,.5])
                                hold off
                                axis([0,max(X.f),0,1])
                                if k1==1,
                                        ylabel(X.ElectrodeName{k2});
                                end;
                                if k1==k2,
                                        title(X.ElectrodeName{k2});
                                end;
                        end;
                end;
                suptitle('Ordinary coherence')
        elseif length(size(X.COH))==4,
                M = size(X.COH,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                tmp = abs(squeeze(X.COH(k1,k2,:,:)));        
                                %plot(X.f,[abs(tmp),tanh([atanh(tmp)*[1,1]+ones(size(tmp))*X.ci*sqrt(1/2/(min(X.N)-X.p))*[1,-1]])]);        
                                mesh(abs(tmp))
                                v = axis; v(5:6)=[0,1]; axis(v);%axis([0,max(X.f),0,1])
                        end;
                end;
                suptitle('timevarying coherence')
        end;
        
elseif strcmp(X.datatype,'MVAR-PHASE'),
        if length(size(X.COH))==3,
                M = size(X.COH,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                plot(f,unwrap(squeeze(angle(X.COH(k1,k2,:))))*180/pi);        
                                axis([0,max(X.f),-360,360])        
                        end;
                end;
                suptitle('Phase')
        elseif (length(size(X.COH))==4) & (size(X.COH,4)==2),
                M = size(X.COH,1);
                for k1 = 1:M;
                        for k2 = k1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                tmp = abs(squeeze(X.COH(k1,k2,:,1)));
                                plot(X.f,unwrap(squeeze(angle(X.COH(k1,k2,:,1))))*180/pi,'b',X.f,unwrap(squeeze(angle(X.COH(k1,k2,:,2))))*180/pi,'g');        
                                axis([0,max(X.f),-360,360])        
                                if k1==1,
                                        ylabel(X.ElectrodeName{k2});
                                end;
                                if k1==k2,
                                        title(X.ElectrodeName{k2});
                                end;
                        end;
                end;
                suptitle('Phase')
        elseif length(size(X.COH))==4,
                M = size(X.COH,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                plot(X.f,unwrap(squeeze(angle(X.COH(k1,k2,:))))*180/pi);        
                                %plot(X.f,[abs(tmp),tanh([atanh(tmp)*[1,1]+ones(size(tmp))*X.ci*sqrt(1/2/(min(X.N)-X.p))*[1,-1]])]);        
                                mesh(abs(tmp))
                                v = axis; v(5:6) = [-360,360]; axis(v);%axis([0,max(X.f),0,1])
                        end;
                end;
                suptitle('timevarying phase')
        end;
        
elseif strcmp(X.datatype,'MVAR-PDCF'),
        if length(size(X.PDCF))==3,
                M = size(X.PDCF,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                plot(f,squeeze(X.PDCF(k1,k2,:)));        
                                axis([0,max(X.f),0,1])        
                        end;
                end; tle('PDCF')
        elseif (length(size(X.PDCF))==4) & (size(X.PDCF,4)==2),
                M = size(X.PDCF,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                tmp = abs(squeeze(X.PDCF(k1,k2,:,1)));
                                plot(X.f,squeeze(X.PDCF(k1,k2,:,1)),'b',X.f,squeeze(X.PDCF(k1,k2,:,2)),'g');        
                                axis([0,max(X.f),0,1])        
                                if k1==1,
                                        ylabel(X.ElectrodeName{k2});
                                end;
                                if 1==k2,
                                        title(X.ElectrodeName{k1});
                                end;
                        end;
                end;
                suptitle('PDCF')
        elseif length(size(X.PDCF))==4,
                M = size(X.PDCF,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                plot(X.f,squeeze(X.PDCF(k1,k2,:)));        
                                %plot(X.f,[abs(tmp),tanh([atanh(tmp)*[1,1]+ones(size(tmp))*X.ci*sqrt(1/2/(min(X.N)-X.p))*[1,-1]])]);        
                                mesh(abs(tmp))
                                v = axis; v(5:6)=[0,1]; axis(v);%axis([0,,,max(X.f),0,1])
                        end;
                end;
                suptitle('time-varying PDCF')
        end;
        
elseif strcmp(X.datatype,'MVAR-PDC'),
        if length(size(X.PDC))==3,
                M = size(X.PDC,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                plot(f,squeeze(X.PDC(k1,k2,:)));        
                                axis([0,max(X.f),0,1])        
                        end;
                end;
                suptitle('PDC')
        elseif (length(size(X.PDC))==4) & (size(X.PDC,4)==2),
                M = size(X.PDC,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                tmp = abs(squeeze(X.PDC(k1,k2,:,1)));
                                plot(X.f,squeeze(X.PDC(k1,k2,:,1)),'b',X.f,squeeze(X.PDC(k1,k2,:,2)),'g');        
                                axis([0,max(X.f),0,1])        
                                if k1==1,
                                        ylabel(X.ElectrodeName{k2});
                                end;
                                if 1==k2,
                                        title(X.ElectrodeName{k1});
                                end;
                        end;
                end;
                suptitle('PDC')
        elseif length(size(X.PDC))==4,
                M = size(X.PDC,1);
                for k1 = 1:M;
                        for k2 = 1:M;
                                subplot(M,M,k1+(k2-1)*M);
                                plot(X.f,squeeze(X.PDC(k1,k2,:)));        
                                %plot(X.f,[abs(tmp),tanh([atanh(tmp)*[1,1]+ones(size(tmp))*X.ci*sqrt(1/2/(min(X.N)-X.p))*[1,-1]])]);        
                                mesh(abs(tmp));
                                v = axis; v(5:6)=[0,1]; axis(v);%axis([0,max(X.f),0,1])
                        end;
                end;
                suptitle('time-varying PDC')
        end;
        
elseif strcmp(X.datatype,'MVAR-SPECTRUM'),
        
elseif strcmp(X.datatype,'EDF'),
        data = arg2;
        [nr,nc]=size(data);
        for k = 1:nc,
                subplot(nc,1,k);
                %t = (X.Block.number(3)+1:X.Block.number(4))/X.AS.MAXSPR*X.Dur;
                
                t = X.FILE.POS*X.Dur+(1-nr:0)/X.AS.MAXSPR*X.Dur/size(X.SIE.T,2)*size(X.SIE.T,1);
                plot(t, data(:,k));
            	ylabel(deblank(X.Label(X.SIE.ChanSelect(k),:)));    
        end;
        xlabel('t [s]')
        
elseif strcmp(X.datatype,'qualitycontrol'),
        if nargin>1,
                Mode=arg2;
        else
                c='b';
                Mode='log';
        end;
        
        for k=1:size(X.AR,1);
                [H(:,k),F] = freqz(sqrt(X.PE(k,size(X.AR,2)+1)/X.SampleRate*2),ar2poly(X.AR(k,:)),(0:.1:X.SampleRate/2)',X.SampleRate);
        end;
        if ~isfield(X,'Impedance')
                X.Impedance=5000; %5kOHM
        end;
        X.Impedance,
        if strcmp(lower(Mode),'log')
                semilogy([0;X.SampleRate/2],[1;1]*[X.stats.QUANT(1)/sqrt(12*X.SampleRate/2),sqrt(4*310*138e-25*X.Impedance)*1e6],':',F,abs(H),'r');
                %semilogy([0,X.SampleRate/2]',[1;1]*X.stats.QUANT/sqrt(12*X.SampleRate/2),':');
                %hold on
                %semilogy([0,X.SampleRate/2]',sqrt(4*310*138e-25*5e3)*1e6*[1,1],'k');
                %h=semilogy(F,abs(H));
                %hold off;
        else
                plot([0;X.SampleRate/2],[1;1]*[X.stats.QUANT(1)/sqrt(12*X.SampleRate/2),sqrt(4*310*138e-25*X.Impedance)],':',F,abs(H),'r');
                %hold on;
                %plot([0,X.SampleRate/2]',sqrt(4*310*138e-25*1e5)*1e6*[1,1],':');
                %h=plot(F,abs(H));
                %hold off;
        end;
        
        legend({'Q-noise',sprintf('%4.1f kOhm',X.Impedance/1000),'amp noise'})
        
        xlabel('frequency [Hz]')
        ylabel([X.PhysDim,'/Hz^{1/2}']);
        title('spectral density')
        clear H
        
elseif strcmp(X.datatype,'HISTOGRAM')
        if nargin<2 
                yscale='lin '; 
        else
                yscale=arg2;
        end;
        t=X.X;
        %HISTO=hist2pdf(HISTO);
        h=X.H;   
        
        mu = sumskipnan(repmat(t,size(h)./size(t)).*h,1)./sumskipnan(h,1);
        x  = (repmat(t,size(h)./size(t))-repmat(mu,size(h)./size(mu)));
        sd2= sumskipnan(x.*x.*h,1)./sumskipnan(h,1);
        
        [tmp,tmp2]=find(h>0);
        
        MaxMin=t([max(tmp) min(tmp)]);
        
        if strcmp(yscale,'lin '),
                plot(t,[h],'-');
        elseif strcmp(yscale,'lin+'),
                tmp=max(h)/2;
                tmp=sum(h)/sqrt(2*pi*sd2);
                plot(t,[h],'-',t,exp(-(t-mu).^2/sd2/2)/sqrt(2*pi*sd2)*sum(h),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5],tmp*ones(7,1),'+-',MaxMin,tmp,'rx' );
                v=axis; v=[MaxMin(2) MaxMin(1) 1 max(h)]; axis(v);
        elseif strcmp(yscale,'log ') | strcmp(yscale,'log'),
                semilogy(t,[h],'-')
        elseif strcmp(yscale,'log+'),
                tmp=sum(h)/sqrt(2*pi*sd2);
                %semilogy(t,[h]+.01,'-',t,exp(-(t*ones(size(mu))-ones(size(t))*mu).^2./(ones(size(t))*sd2)/2)./(ones(size(t))*(sqrt(2*pi*sd2)./sum(h))),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5],tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
                semilogy(t,[h]+.01,'-',t,exp(-(t(:,ones(size(mu)))-mu(ones(size(t)),:)).^2/sd2(ones(size(t)),:)/2)./sqrt(2*pi*sd2(ones(size(t)),:)).*(ones(size(t))*sum(h)),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5],tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
                v=axis; v=[MaxMin(2)+0.1*diff(MaxMin) MaxMin(1)-0.1*diff(MaxMin) 1 max(h)]; axis(v);
        elseif strcmp(yscale,'csum'),
                tmp=sum(h)/2;
                plot(t,cumsum(h),'-',t,cumsum(exp(-(t-mu).^2/sd2/2)/sqrt(2*pi*sd2)/X.N),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5],tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
                v=axis; v(1:2)=[MaxMin(2)+0.1*diff(MaxMin) MaxMin(1)-0.1*diff(MaxMin)]; axis(v);
        elseif strcmp(yscale,'stacked'),
                bar(t,h,'stacked');        
        end;
        
elseif strcmp(X.datatype,'DBI-EPs'),
        if nargin<2,
                arg2='b';        
        end;
        if nargin<3
                arg3=100;
        end;
        
        for k=1:length(X.eegchan),
                subplot(13,10,k);
                
                mu=X.RES(k).SUM./X.RES(k).N;
                sd=sqrt((X.RES(k).SSQ-X.RES(k).SUM.*mu)./max(X.RES(k).N,0)); 
                se=sd./sqrt(X.RES(k).N);
                
                h=plot(X.t,[mu(:),sd(:),se(:)]*[1,1,1,1,1;-1,1,0,0,0;0,0,-1,1,0],arg2);
                set(h(5),'linewidth',2);
                set(h(1),'color',[0,.75,.75]);
                set(h(2),'color',[0,.75,.75]);
                set(h(3),'color',[.5,.5,1]);
                set(h(4),'color',[.5,.5,1]);
                axis([-3,3,-arg3,arg3]);
                title(sprintf('#%i',X.eegchan(k))); 
        end;        
        
        
elseif strcmp(X.datatype,'SCATTER'),
        s='x';
        if nargin<2,
                
        elseif nargin==2,
                if length(arg2)==1
                        s = arg2;
                else
                        Labels=arg2;
                end
        elseif nargin>2,
                s = arg2;
                Labels = arg3;
        end;
        
        if length(X)==1,
                if ~isfield(X,'R');
	                [X.R,X.p,X.CIL,X.CIU] = corrcoef(X.data,'Rank');        
                end;        
                [nr,nc] = size(X.data);
                nc=nc-1;       
                for k   = 1:nc,
                        for l = k+1:nc+1,%[1:k-1,k+1:nc],
                                h=subplot(nc,nc,(k-1)*nc+l-1);
                                plot(X.data(:,l),X.data(:,k),s);
                                %ht=title(sprintf('r=%.3f %s [%.3f,%.3f]',X.R(k,l),char('*'*(X.p(k,l)<[.05,.01,.001])),X.CIL(k,l),X.CIU(k,l)));
                                ht=title(sprintf('r = %.4f %s',X.R(k,l),char('*'*(X.p(k,l)<[.05,.01,.001]))));
                                pos=get(ht,'position');
                                %pos(2)=max(X.data(k));
                                %set(ht,'position',pos);
                                % title gives the r-value, its confidence interval (see CORRCOFF for which level), 
                                %      and indicates the significance for alpha=0.05 (*), 0.01(**) and 0.001(***)
                                if l == (k+1),
                                        xlabel(Labels{l});        
                                        ylabel(Labels{k});        
                                else
                                        set(h,'xtick',[]);    
                                        set(h,'ytick',[]);    
                                end;
                        end;
                end;
        else
                if ~isfield(X,'R');
	                [X.R,X.p,X.CIL,X.CIU] = corrcoef(X(1).data,X(2).data,'Rank');        
                end;
                [nr,nc2] = size(X(2).data);
                for k   = 1:nc,
                        for l = 1:nc2,%[1:k-1,k+1:nc],
                                h=subplot(nc,nc2,(k-1)*nc2+l-1);
                                plot(X(2).data(:,l),X(1).data(:,k),s);
                                ht=title(sprintf('r=%.3f %s [%.3f,%.3f]',X.R(k,l),char('*'*(X.p(k,l)<[.05,.01,.001])),X.CIL(k,l),X.CIU(k,l)));
                                %ht=title(sprintf('r = %.4f %s',X.R(k,l),char('*'*(X.p(k,l)<[.05,.01,.001]))));
                                pos=get(ht,'position');
                                %pos(2)=max(X.data(k));
                                %set(ht,'position',pos);
                                % title gives the r-value, its confidence interval (see CORRCOFF for which level), 
                                %      and indicates the significance for alpha=0.05 (*), 0.01(**) and 0.001(***)
                                if l == 1,
                                        %xlabel(arg3{l});        
                                        ylabel(arg3{k});        
                                else
                                        %set(h,'xtick',[]);    
                                        set(h,'ytick',[]);    
                                end;
                        end;
                        if k == nc,
                                xlabel(arg3{l});        
                                %ylabel(arg3{k});        
                        else
                                set(h,'xtick',[]);    
                                %set(h,'ytick',[]);    
                        end;
                end;
        end;
        
elseif strcmp(X.datatype,'STAT2'),
        if nargin<2,
                arg2='b';        
        end;
        if isfield(X,'t')
                t=X.t;        
        else
                t=1:length(X.SUM);        
        end;
        if nargin>2
                t=arg3;
        end;
        
        mu=X.SUM./X.N;
        sd=sqrt((X.SSQ-X.SUM.*mu)./max(X.N-1,0));
        se=sd./sqrt(X.N);
        
        h=plot(t,[mu(:),sd(:),se(:)]*[1,1,1,1,1;0,-1,1,0,0;0,0,0,-1,1],arg2);
        set(h(1),'linewidth',2);
        tmp=get(h(1),'color');
        set(h(2),'color',1-(1-tmp)/2);
        set(h(3),'color',1-(1-tmp)/2);
        %set(h(4),'color',[0,0,1]);
        %set(h(5),'color',[0,0,1]);
        
elseif strcmp(X.datatype,'TSD1'),
        if nargin<2,
                arg2='b';        
        end;
        h=plot(X.t,[X.mu,X.sd]*[1,1,1;0,-1,1],arg2);
        set(h(1),'linewidth',2);
        hold on
        h=plot(X.TI(:),1,'.k');
        
        
elseif strcmp(X.datatype,'TSD_BCI7'),
        if nargin<2,
                for k=1:3, 
                        nf(k)=subplot(1,3,k); 
                end;
        else
                nf=arg2;
        end;
        Fs=128;
        N=length(X.I);
        
        if isfield(X,'T')
                t=(min(X.T(:)):max(X.T(:)))/X.Fs;
        else
                t=(1:N)/Fs;        
        end;
        
        subplot(nf(1));
        plot(t,X.ERR*100);
        ylabel('Error rate [%]')
        v=axis;v=[0,10,0,60];axis(v);
        
        subplot(nf(2));
        h=plot(t,[X.M1(:),X.M2(:),X.SD1(:),X.SD2(:)]*[1,0,0,0; 0,1,0,0; 1,0,1,0; 1,0,-1,0; 0,1,0,1; 0,1,0,-1]','b',t([1,length(t)]),[0,0],'k');
        set(h(1),'linewidth',2);
        set(h(2),'linewidth',2);
        set(h(7),'linewidth',2);
        set(h(2),'color','g');
        set(h(5),'color','g');
        set(h(6),'color','g');
        ylabel('Average TSD');
        v=axis;v(1:2)=[0,10];axis(v);
        
        subplot(nf(3));
        plot(t,X.I);
        ylabel('Mutual Information [bits]')
        v=axis;v=[0,10,0,2];axis(v);
        
        if length(nf)>3,
                subplot(nf(4))
                plot(t,X.corcov)
                v=axis;v=[0,10,-1,1];axis(v);
                grid on
                ylabel('correlation coefficient')        
        end;
        
elseif strcmp(X.datatype,'REV'),
        if nargin<2
                arg2=[];
        end;
        
        if strcmpi(arg2,'3d')        
                R=X.REV;
                [mxR,ix]=min(R);
                [myR,iy]=min(R');
                [IX,IY]=find(R==min(R(:)));
                [IX,IY],
                h=mesh(R');hold on
                plot3(ix,1:length(ix),mxR,'go-')
                plot3(1:length(iy),iy,myR,'cx-')
                plot3(IX,IY,min(R(:)),'r*');
                hold off
                v=axis;v(5:6)=[0.1,.2];axis(v);
                a=gca;
                set(a,'xtick',X.P);
                t=1:5:length(X.UC);
                set(a,'ytick',t);
                set(a,'yticklabel',X.UC(t));
                ylabel('update coefficent UC')
                xlabel('model order p')
                zlabel('REV = MSE/MSY')
        else  %if strcmpi(arg2,'2d')        
                subplot(121)
                semilogx(X.UC,X.REV')
                v=axis;v=[1e-6,1e-1,0.1,.15];axis(v);
                xlabel('update coefficent UC')
                ylabel('REV = MSE/MSY')
                
                subplot(122)
                h=plot(X.P,X.REV)
                v=axis;v(3:4)=[0.1,.15];axis(v);
                xlabel('model order p')
                ylabel('REV = MSE/MSY')
                
        end;
        
elseif strcmp(X.datatype,'MDist-matrix'),
        if nargin<2
                arg2=[];
        end;
        h=mesh(X.D)
        h=get(gca,'xtick');
        %set(gca,'xticklabel',X.t(h(h>0)))
        h=get(gca,'ytick');
        %set(gca,'yticklabel',X.t(h(h>0)))
        
elseif strcmp(X.datatype,'MD'),
        if nargin<2
                arg2=[];
        end;
        
        if strcmpi(arg2,'3d')        
                R=X.REV;
                [mxR,ix]=max(R);
                [myR,iy]=max(R');
                [IX,IY]=find(R==max(R(:)));
                h=mesh(R');hold on
                plot3(ix,1:length(ix),mxR,'go-')
                plot3(1:length(iy),iy,myR,'cx-')
                plot3(IX,IY,min(R(:)),'r*');
                hold off
                %                v=axis;v(5:6)=[0.1,.2];axis(v);
                a=gca;
                set(a,'xtick',X.P);
                t=1:5:length(X.UC);
                set(a,'ytick',t);
                set(a,'yticklabel',X.UC(t));
                ylabel('update coefficent UC')
                xlabel('model order p')
                zlabel('log_{10}(MD)')
        else  %if strcmpi(arg2,'2d')        
                subplot(121)
                semilogx(X.UC,X.REV')
                %                v=axis;v=[1e-6,1e-1,0.1,.15];axis(v);
                xlabel('update coefficent UC')
                ylabel('log_{10}(MD)')
                
                subplot(122)
                h=plot(X.P,X.REV)
                %               v=axis;v(3:4)=[0.1,.15];axis(v);
                xlabel('model order p')
                ylabel('log_{10}(MD)')
        end;
end;


if nargout,
        H=h;
end;
