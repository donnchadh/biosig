function [xyz,code]=elpos3(Label);
% ELPOS3 provides electrode positions in 3-D according to FEF 
%   
% [YX,code]=elpos3(Label);
%
% see also: ELPOS
%
% Reference(s): 
% [1] FEF: 
%   File Exchange Format for Vital Signs - Normative Annex A (Normative). 
%   The Medical Data Information Base (MDIB), Nomenclature, Data Dictionary and Codes
%   Table A.6.3: Nomenclature and Codes for Electrode Sites 
%   for Electroencephalography according to the International 10-20 system.
%   CEN/TC251/PT-40 (2001)
 


%	$Revision: 1.1 $
%	$Id: elpos3.m,v 1.1 2004-02-13 17:19:53 schloegl Exp $
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
xyz=ones(nr,3);

l=1:size(Electrode.Theta,2);
XYZ = [	sin(pi/180*Electrode.Theta).*cos(pi/180*Electrode.Phi);...
	sin(pi/180*Electrode.Theta).*sin(pi/180*Electrode.Phi);...
	cos(pi/180*Electrode.Theta)]';

for k=1:nr,
for l=1:102,%size(Electrode.Theta,2), 
	if strcmp(upper(deblank(Label(k,:))),upper(Electrode.Acronym{l}))
		code(k)=l;
		xyz(k,1)=sin(pi/180*Electrode.Theta(l)').*cos(pi/180*Electrode.Phi(l)');
		xyz(k,2)=sin(pi/180*Electrode.Theta(l)').*sin(pi/180*Electrode.Phi(l)');
		xyz(k,3)=cos(pi/180*Electrode.Theta(l)');
break;
	end;
end;
end;


K=code(code>0)';
plot3(XYZ(:,1),XYZ(:,2),XYZ(:,3),'x',XYZ(K,1),XYZ(K,2),XYZ(K,3),'ro');
%plot3(XYZ,'x',XYZ(K,:),'ro');
%for k=K,text(real(xy(k)),imag(xy(k)),Electrode.Acronym{k});end;
for k=[1:77 87 88 94 95],text(XYZ(k,1),XYZ(k,2),XYZ(k,3),Electrode.Acronym{k});end;
%plot(YX,'ro');
set(gca,'xtick',0,'ytick',0)


return;


ELPOS=[...
'                E1      E2                 ';...
'                FP1 FPZ FP2                ';...
'        AF7   AF3   AFZ   AF4   AF8        ';...
'    F7  F5  F3  F1  FZ  F2  F4  F6  F8     ';...
'    FT7 FC5 FC3 FC1 FCZ FC2 FC4 FC6 FT8    ';...
'T9  T7  C5  C3  C1  CZ  C2  C4  C6  T8  T10';...
'A1  TP7 CP5 CP3 CP1 CPZ CP2 CP4 CP6 CP8 A2 ';...
'    P7  P5  P3  P1  PZ  P2  P4  P6  P8     ';...
'        PO7   PO3   POZ   PO4   PO8        ';...
'                O1  OZ  O2                 ';...
'                    IZ                     ';...
];

%Label=[Label '    '];
fid=fopen('elpos.txt','r');
s=fread(fid,inf,'char');
fclose(fid);

load elposdeg.txt;

TabPos=find(s==9);
CrPos=find(s==10);
Electrode = struct ('Acronym',[],'Code',[],'Systematic_Name',[],'Description',[],'Theta',[],'Phi',[]);

for k=1:length(CrPos)-1,
	tmp=TabPos(TabPos>CrPos(k));
    	Electrode.Acronym{k}=setstr(s(CrPos(k)+1:tmp(1)-1))';
    	Electrode.Code(k)=str2num(setstr(s(tmp(1)+1:tmp(2)-1))');
    	tmp1=find(Electrode.Code(k)==elposdeg(:,1));
%    	if ~isempty(tmp1)
%		Electrode.Theta{k}=elposdeg(tmp1,2);
%		Electrode.Phi{k}=elposdeg(find(Electrode.Code(k)==elposdeg(:,1)),3);
%	end;
    	Electrode.Systematic_Name{k}=setstr(s(tmp(2)+1:tmp(3)-1))';
    	Electrode.Description{k}=setstr(s(tmp(3)+1:CrPos(k+1)-2))';
end;    	

%save Electrode Electrode
for l=1:size(Label,1),
for k=1:length(Electrode.Code),
%    	if upper(char(Electrode.Acronym(k)))==upper(deblank(char(Label(l,:))))
    	if strcmp(upper(char(Electrode.Acronym(k))),upper(deblank(char(Label(l,:)))))
    		code(l,1)=Electrode.Code(k);
    	end;
end;
end;


ELN2=['FP';'AF';'FT';'FC';'TP';'CP';'PO'];
ELY2=[ 2   3    5    5    7    7    9];
ELN1=['E';'F';'T';'C';'M';'A';'P';'O';'I'];
ELY1=[ 1   4   6   6   7   7   8   10  11];

[nr,nc]=size(char(Label));
[er,ec]=size(ELPOS);

YX=[0 0];
for k=1:nr,
    	L=upper(char(Label(k,:)));
	eno=find(((L>='0') & (L<='9')) | (L=='Z'));

	FY2=0;
	i=1;    
	for i=1:length(eno),
		if eno(i)>2
  			for l=1:7,
  				FY2(l)=all(ELN2(l,:)==upper(L(eno(i)+[-2:-1])));
  			end;
  			YX1=ELY2(FY2~=0);
        	end;
        	if any(FY2)
			YX2=findstr(ELPOS(YX1,:),upper(L(eno(i)+[-2:0])));
			tmp=0;
               	else
        		FY1=ELN1==upper(L(eno(i)-1));  	
  			YX1=ELY1(FY1);
			YX2=findstr(ELPOS(YX1,:),upper(L(eno(i)+[-1:0])));
  		end;

		if ~isempty(YX1) & ~isempty(YX2) 
			YX=[11-YX1 YX2/4-0.25];
		end;
    	end;
end;


