function [YX,code]=elpos(Label);
% ELPOS provides electrode positions in 2-D according to [1]
%   
% [YX,code]=elpos(Label);
%
% see also: ELPOS3
%
% Reference(s): 
% [1] FEF: 
%   File Exchange Format for Vital Signs - Normative Annex A (Normative). 
%   The Medical Data Information Base (MDIB), Nomenclature, Data Dictionary and Codes
%   Table A.6.3: Nomenclature and Codes for Electrode Sites 
%   for Electroencephalography according to the International 10-20 system.
%   CEN/TC251/PT-40 (2001)

%	$Revision: 1.1 $
%	$Id: elpos.m,v 1.1 2004-02-13 17:19:53 schloegl Exp $
%	Copyright (c) 1997, 1998, 2004 by Alois Schloegl
%	a.schloegl@ieee.org	
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



load electrode.mat;

if nargin<1,
        Label='';
end
[nr,nc]=size(Label);
code=zeros(nr,1);
YX=100*ones(nr,2);

N=77;
for k=1:nr;
for l=1:102;%size(Electrode.Theta,2), 
	if strcmp(upper(deblank(Label(k,:))),upper(Electrode.Acronym{l}))
		code(k)=l;
		YX(k,:)=real(Electrode.Theta(l).*exp(i*pi/180*Electrode.Phi(l))*[1 -i]);
break;
	end;
end;
end;

K=code(code>0)';
xy=Electrode.Theta(1:N).*exp(i*pi/180*Electrode.Phi(1:N));

T=0:.001:2*pi;
R=110;
plot(real(xy),imag(xy),'x',real(xy(K)),imag(xy(K)),'ro',R*sin(T),R*cos(T),'b-',-R+R/10*sin(-T/2),10*cos(-T/2),'b-',10*sin(T/2)+R,10*cos(T/2),'b-',[-10 0 10],[R R+10 R],'b-');
%for k=K,text(real(xy(k)),imag(xy(k)),Electrode.Acronym{k});end;
%for k=[1:77 87 88 94 95]; text(real(xy(k)),imag(xy(k)),Electrode.Acronym{k});end;
for k=[1:77]; text(real(xy(k)),imag(xy(k)),Electrode.Acronym{k});end;
%plot(YX,'ro');
set(gca,'xtick',0,'ytick',0,'xticklabel','','yticklabel','')

return;

